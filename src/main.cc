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

#include <filesystem>
#include <fstream>
#include <string>

#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

#include "purrito.h"

/*
 * function for printing the help of the code
 */
void print_help() {
  printf(
      "usage: purrito [-hdsipmg]                                       \n\n"
      "        -h                                                        \n"
      "            print this help                                     \n\n"
      "        -d domain                                                 \n"
      "            REQUIRED                                              \n"
      "            domain that will be used as prefix of returned paste  \n"
      "            NOTE: should be the full name, including trailing /   \n"
      "              e.g. https://bsd.ac/                              \n\n"
      "        -s storage_directory                                      \n"
      "            DEFAULT: /var/www/purritobin                          \n"
      "            path to the storage directory for storing the paste   \n"
      "            NOTE: should exist prior to creation and should be    \n"
      "                  writable by the user running purrito            \n"
      "        -i bind_ip                                                \n"
      "            DEFAULT: 0.0.0.0                                      \n"
      "            IP on which to listen for incoming connections      \n\n"
      "        -p bind_port                                              \n"
      "            DEFAULT: 42069                                        \n"
      "            port on which to listen for connections             \n\n"
      "        -m max_paste_size (in bytes)                              \n"
      "            DEFAULT: 65536 (64KB)                               \n\n"
      "        -g slug_size                                              \n"
      "            DEFAULT: 7                                          \n\n");
}

/*
 * main code for running the server
 */
int main(int argc, char **argv) {
  int opt;
  std::string domain, storage_directory, bind_ip;
  uint8_t slug_size;
  uint16_t bind_port;
  uint32_t max_paste_size;

  /* open syslog with purritobin identity */
  openlog("purritobin", LOG_PERROR | LOG_PID, LOG_DAEMON);

  /* we should define the default values for variables not
   * considered essential
   */
  bind_ip = "0.0.0.0";
  slug_size = 7;                             // the magic number
  bind_port = 42069;                         // dank af
  max_paste_size = 65536;                    // seems reasonable for most
  storage_directory = "/var/www/purritobin"; // should probably be owned
                                             // by user running the program

  while ((opt = getopt(argc, argv, "hd:s:i:p:m:g:")) != EOF)
    switch (opt) {
    case 'h':
      print_help();
      return 0;
      break;
    case 'd':
      domain = optarg;
      break;
    case 's':
      storage_directory = optarg;
      break;
    case 'i':
      bind_ip = optarg;
      break;
    case 'p':
      bind_port = std::stoi(optarg);
      break;
    case 'm':
      max_paste_size = std::stoi(optarg);
      break;
    case 'g':
      slug_size = std::stoi(optarg);
      break;
    default:
      print_help();
      err(1, "Error: incomplete arguments");
      break;
    }

  /*
   * we also do a basic check to see that the domain name is given
   * NOTE: this is not a full check to see a valid domain name
   *       or something thereof, you need to initialize with a correct
   *       name, or suffer the consequences
   */
  if (domain == "") {
    print_help();
    err(1, "Error: empty domain name");
  }

  /*
   * lets first check that we can even access it correcty, and afterwards we
   * will do a write test to see if everything worked out a-OK
   */
  if (access(storage_directory.c_str(), W_OK) != 0) {
    print_help();
    err(1, "Error: storage directory is invalid or is not writable");
  }

  /*
   * the best method to take care of the fact that the given path
   * is a directory of a link to a directory or anything there of, is to try and
   * make a init file and write to it and then we delete it
   */
  {
    std::filesystem::path fpath = storage_directory;
    fpath /= "__init__";
    std::ofstream output(fpath.string());
    (void)remove(fpath.c_str());
  }

  /* based and lit method to make sure that nothing goes wrong */
#if defined(__OpenBSD__)
  /* the only directory we need access to is the storage directory */
  int unveil_err = unveil(storage_directory.c_str(), "rwxc");
  if (unveil_err != 0) {
    err(unveil_err, "Error: could not unveil storage folder: %s",
        storage_directory.c_str());
  }
  /* also we only need small amounts of net and socket access */
  (void)pledge("stdio rpath wpath cpath inet unix", NULL);
#endif

  syslog(LOG_INFO,
         "Started PurritoBin with settings - "
         "{ "
         "domain: %s, "
         "storage_directory: %s, "
         "bind_ip: %s, "
         "bind_port: %d, "
         "max_paste_size: %d, "
         "slug_size: %d "
         "}",
         domain.c_str(), storage_directory.c_str(), bind_ip.c_str(), bind_port,
         max_paste_size, slug_size);
  /* initialize the settings to be passed to the server */
  purrito_settings settings(domain, storage_directory, bind_ip, bind_port,
                            max_paste_size, slug_size);

  /* create the server and start running it */
  purr(settings);

  /* it should not be possible to reach here */
  return 0;
}
