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
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <syslog.h>

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
  const std::vector<std::uint_fast16_t> bind_port;

  /*
   * DEFAULT: 65536 // 64KB
   * size in bytes of the largest possible paste
   */
  const std::uint_fast64_t max_paste_size;

  /*
   * DEFAULT: 7
   * size of the random slug for the paste
   */
  const std::string::size_type slug_size;

  /*
   * DEFAULT: "0123456789abcdefghijklmnopqrstuvwxyz"
   * characters used for generating the slug
   */
  const std::string slug_characters;

  /*
   * DEFAULT: {}
   * response headers
   */
  const std::map<std::string, std::string> headers;

  /*
   * DEFAULT: {}
   * ssl options for https listener
   * - cert
   * - key
   * - dhparam
   * - passphrase
   */
  const uWS::SocketContextOptions ssl_options;

  /*
   * DEFAULT: index.html
   * index file for top level directory
   */
  const std::string index_file;

  purrito_settings(const std::string &domain,
                   const std::string &storage_directory,
                   const std::vector<std::string> &bind_ip,
                   const std::vector<std::uint_fast16_t> &bind_port,
                   const std::uint_fast64_t &max_paste_size,
                   const std::string::size_type &slug_size,
                   const std::string &slug_characters,
                   const std::map<std::string, std::string> &headers,
                   const uWS::SocketContextOptions ssl_options,
                   const std::string index_file)
      : domain(domain), storage_directory(storage_directory), bind_ip(bind_ip),
        bind_port(bind_port), max_paste_size(max_paste_size),
        slug_size(slug_size), slug_characters(slug_characters),
        headers(headers), ssl_options(ssl_options), index_file(index_file) {}
};

/*
 * the default server and listener which will handle the requests
 * this does is using the dank uWebSockets library
 * https://github.com/uNetworking/uWebSockets
 */
template <bool SSL> uWS::TemplatedApp<SSL> purr(const purrito_settings &);

/*
 * high precision timer and random number generator
 * see: https://codeforces.com/blog/entry/61587
 * it is also thread safe, so useful for async
 */
std::mt19937_64
    rng(std::chrono::steady_clock::now().time_since_epoch().count());

/* generate a random slug of required length */
std::string random_slug(const std::string &, const std::string::size_type &);

/*
 * read data in a registered call back function
 */
template <bool SSL>
void read_paste(const purrito_settings &, uWS::HttpResponse<SSL> *);

/******************************************************************************/
template <bool SSL>
uWS::TemplatedApp<SSL> purr(const purrito_settings &settings) {

  /* create a standard non tls app to listen for requests */
  auto purrito = uWS::TemplatedApp<SSL>();
  purrito
      .post("/",
            /* specifically ignoring the request parameter, as c++ is dumb */
            [&](auto *res, auto *) {
              /* Log that we are getting a connection */
              auto paste_ip = std::string(res->getRemoteAddressAsText());
              std::uint_fast64_t session_id = rng();
              syslog(LOG_INFO,
                     "(%s) Got a POST connection - session id (%" PRIuFAST64
                     ")",
                     paste_ip.c_str(), session_id);

              for (auto it : settings.headers)
                res->writeHeader(it.first, it.second);

              /*
               * register the callback, which will cork the request properly
               */
              res->cork([=]() { read_paste<SSL>(settings, session_id, res); });

              /*
               * attach a standard abort handler, in case something goes wrong
               */
              res->onAborted([=]() {
                syslog(LOG_WARNING,
                       "(%" PRIuFAST64
                       ") WARNING: Request was prematurely aborted",
                       session_id);
              });
            })
      .get("/*", [&](auto *res, auto *req) {
        std::string paste_filename(req->getUrl()), paste_data;
        /* Log that we are getting a connection */
        auto paste_ip = std::string(res->getRemoteAddressAsText());
        std::uint_fast64_t session_id = rng();
        syslog(LOG_INFO,
               "(%s) Got a GET connection {%s} - session id (%" PRIuFAST64 ")",
               paste_ip.c_str(), paste_filename.c_str(), session_id);
        /*
         * attach a standard abort handler, in case something goes wrong
         */
        res->onAborted([=]() {
          syslog(LOG_WARNING,
                 "(%" PRIuFAST64 ") WARNING: Request was prematurely aborted",
                 session_id);
        });

        if (paste_filename.size() <= 1)
          paste_filename = "/" + settings.index_file;

        paste_filename = settings.storage_directory + paste_filename.substr(1);

        std::ifstream paste_stream(paste_filename,
                                   std::ios::in | std::ios::binary);
        if (!paste_stream) {
          res->writeStatus("404 Not Found");
        } else {
          paste_stream.seekg(0, std::ios::end);
          paste_data.resize(paste_stream.tellg());
          paste_stream.seekg(0, std::ios::beg);
          paste_stream.read(&paste_data[0], paste_data.size());
          paste_stream.close();
        }
        for (auto it : settings.headers)
          res->writeHeader(it.first, it.second);
        res->write(paste_data);
        res->end();
      });
  for (std::vector<std::uint_fast16_t>::size_type i = 0;
       i < settings.bind_ip.size(); i++) {
    purrito.listen(
        settings.bind_ip[i], settings.bind_port[i], [&](auto *listenSocket) {
          if (listenSocket) {
            syslog(LOG_INFO,
                   "Listening for connections on %s:%" PRIuFAST16 "...",
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
template <bool SSL>
void read_paste(const purrito_settings &settings,
                const std::uint_fast64_t session_id,
                uWS::HttpResponse<SSL> *res) {
  /* calculate the correct number of characters allowed in the paste */
  uint_fast64_t max_chars = settings.max_paste_size;

  std::string slug = random_slug(settings.slug_characters, settings.slug_size);
  auto paste_url = std::make_unique<std::string>(settings.domain + slug + "\n");

  /* get the filename to open */
  auto ofile = settings.storage_directory + slug;

  auto ofile_path = std::make_unique<std::string>(ofile.begin(), ofile.end());

  /* get the file descriptor */
  FILE *output_file = fopen(ofile.c_str(), "wx");

  if (output_file == NULL) {
    syslog(LOG_WARNING, "(%" PRIuFAST64 ") WARNING: error while opening file",
           session_id);
    res->close();
    return;
  }

  /* keep a counter on how much was already read */
  auto read_count = std::make_unique<std::uint_fast64_t>(0);

  /* Log that we are starting to read the paste */
  syslog(LOG_INFO, "(%" PRIuFAST64 ") Starting to read the paste", session_id);

  res->onData([=, read_count = std::move(read_count),
               paste_url = std::move(paste_url),
               ofile_path = std::move(ofile_path)](std::string_view chunk,
                                                   bool is_last) {
    if (chunk.size() > max_chars - *read_count) {
      syslog(LOG_WARNING,
             "(%" PRIuFAST64 ") WARNING: paste was too large, "
             "forced to close the request",
             session_id);
      std::fclose(output_file);
      std::remove(ofile_path->c_str());
      res->close();
      return;
    }

    /* remember to increment the read count */
    *read_count = chunk.size() + *read_count;

    int write_count =
        fwrite(chunk.data(), sizeof(char), chunk.size(), output_file);

    if (write_count < 0) {
      syslog(LOG_WARNING,
             "(%" PRIuFAST64 ") WARNING: error (%d) while writing to file",
             session_id, write_count);
      std::fclose(output_file);
      std::remove(ofile_path->c_str());
      res->close();
      return;
    }

    if (is_last) {
      std::fclose(output_file);
      /* Log that we finished reading the paste */
      syslog(LOG_INFO,
             "(%" PRIuFAST64 ") Finished reading a paste of size %" PRIuFAST64,
             session_id, *read_count);

      /* print out the separator */
      syslog(LOG_INFO, "(%" PRIuFAST64 ") Sent paste url back: %s", session_id,
             paste_url->c_str());

      /* and return it to the user */
      res->end(paste_url->c_str());
    }
  });
}

/*
 * linear time generation of random slug
 */
std::string random_slug(const std::string &slug_characters,
                        const std::string::size_type &slug_size) {
  auto rslug = std::unique_ptr<char[]>(new char[slug_size + 1]);

  /* finally generate the random string by sampling */
  for (std::string::size_type i = 0; i < slug_size; i++) {
    rslug[i] = slug_characters[rng() % slug_characters.length()];
  }

  /* add the final character for converting back to string */
  rslug[slug_size] = '\0';
  std::string new_slug(rslug.get());

  return new_slug;
}

#endif //_PURRITO
