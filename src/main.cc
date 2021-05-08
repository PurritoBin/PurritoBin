/*
 * Copyright (c) 2020-2021 Aisha Tammy <purrito@bsd.ac>
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

#include <err.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <string>

#include "purrito.h"

#ifndef PURRITO_PORT
#define PURRITO_PORT 42069
#endif

// clang-format off
void print_help() {
  std::printf("usage: purrito [-abcdefghijklmnpqrstvwxz] -d domain [-a slug_characters]\n"
              "               [-b max_database_size] [-c public_cert_file] [-e dhparams_file]\n"
              "               [-f index_file] [-g slug_size] [-h] [-i bind_ip]\n"
              "               [-j autoclean_interval] [-k private_key_file] [-l]\n"
              "               [-m max_paste_size] [-n server name] [-p bind_port]\n"
              "               [-q default_time_limit] [-r max_retries] [-s storage_directory]\n"
              "               [-t] [-v header_value] [-w passphrase] [-x header]\n"
              "               [-z database_directory]\n");
}
// clang-format on

int main(int argc, char **argv) {
	int opt;
	std::string domain, storage_directory, database_directory,
	    slug_characters, index_file, server_name;
	std::vector<std::uint_fast16_t> bind_port;
	std::map<std::string, std::string> headers;
	std::vector<std::string> bind_ip, header_names, header_values;
	std::uint_fast8_t slug_size;
	std::uint_fast32_t max_retries;
	bool enable_httpserver, ssl_server;
	uWS::SocketContextOptions ssl_options;
	std::string::size_type max_paste_size;
	std::uint_fast64_t max_database_size, default_time_limit,
	    autoclean_interval;

	/* open syslog with purritobin identity */
	openlog("purritobin", LOG_PERROR | LOG_PID, LOG_DAEMON);

	/* we should define the default values for variables not
	 * considered essential
	 */
	slug_size = 7;  // the magic number
	slug_characters = "0123456789abcdefghijklmnopqrstuvwxyz";
	enable_httpserver = false;
	index_file = "index.html";
	max_paste_size = 65536;  // seems reasonable for most
	max_retries = 5;
	storage_directory = "/var/www/purritobin/";
	database_directory = "/var/db/purritobin/";
	max_database_size = 16777216;
	ssl_server = false;
	default_time_limit = 604800;  // 1 week in seconds \o/
	autoclean_interval = 300;     // 5 mins in seconds

	while ((opt = getopt(argc, argv,
	                     "a:b:c:d:e:f:g:hi:j:k:lm:n:p:q:r:s:tv:w:x:z:")) !=
	       EOF)
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
				bind_port.push_back(std::stoul(optarg));
				break;
			case 'm':
				max_paste_size = std::stoull(optarg);
				break;
			case 'g':
				slug_size = std::stoul(optarg);
				break;
			case 'a':
				slug_characters = optarg;
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
			case 'f':
				index_file = optarg;
				break;
			case 't':
				enable_httpserver = true;
				break;
			case 'r':
				max_retries = std::stoul(optarg);
				break;
			case 'b':
				max_database_size = std::stoull(optarg);
				break;
			case 'z':
				database_directory = optarg;
				break;
			case 'q':
				default_time_limit = std::stoull(optarg);
				default_time_limit *= (unsigned long long)1e9;
				break;
			case 'j':
				autoclean_interval = std::stoull(optarg);
				break;
			default:
				print_help();
				errx(1, "ERROR: incorrect parameters");
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
		errx(1, "ERROR: empty domain name");
	}

	if (slug_characters == "") {
		print_help();
		errx(1, "ERROR: slug character set is empty");
	}

	if (ssl_server && ((ssl_options.cert_file_name != NULL &&
	                    strlen(ssl_options.cert_file_name) == 0) ||
	                   (ssl_options.key_file_name != NULL &&
	                    strlen(ssl_options.key_file_name) == 0))) {
		print_help();
		errx(1,
		     "ERROR: public certificate or private key not specified");
	}

	/*
	 * lets first check that we can even access it correcty, and afterwards
	 * we will do a write test to see if everything worked out a-OK
	 */
	if (storage_directory.back() != '/') storage_directory += "/";
	if (database_directory.back() != '/') database_directory += "/";

	if (access(storage_directory.c_str(), W_OK) != 0) {
		print_help();
		errx(1,
		     "ERROR: storage directory is invalid or is not writable");
	}
	if (access(database_directory.c_str(), W_OK) != 0) {
		print_help();
		errx(1,
		     "ERROR: database directory is invalid or is not writable");
	}

	/*
	 * the best method to take care of the fact that the given path
	 * is a directory of a link to a directory or anything there of, is to
	 * try and make a init file and write to it and then we delete it
	 */
	{
		auto fpath = storage_directory + "__init__";
		std::ofstream output(fpath);
		int removed = std::remove(fpath.c_str());
		if (removed != 0)
			errx(removed,
			     "ERROR: could not remove __init__ test file");
	}
	{
		auto fpath = database_directory + "__init__";
		std::ofstream output(fpath);
		int removed = std::remove(fpath.c_str());
		if (removed != 0)
			errx(removed,
			     "ERROR: could not remove __init__ test file");
	}

#if defined(__OpenBSD__)
	/* the only directory we need access to is the storage directory */
	int unveil_err = unveil(storage_directory.c_str(), "rwc");
	if (unveil_err != 0) {
		errx(unveil_err, "ERROR: could not unveil storage folder: %s",
		     storage_directory.c_str());
	}
	unveil_err = unveil(database_directory.c_str(), "rwc");
	if (unveil_err != 0) {
		errx(unveil_err, "ERROR: could not unveil storage folder: %s",
		     database_directory.c_str());
	}
	unveil_err = unveil("/tmp", "rwc");
	if (unveil_err != 0) {
		errx(unveil_err, "ERROR: could not unveil temporary folder: /tmp");
	}

	if (ssl_server) {
		unveil_err = unveil(ssl_options.cert_file_name, "r");
		if (unveil_err != 0)
			errx(unveil_err,
			     "ERROR: could not unveil public certificate file: "
			     "%s",
			     ssl_options.cert_file_name);
		unveil_err = unveil(ssl_options.key_file_name, "r");
		if (unveil_err != 0)
			errx(unveil_err,
			     "ERROR: could not unveil private key file: %s",
			     ssl_options.key_file_name);
		if (ssl_options.dh_params_file_name != NULL &&
		    strlen(ssl_options.dh_params_file_name) != 0) {
			unveil_err =
			    unveil(ssl_options.dh_params_file_name, "r");
			if (unveil_err != 0)
				errx(
				    unveil_err,
				    "ERROR: could not unveil dhparams file: %s",
				    ssl_options.dh_params_file_name);
		}
	}
	/* also we only need small amounts of net and socket access */
	(void)pledge("stdio rpath wpath cpath inet unix flock", NULL);
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
		errx(1,
		     "ERROR: Could not normalize and sanitize ips and ports.");
	}

	if (header_names.size() != header_values.size()) {
		errx(1, "ERROR: header names and values can't be matched");
	}
	for (std::map<std::string, std::string>::size_type i = 0;
	     i < header_names.size(); i++)
		headers[header_names[i]] = header_values[i];

	syslog(LOG_INFO,
	       "Starting PurritoBin with settings - "
	       "{ "
	       "domain: %s, "
	       "slug_size: %" PRIuFAST8
	       ", storage_directory: %s"
	       ", database_directory: %s"
	       ", max_paste_size: %" PRIuFAST64
	       ", max_database_size: %" PRIuFAST64
	       ", autoclean_interval: %" PRIuFAST64
	       ", default_time_limit: %" PRIuFAST64
	       ", max_retries: %" PRIuFAST32 " }",
	       domain.c_str(), slug_size, storage_directory.c_str(),
	       database_directory.c_str(), max_paste_size, max_database_size,
	       autoclean_interval, default_time_limit, max_retries);

	/* initialize the settings to be passed to the server */
	purrito_settings settings(domain, storage_directory, database_directory,
	                          bind_ip, bind_port, max_paste_size,
	                          max_database_size, slug_size, slug_characters,
	                          default_time_limit, headers, ssl_options,
	                          enable_httpserver, index_file, max_retries);

	/* create the server and start running it */
	std::thread purrito_thread;
	if (ssl_server) {
		syslog(LOG_INFO, "Listening with SSL");
		purrito_thread = std::thread([&]() {
			auto purrito = purr<true>(settings);
			purrito.addServerName(server_name, ssl_options);
			purrito.run();
		});
	} else {
		syslog(LOG_INFO, "Listening without SSL");
		purrito_thread = std::thread([&]() {
			auto purrito = purr<false>(settings);
			purrito.run();
		});
	}
	auto cleaner = std::thread([&]() {
		while (1) {
			syslog(LOG_INFO, "(cleaner) Starting a new run...");
			auto current_time = time_since_epoch();
			std::vector<std::string> files_to_clean, timestamps;
			try {
				{
					auto rtxn = lmdb::txn::begin(
					    settings.env, nullptr, MDB_RDONLY);
					auto dbi =
					    lmdb::dbi::open(rtxn, nullptr);
					std::string_view timestamp, slug;
					auto cursor =
					    lmdb::cursor::open(rtxn, dbi);
					if (cursor.get(timestamp, slug,
					               MDB_FIRST)) {
						do {
							if (timestamp <
							    current_time) {
								timestamps
								    .emplace_back(
								        timestamp);
								files_to_clean
								    .emplace_back(
								        slug);
							} else
								continue;
						} while (cursor.get(
						    timestamp, slug, MDB_NEXT));
					}
				}
			} catch (lmdb::error &ex) {
				syslog(LOG_WARNING,
				       "(cleaner) Caught an error while "
				       "cursoring - { %d, %s )",
				       ex.code(), ex.what());
			} catch (...) {
			}
			syslog(LOG_INFO,
			       "(cleaner) Number of pastes to clean = %zu",
			       files_to_clean.size());
			for (std::string &paste : files_to_clean)
				syslog(LOG_INFO, "(cleaner) - %s",
				       paste.c_str());
			try {
				auto wtxn = lmdb::txn::begin(settings.env);
				auto dbi = lmdb::dbi::open(wtxn, nullptr);

				for (std::size_t i = 0; i < timestamps.size();
				     i++) {
					std::string file_path =
					    settings.storage_directory +
					    files_to_clean[i];
					int fd =
					    open(file_path.c_str(), O_WRONLY);
					if (fd != -1) {
						int locked = flock(
						    fd, LOCK_EX | LOCK_NB);
						if (locked == -1) {
							close(fd);
							continue;
						} else {
							close(fd);
							flock(fd, LOCK_UN);
						}
						std::remove(file_path.c_str());
					}
					dbi.del(wtxn, timestamps[i]);
				}
				wtxn.commit();
			} catch (lmdb::error &ex) {
				syslog(LOG_WARNING,
				       "(cleaner) Caught an error while "
				       "cleaning - { %d, %s )",
				       ex.code(), ex.what());
			} catch (...) {
			}
			syslog(LOG_INFO, "(cleaner) Sleeping...");
			std::this_thread::sleep_for(
			    std::chrono::seconds(autoclean_interval));
		}
	});
	cleaner.join();
	purrito_thread.join();

	/* it should not be possible to reach here */
	return 0;
}
