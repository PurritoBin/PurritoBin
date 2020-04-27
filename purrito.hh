/*
 * Copyright (c) 2020 Aisha Tammy <purrito@bsd.ac>

 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

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

#include <cstdint>
#include <string>

using namespace std;

class purrito_settings {
public:
  /*
   * REQUIRED
   * domain that will be used as prefix of returned paste
   * NOTE: should be the full name, including trailing /
   *   e.g. https://bsd.ac/
   */
  const string domain;

  /*
   * REQUIRED
   * path to the storage directory for storing the paste
   * NOTE: should exist prior to creation and should be
   *       writable by the user running purrito
   */
  const string storage_directory;

  /*
   * DEFAULT: 0.0.0.0
   * IP on which to listen for incoming connections
   * NOTE: defaults to all
   */
  const string bind_ip;

  /*
   * DEFAULT: 42069 // dank af
   * port on which to listen for connections
   * NOTE: if you want to make sure that connections
   *       are not going to be abused, look at something
   *       such as fail2ban
   */
  const uint16_t bind_port;

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

  purrito_settings(const string &domain, const string &storage_directory, const string & bind_ip, const uint16_t &bind_port, const uint32_t &max_paste_size, const uint8_t &slug_size): domain(domain), storage_directory(storage_directory), bind_ip(bind_ip), bind_port(bind_port), max_paste_size(max_paste_size), slug_size(slug_size) {}
};

class purrito {
public:
  const purrito_settings settings;

  purrito(const purrito_settings settings): settings(settings) {}

  void start_server();
};

#endif //_PURRITO
