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

int main(int argc, char **argv) {
	/* open syslog with purritobin identity */
	openlog("purritobin", LOG_PERROR | LOG_PID, LOG_DAEMON);

	bool config_check_only = false;
	std::string config_file = "/etc/purritobin.ini";

	int opt;
	while ((opt = getopt(argc, argv, "f:n")) != EOF) {
		switch (opt) {
			case 'f':
				config_file = optarg;
				break;
			case 'n':
				config_check_only = true;
				break;
			default:
				std::printf(
				    "usage: purrito [-nf] [-f config_file] "
				    "[-n]");
				std::exit(EXIT_FAILURE);
				break;
		}
	}
	purrito_settings settings = parse_config(config_file);

	if (config_check_only) std::exit(EXIT_SUCCESS);

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
