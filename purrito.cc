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
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
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

#include <uWebSockets/App.h>

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
std::string save_buffer(const char *, const uint32_t, const purrito_settings &);

/*
 * read data in a registered call back function
 */
void read_paste(const purrito_settings &, uWS::HttpResponse<false> *);

/******************************************************************************/

void purr(const purrito_settings &settings) {

  /* create a standard non tls app to listen for requests */
  uWS::App()
      .post("/",
            /* specifically ignoring the requet parameter, as c++ is dumb */
            [&](auto *res, auto *) {
              /* Log that we are getting a connection */
              printf("Purrito: Got a connection\n");

              /* register the callback, which will cork the request properly
               */
              res->cork([=]() { read_paste(settings, res); });

              /*
               * attach a standard abort handler, in case something goes wrong
               */
              res->onAborted([]() {
                printf("Purrito: Warning - request was prematurely aborted\n");
              });
            })
      .listen(settings.bind_port,
              [](auto *listenSocket) {
                if (listenSocket) {
                  printf("Purrito: Listening for connections...\n"
                         "-----------------------------------"
                         "-----------------------------------\n");
                }
              })
      .run();

  /* if we reached here, it means something went wrong
   */
  err(EXIT_FAILURE, "Error: could not start listening on the socket");
}

/******************************************************************************/

/*
 * process the request
 */
void read_paste(const purrito_settings &settings,
                uWS::HttpResponse<false> *res) {
  /* calculate the correct number of characters allowed in the paste */
  uint32_t max_chars = settings.max_paste_size / sizeof(char);

  /* now create the buffer, remember to free */
  char *buffer;
  buffer = (char *)malloc(max_chars + 1);

  /* keep a counter on how much was already read */
  uint32_t *read_count = new uint32_t;
  *read_count = 0;

  /* Log that we are starting to read the paste */
  printf("Purrito: Starting to read the paste\n");

  /* uWebSockets doesn't cork something already corked so we cork */
  res->cork([=]() {
    res->onData([=](std::string_view chunk, bool is_last) {
      /* calculate how much to copy over */
      uint32_t copy_size = std::max<int>(
          0, std::min<int>(max_chars - *read_count, chunk.size()));

      /* actually do copy it over */
      chunk.copy(buffer + *read_count, copy_size);
      if (is_last) {

        /* remember to increment the read count */
        *read_count = copy_size + *read_count;

        /* there are two condition when we stop and save */
        if (is_last || *read_count == max_chars) {

          /* set the last element correctly */
          buffer[*read_count] = '\0';

          /* Log that we finished reading the paste */
          printf("Purrito: Finished reading the paste of size %u\n",
                 *read_count);

          /* get the paste_url after saving */
          std::string paste_url = save_buffer(buffer, *read_count, settings);

          /* and return it to the user */
          res->end(paste_url.c_str());
          free(read_count);

          /* print out the separator */
          printf("-----------------------------------"
                 "-----------------------------------\n");
        }
      }
    });
  });
}

/*
 * save the buffer to a file and return the paste url
 */
std::string save_buffer(const char *buffer, const uint32_t buffer_size,
                        const purrito_settings &settings) {
  /* generate the slug */
  std::string slug = random_slug(settings.slug_size);

  /* get the filename to open */
  std::filesystem::path ofile = settings.storage_directory;
  ofile /= slug;

  /* get the file descriptor */
  FILE *output_file = fopen(ofile.c_str(), "w");

  int write_count = fwrite(buffer, sizeof(char), buffer_size, output_file);

  fclose(output_file);

  if (write_count < 0) {
    warn("Purrito: WARNING (%d) - error while writing to file\n", write_count);
    return "PurritoBin: Error while pasting\n";
  }

  return settings.domain + slug + "\n";
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
  char *rslug = new char[slug_size + 1];

  /* finally generate the random string by sampling */
  for (int i = 0; i < slug_size; i++) {
    rslug[i] = alphanum[rng() % len];
  }

  /* add the final character for converting back to string */
  rslug[slug_size] = '\0';
  std::string new_slug(rslug);

  /* definitely learning some weird paradigms in c++ */
  delete[] rslug;

  return new_slug;
}
