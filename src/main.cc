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

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <err.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>

#include "purrito.h"

#ifndef PURRITO_PORT
#define PURRITO_PORT 42069
#endif

/*
 * function for printing the help of the code
 */
void print_help() {
  std::printf(
      "usage: purrito [-cdeghiklmnpsvwx] -d domain [-c public_cert_file]\n"
      "               [-e dhparams_file] [-g slug_size] [-h] [-i bind_ip]\n"
      "               [-k private_key_file] [-l] [-m max_paste_size] [-n server name]\n"
      "               [-p bind_port] [-s storage_directory] [-v header_value]\n"
      "               [-w passphrase] [-x header]\n");
}

/*
 * main code for running the server
 */
int main(int argc, char **argv) {
  int opt;
  std::string domain, storage_directory;
  std::vector<std::string> bind_ip;
  std::vector<std::uint_fast16_t> bind_port;
  std::map<std::string, std::string> headers;
  std::vector<std::string> header_names, header_values;
  std::uint_fast8_t slug_size;
  std::string::size_type max_paste_size;

  /* open syslog with purritobin identity */
  openlog("purritobin", LOG_PERROR | LOG_PID, LOG_DAEMON);

  /* we should define the default values for variables not
   * considered essential
   */
  slug_size = 7;                             // the magic number
  max_paste_size = 65536;                    // seems reasonable for most
  storage_directory = "/var/www/purritobin"; // should probably be owned
                                             // by user running the program

  bool ssl_server = false;
  uWS::SocketContextOptions ssl_options = {};
  std::string server_name;

  while ((opt = getopt(argc, argv, "hd:s:i:p:m:g:ln:c:k:e:w:x:v:")) != EOF)
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
      bind_ip.push_back(optarg);
      break;
    case 'p':
      bind_port.push_back(std::stoi(optarg));
      break;
    case 'm':
      max_paste_size = std::stoull(optarg);
      break;
    case 'g':
      slug_size = std::stoi(optarg);
      break;
    case 'l':
      ssl_server = true;
      break;
    case 'n':
      server_name = optarg;
      break;
    case 'c':
      ssl_options.cert_file_name = optarg;
      break;
    case 'k':
      ssl_options.key_file_name = optarg;
      break;
    case 'e':
      ssl_options.dh_params_file_name = optarg;
      break;
    case 'w':
      ssl_options.passphrase = optarg;
      break;
    case 'x':
      header_names.push_back(optarg);
      break;
    case 'v':
      header_values.push_back(optarg);
      break;
    default:
      print_help();
      err(1, "Error: incorrect parameters");
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

  if (ssl_server && ((ssl_options.cert_file_name != NULL &&
                      strlen(ssl_options.cert_file_name) == 0) ||
                     (ssl_options.key_file_name != NULL &&
                      strlen(ssl_options.key_file_name) == 0))) {
    print_help();
    err(1, "Error: public certificate or private key not specified");
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
    int removed = std::remove(fpath.c_str());
    if (removed != 0)
      err(removed, "Error: could not remove __init__ test file");
  }

  /* based and lit method to make sure that nothing goes wrong */
#if defined(__OpenBSD__)
  /* the only directory we need access to is the storage directory */
  int unveil_err = unveil(storage_directory.c_str(), "rwc");
  if (unveil_err != 0) {
    err(unveil_err, "Error: could not unveil storage folder: %s",
        storage_directory.c_str());
  }

  if (ssl_server) {
    unveil_err = unveil(ssl_options.cert_file_name, "r");
    if (unveil_err != 0)
      err(unveil_err, "Error: could not unveil public certificate file: %s",
          ssl_options.cert_file_name);
    unveil_err = unveil(ssl_options.key_file_name, "r");
    if (unveil_err != 0)
      err(unveil_err, "Error: could not unveil private key file: %s",
          ssl_options.key_file_name);
    if (ssl_options.dh_params_file_name != NULL &&
        strlen(ssl_options.dh_params_file_name) != 0) {
      unveil_err = unveil(ssl_options.dh_params_file_name, "r");
      if (unveil_err != 0)
        err(unveil_err, "Error: could not unveil dhparams file: %s",
            ssl_options.dh_params_file_name);
    }
  }
  /* also we only need small amounts of net and socket access */
  (void)pledge("stdio rpath wpath cpath inet unix", NULL);
#endif

  /* sanitize the settings for ports and ips */
  if (bind_ip.size() == 0) {
    bind_ip.push_back("0.0.0.0");
    bind_ip.push_back("::");
  }
  if (bind_port.size() == 0) {
    bind_port.push_back(PURRITO_PORT);
  }
  while (bind_ip.size() < bind_port.size()) {
    bind_ip.push_back(bind_ip[bind_ip.size() - 1]);
  }
  while (bind_port.size() < bind_ip.size()) {
    bind_port.push_back(bind_port[bind_port.size() - 1]);
  }

  if (bind_port.size() != bind_ip.size()) {
    err(1, "Error: Could not normalize and sanitize ips and ports.");
  }

  if (header_names.size() != header_values.size()) {
    err(1, "Error: header names and values can't be matched");
  }
  for (std::map<std::string, std::string>::size_type i = 0;
       i < header_names.size(); i++)
    headers[header_names[i]] = header_values[i];

  syslog(LOG_INFO,
         "Starting PurritoBin with settings - "
         "{ "
         "domain: %s, "
         "storage_directory: %s, "
         "max_paste_size: %" PRIuFAST64 ", "
         "slug_size: %" PRIuFAST8 " }",
         domain.c_str(), storage_directory.c_str(), max_paste_size, slug_size);

  /* initialize the settings to be passed to the server */
  purrito_settings settings(domain, storage_directory, bind_ip, bind_port,
                            max_paste_size, slug_size, headers, ssl_options);

  /* create the server and start running it */
  if (ssl_server) {
    syslog(LOG_INFO, "Listening with SSL");
    auto purrito = purr<true>(settings);
    purrito.addServerName(server_name, ssl_options);
    purrito.run();
  } else {
    syslog(LOG_INFO, "Listening without SSL");
    auto purrito = purr<false>(settings);
    purrito.run();
  }

  /* it should not be possible to reach here */
  return 0;
}
