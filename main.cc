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


#include <string>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <fstream>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

#include "purrito.hh"

using namespace std;

/*
 * function for printing the help of the code
 */
void
print_help() {
  cout << "usage: purrito [-hdsipmg]                                       " << endl
       << "        -h                                                      " << endl
       << "            print this help                                     " << endl
       << "        -d domain                                               " << endl
       << "            REQUIRED                                            " << endl
       << "            domain that will be used as prefix of returned paste" << endl
       << "            NOTE: should be the full name, including trailing / " << endl
       << "              e.g. https://bsd.ac/                              " << endl << endl
       << "        -s storage_directory                                    " << endl
       << "            REQUIRED                                            " << endl
       << "            path to the storage directory for storing the paste " << endl
       << "            NOTE: should exist prior to creation and should be  " << endl
       << "                  writable by the user running purrito          " << endl
       << "              e.g. /var/www/purrito                             " << endl << endl
       << "        -i bind_ip                                              " << endl
       << "            DEFAULT: 0.0.0.0                                    " << endl
       << "            IP on which to listen for incoming connections      " << endl << endl
       << "        -p bind_port                                            " << endl
       << "            DEFAULT: 42069                                      " << endl
       << "            port on which to listen for connections             " << endl << endl
       << "        -m max_paste_size (in bytes)                            " << endl
       << "            DEFAULT: 65536 (64KB)                               " << endl << endl
       << "        -g slug_size                                            " << endl
       << "            DEFAULT: 7                                          " << endl << endl;
}

/*
 * main code for running the server
 */
int
main(int argc, char **argv) {

  int opt;
  string domain, storage_directory, bind_ip;
  uint8_t slug_size;
  uint16_t bind_port;
  uint32_t max_paste_size;

  /* we should define the default values for variables not considered essential */
  bind_ip = "0.0.0.0";
  slug_size = 7; // the magic number
  bind_port = 42069; // dank af
  max_paste_size = 65536; // seems reasonable for most

  while( (opt = getopt(argc, argv, "hd:s:i:p:m:g:")) != EOF )
    switch(opt) {
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
      bind_port = stoi(optarg);
      break;
    case 'm':
      max_paste_size = stoi(optarg);
      break;
    case 'g':
      slug_size = stoi(optarg);
      break;
    default:
      print_help();
      err(1, "Error: incomplete arguments");
      break;
    }

  /* based and lit method to make sure that nothing goes wrong */
#if defined(__OpenBSD__)
  /* the only directory we need access to is the storage directory */
  int unveil_error = unveil(storage_directory, "rwxc");
  if (unveil_err != 0) {
    err(unveil_err, "Error: could not unveil storage folder: %s", storage_directory);
    return unveil_err;
  }
  /* also we only need small amounts of net and socket access */
  (void)pledge("stdio rpath wpath cpath inet unix", NULL);
#endif

  /*
   * we also do a basic check to see that the domain name is given
   * NOTE: this is not a full check to see a valid domain name
   *       or something thereof, you need to initialize with a correct
   *       name, or suffer the consequences
   */
  if (domain == "") {
    print_help();
    cout << endl;
    err(1, "Error: empty domain name");
  }

  /*
   * lets first check that we can even access it correcty, and afterwards we will do
   * a write test to see if everything worked out a-OK
   */
  if (storage_directory == "" || access(storage_directory.c_str(), W_OK) != 0){
    print_help();
    cout << endl;
    err(1, "Error: storage directory is invalid or is not writable");
  }

  /*
   * the best method to take care of the fact that the given path
   * is a directory of a link to a directory or anything there of, is to try and
   * make a init file and write to it and then we delete it
   */
  {
    filesystem::path fpath = storage_directory;
    fpath /= "__init__";
    ofstream output(fpath.string());
    (void)remove(fpath.c_str());
  }

  purrito_settings settings(domain, storage_directory, bind_ip, bind_port, max_paste_size, slug_size);

  purrito purr(settings);

  purr.start_server();
}
