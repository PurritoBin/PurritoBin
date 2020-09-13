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
 *
 */

#ifndef _PURRITO
#define _PURRITO

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
#include <syslog.h>
#include <unistd.h>

#include <uWebSockets/App.h>

class purrito_settings {
public:
  /*
   * REQUIRED
   * domain that will be used as prefix of returned paste
   * NOTE: should be the full name, including trailing /
   *   e.g. https://bsd.ac/
   */
  const std::string domain;

  /*
   * REQUIRED
   * path to the storage directory for storing the paste
   * NOTE: should exist prior to creation and should be
   *       writable by the user running purrito
   */
  const std::string storage_directory;

  /*
   * DEFAULT: 0.0.0.0
   * IP on which to listen for incoming connections
   * NOTE: defaults to all
   */
  const std::vector<std::string> bind_ip;

  /*
   * DEFAULT: 42069 // dank af
   * port on which to listen for connections
   * NOTE: if you want to make sure that connections
   *       are not going to be abused, look at something
   *       such as fail2ban
   */
  const std::vector<uint16_t> bind_port;

  /*
   * DEFAULT: 65536 // 64KB
   * size in bytes of the largest possible paste
   */
  const uint32_t max_paste_size;

  /*
   * DEFAULT: 7
   * size of the random slug for the paste
   */
  const uint8_t slug_size;

  /*
   * DEFAULT: {}
   * ssl options for https listener
   * - cert
   * - key
   * - dhparam
   * - passphrase
   */
   const struct us_socket_context_options_t ssl_options;

  purrito_settings(const std::string &domain,
                   const std::string &storage_directory,
                   const std::vector<std::string> &bind_ip,
                   const std::vector<uint16_t> &bind_port,
                   const uint32_t &max_paste_size, const uint8_t &slug_size,
                   const struct us_socket_context_options_t ssl_options)
      : domain(domain), storage_directory(storage_directory), bind_ip(bind_ip),
        bind_port(bind_port), max_paste_size(max_paste_size),
        slug_size(slug_size), ssl_options(ssl_options) {}
};

/*
 * the default server and listener which will handle the requests
 * this does is using the dank uWebSockets library
 * https://github.com/uNetworking/uWebSockets
 */
uWS::App purr(const purrito_settings &);

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
std::string save_buffer(const char *, const uint32_t, const char *,
                        const purrito_settings &);

/*
 * read data in a registered call back function
 */
void read_paste(const purrito_settings &, char *, uWS::HttpResponse<false> *);

/******************************************************************************/

uWS::App purr(const purrito_settings &settings) {

  /* create a standard non tls app to listen for requests */
  auto purrito = uWS::App();
  purrito.post(
      "/",
      /* specifically ignoring the request parameter, as c++ is dumb */
      [&](auto *res, auto *) {
        /* Log that we are getting a connection */
        size_t ip_size = res->getRemoteAddressAsText().size();
        auto paste_ip = new char[ip_size + 1];
        memcpy(paste_ip, std::string(res->getRemoteAddressAsText()).c_str(),
               ip_size + 1);
        syslog(LOG_INFO, "(%s) Got a connection...", paste_ip);

        /* register the callback, which will cork the request properly
         */
        res->cork([=]() { read_paste(settings, paste_ip, res); });

        /*
         * attach a standard abort handler, in case something goes wrong
         */
        res->onAborted([&]() {
          syslog(LOG_WARNING, "(%s) Warning: request was prematurely aborted",
                 paste_ip);
        });
      });
  for (size_t i = 0; i < settings.bind_ip.size(); i++) {
    purrito.listen(
        settings.bind_ip[i], settings.bind_port[i], [&](auto *listenSocket) {
          if (listenSocket) {
            syslog(LOG_INFO, "Listening for connections on %s:%d...",
                   settings.bind_ip[i].c_str(), settings.bind_port[i]);
          }
        });
  }
  return purrito;
}

/******************************************************************************/

/*
 * process the request
 */
void read_paste(const purrito_settings &settings, char *paste_ip,
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
  syslog(LOG_INFO, "(%s) Starting to read the paste", paste_ip);

  /* uWebSockets doesn't cork something already corked so we cork */
  res->cork([=]() {
    res->onData([=](std::string_view chunk, bool is_last) {
      /* calculate how much to copy over */
      uint32_t copy_size = std::max<int>(
          0, std::min<int>(max_chars - *read_count, chunk.size()));

      /* actually do copy it over */
      chunk.copy(buffer + *read_count, copy_size);

      /* remember to increment the read count */
      *read_count = copy_size + *read_count;

      if (!is_last && *read_count == max_chars) {
        syslog(LOG_WARNING,
               "(%s) Warning: paste was too large, "
               "forced to close the request",
               paste_ip);
        delete read_count;
        delete[] paste_ip;
        free(buffer);
        res->close();
      }
      /* there are two condition when we stop and save */
      else if (is_last) {

        /* set the last element correctly */
        buffer[*read_count] = '\0';

        /* Log that we finished reading the paste */
        syslog(LOG_INFO, "(%s) Finished reading a paste of size %u", paste_ip,
               *read_count);

        /* get the paste_url after saving */
        std::string paste_url =
            save_buffer(buffer, *read_count, paste_ip, settings);

        /* print out the separator */
        syslog(LOG_INFO, "(%s) Sent paste url back", paste_ip);

        /* free the proper variables */
        delete read_count;
        delete[] paste_ip;
        free(buffer);

        /* and return it to the user */
        res->end(paste_url.c_str());
      }
    });
  });
}

/*
 * save the buffer to a file and return the paste url
 */
std::string save_buffer(const char *buffer, const uint32_t buffer_size,
                        const char *paste_ip,
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
    syslog(LOG_WARNING, "(%s) Warning: error (%d) while writing to file",
           paste_ip, write_count);
    return "";
  }
  syslog(LOG_INFO, "(%s) Saved paste to file %s", paste_ip, slug.c_str());

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

#endif //_PURRITO
