/*
 * Copyright (c) 2020 Aisha Tammy <purrito@bsd.ac>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <algorithm>
#include <bits/stdint-uintn.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <App.h>

#include "purrito.h"

/*
 * high precision timer and random number generator
 * see: https://codeforces.com/blog/entry/61587
 * it is also thread safe, so useful for async
 */
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

/* generate a random slug of required length */
std::string random_slug(const int &);

/*
 * try and save a buffer to file and return the error code
 * and also save the returned filename in the argument
 */
uint8_t save_buffer(const char *, const purrito_settings &, char *);

/*
 * read data in a registered call back function
 */
uint8_t read_paste(const purrito_settings &, uWS::HttpResponse<false> *,
                   char *);

/******************************************************************************/

void purr(const purrito_settings &settings) {

  /* create a standard non tls app to listen for requests */
  uWS::App()
      .post("/",
            [&](auto *res, auto *req) {
              /* create an array for storing the returning string */
              char *paste_url;

              /* calculate number of characters in the paste_url */
              uint32_t n = settings.domain.length() + settings.slug_size + 2;

              /* now we can malloc it, remember to free */
              paste_url = (char *)malloc(n * sizeof(char));
              memset(paste_url, 0, n * sizeof(char));

              /* register the callback, which will cork the request properly */
              int perr = read_paste(settings, res, paste_url);

              /*
               * if something went wrong we are guaranteed that
               * paste_url has nothing in it
               */
              if (perr != 0) {

                /* send out a warning */
                warn("Purrito: WARNING (%d) - could not process the request\n",
                     n);

                /* free the malloc cuz we getting out of here */
                free(paste_url);

                return;
              }

              /* remember to free the paste_url */
              free(paste_url);

              res->onAborted([]() {
                printf("Purrito: Warning - request was prematurely aborted\n");
              });
            })
      .listen(settings.bind_port,
              [](auto *listenSocket) {
                if (listenSocket) {
                  printf("Purrito: Listening for connections...\n");
                }
              })
      .run();

  /* if we reached here, it means something went wrong */
  err(EXIT_FAILURE, "Error: could not start listening on the socket");
}

/******************************************************************************/

/*
 * process the request
 */
uint8_t read_paste(const purrito_settings &settings,
                   uWS::HttpResponse<false> *res, char *paste_url) {

  /* calculate the correct number of characters allowed in the paste */
  uint32_t max_chars = settings.max_paste_size / sizeof(char);

  printf("max_chars %d\n", max_chars);

  /* now create the buffer, remember to free */
  char *buffer;
  buffer = (char *)malloc(max_chars + 1);

  /* keep a counter on how much was already read */
  uint32_t *read_count = new uint32_t;
  *read_count = 0;

  printf("going to start reading the paste\n");

  /* uWebSockets doesn't cork something already corked so we just cork */
  res->cork([=]() {
    res->onData([=](std::string_view chunk, bool is_last) {
      /* calculate how much to copy over */
      printf("just got in\n");
      uint32_t copy_size = std::max<int>(
          0, std::min<int>(max_chars - *read_count, chunk.size()));

      printf("copy size: %d\n", copy_size);
      /* actually do copy it over */
      chunk.copy(buffer + *read_count, copy_size);
      printf("copied\n");
      if (is_last) {
        printf("is_last %d\n", is_last);
        /* remember to increment the read count */
        *read_count = copy_size + *read_count;
        printf("read_count %d\n", *read_count);
        /* there are two condition when we stop and save */
        if (is_last || *read_count == max_chars) {
          /* set the last element correctly */
          buffer[*read_count] = '\0';
          save_buffer(buffer, settings, paste_url);
          res->end(paste_url);
        }
      }
    });
  });
  return 0;
}

/*
 * save the buffer to a file and save the paste url
 */
uint8_t save_buffer(const char *buffer, const purrito_settings &settings,
                    char *paste_url) {

  printf("going to start the save\n");
  /* generate the slug */
  std::string slug = random_slug(settings.slug_size);
  printf("slug %s\n", slug.c_str());

  /* get the filename to open */
  std::filesystem::path ofile = settings.storage_directory;
  ofile /= slug;

  /* get the file descriptor */
  FILE *output_file = fopen(ofile.c_str(), "w");

  int write_err = fprintf(output_file, "%s", buffer);

  fclose(output_file);

  if (write_err < 0)
    warn("Purrito: WARNING (%d) - error while writing to file\n", write_err);

  settings.domain.copy(paste_url, settings.domain.size());

  slug.copy(paste_url + settings.domain.size(), slug.size());

  return 0;
}

/*
 * linear time generation of random slug
 */
std::string random_slug(const int &slug_size) {
  /* we generate only alpha-num slugs */
  std::string alphanum = "0123456789abcdefghijklmnopqrstuvwxyz";

  /* get the size, cuz 10+26 is too hard */
  size_t len = alphanum.size();

  /* work around variable length array iso dumbass */
  char *rslug = new char[slug_size + 2];

  /* finally generate the random string by sampling */
  for (int i = 0; i < slug_size; i++) {
    rslug[i] = alphanum[rng() % len];
  }

  /* add the final character for converting back to string */
  rslug[slug_size] = '\n';
  rslug[slug_size + 1] = '\0';
  std::string new_slug(rslug);

  /* definitely learning some weird paradigms in c++ */
  delete[] rslug;

  return new_slug;
}
