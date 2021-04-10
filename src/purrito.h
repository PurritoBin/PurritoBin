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

#ifndef _PURRITO
#define _PURRITO

#include <error.h>
#include <fcntl.h>
#include <syslog.h>

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <system_error>
#include <vector>

#if __has_include(<sys/file.h>)
#include <sys/file.h>
#endif

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
	 * DEFAULT: false
	 * enable a very simple http get server
	 */
	const bool enable_httpserver;

	/*
	 * DEFAULT: index.html
	 * index file for top level directory
	 */
	const std::string index_file;

	/*
	 * DEFAULT: 5
	 * maximum retries to find an available slug
	 */
	const std::uint_fast32_t max_retries;

	purrito_settings(const std::string &domain,
	                 const std::string &storage_directory,
	                 const std::vector<std::string> &bind_ip,
	                 const std::vector<std::uint_fast16_t> &bind_port,
	                 const std::uint_fast64_t &max_paste_size,
	                 const std::string::size_type &slug_size,
	                 const std::string &slug_characters,
	                 const std::map<std::string, std::string> &headers,
	                 const uWS::SocketContextOptions ssl_options,
	                 const bool enable_httpserver,
	                 const std::string index_file,
	                 const std::uint_fast32_t max_retries)
	    : domain(domain),
	      storage_directory(storage_directory),
	      bind_ip(bind_ip),
	      bind_port(bind_port),
	      max_paste_size(max_paste_size),
	      slug_size(slug_size),
	      slug_characters(slug_characters),
	      headers(headers),
	      ssl_options(ssl_options),
	      enable_httpserver(enable_httpserver),
	      index_file(index_file),
	      max_retries(max_retries) {}
};

/*
 * the default server and listener which will handle the requests
 * this does is using the dank uWebSockets library
 * https://github.com/uNetworking/uWebSockets
 */
template <bool SSL>
uWS::TemplatedApp<SSL> purr(const purrito_settings &);

/*
 * high precision timer and random number generator
 * see: https://codeforces.com/blog/entry/61587
 * it is also thread safe, so useful for async
 */
std::mt19937_64 rng(
    std::chrono::steady_clock::now().time_since_epoch().count());

/* generate a random slug of required length */
std::string random_slug(const std::string &, const std::string::size_type &);

/* simplified random file wrapper which locks and throws exceptions */
class purrito_paste_file {
       public:
	int fd;
	std::string slug;
	std::string file_path;
	std::FILE *file;
	bool to_remove;
	purrito_paste_file(const purrito_settings &settings)
	    : to_remove(false) {
		slug =
		    random_slug(settings.slug_characters, settings.slug_size);
		std::uint_fast32_t retries = 0;
		for (; retries < settings.max_retries;
		     retries++,
		     slug = random_slug(settings.slug_characters,
		                        settings.slug_size),
		     file_path = settings.storage_directory + slug) {
			int temp_fd =
			    open(file_path.c_str(), O_WRONLY | O_CREAT,
			         S_IRUSR | S_IWUSR | S_IRGRP);
			if (temp_fd == -1) continue;
			int locked = flock(temp_fd, LOCK_EX | LOCK_NB);
			if (locked == -1)
				continue;
			else {
				fd = temp_fd;
				file = std::fopen(file_path.c_str(), "w");
				if (!file) {
					flock(fd, LOCK_UN);
					close(fd);
					std::remove(file_path.c_str());
					continue;
				}
				file_path = settings.storage_directory + slug;
				break;
			}
		}
		if (retries == settings.max_retries) {
			throw std::system_error(std::make_error_code(
			    static_cast<std::errc>(errno)));
		}
	}
	~purrito_paste_file() {
		std::fclose(file);
		flock(fd, LOCK_UN);
		close(fd);
		if (to_remove) std::remove(file_path.c_str());
	}
};

/*
 * read data in a registered call back function
 */
template <bool SSL>
void read_paste(const purrito_settings &, const uint_fast64_t,
                std::shared_ptr<purrito_paste_file>, uWS::HttpResponse<SSL> *);

/******************************************************************************/

template <bool SSL>
uWS::TemplatedApp<SSL> purr(const purrito_settings &settings) {
	/* create a standard non tls app to listen for requests */
	auto purrito = uWS::TemplatedApp<SSL>();
	purrito.post(
	    "/",
	    /* specifically ignoring the request parameter, as c++ is dumb */
	    [&](auto *res, auto *) {
		    /* Log that we are getting a connection */
		    auto paste_ip = std::string(res->getRemoteAddressAsText());
		    std::uint_fast64_t session_id = rng();
		    syslog(
		        LOG_INFO,
		        "(%s) Got a POST connection - session id (%" PRIuFAST64
		        ")",
		        paste_ip.c_str(), session_id);

		    std::shared_ptr<purrito_paste_file> pfile;
		    /* first give the abort handler */
		    res->onAborted([=]() {
			    if (pfile) pfile->to_remove = true;
			    syslog(LOG_WARNING,
			           "(%" PRIuFAST64
			           ") WARNING: Request was prematurely aborted",
			           session_id);
		    });

		    try {
			    pfile =
			        std::make_shared<purrito_paste_file>(settings);
		    } catch (std::system_error &ex) {
			    syslog(LOG_WARNING,
			           "(%" PRIuFAST64
			           ") WARNING: Could not generate file - %s",
			           session_id, ex.what());
			    res->close();
			    return;
		    }

		    for (auto it : settings.headers)
			    res->writeHeader(it.first, it.second);

		    res->cork([&, session_id, pfile]() {
			    read_paste<SSL>(settings, session_id, pfile, res);
		    });
	    });
	if (settings.enable_httpserver)
		purrito.get("/*", [&](auto *res, auto *req) {
			std::string paste_filename(req->getUrl()), paste_data;
			/* Log that we are getting a connection */
			auto paste_ip =
			    std::string(res->getRemoteAddressAsText());
			std::uint_fast64_t session_id = rng();
			syslog(LOG_INFO,
			       "(%s) Got a GET connection {%s} - session id "
			       "(%" PRIuFAST64 ")",
			       paste_ip.c_str(), paste_filename.c_str(),
			       session_id);
			/*
			 * attach a standard abort handler, in case something
			 * goes wrong
			 */
			res->onAborted([=]() {
				syslog(LOG_WARNING,
				       "(%" PRIuFAST64
				       ") WARNING: Request was prematurely "
				       "aborted",
				       session_id);
			});

			if (paste_filename.size() <= 1)
				paste_filename = "/" + settings.index_file;

			paste_filename = settings.storage_directory +
			                 paste_filename.substr(1);

			std::ifstream paste_stream(
			    paste_filename, std::ios::in | std::ios::binary);
			if (!paste_stream) {
				res->writeStatus("404 Not Found");
			} else {
				paste_stream.seekg(0, std::ios::end);
				paste_data.resize(paste_stream.tellg());
				paste_stream.seekg(0, std::ios::beg);
				paste_stream.read(&paste_data[0],
				                  paste_data.size());
				paste_stream.close();
			}
			for (auto it : settings.headers)
				res->writeHeader(it.first, it.second);
			res->write(paste_data);
			res->end();
		});
	for (std::vector<std::uint_fast16_t>::size_type i = 0;
	     i < settings.bind_ip.size(); i++) {
		purrito.listen(settings.bind_ip[i], settings.bind_port[i],
		               [&](auto *listenSocket) {
			               if (listenSocket) {
				               syslog(
				                   LOG_INFO,
				                   "Listening for connections "
				                   "on %s:%" PRIuFAST16 "...",
				                   settings.bind_ip[i].c_str(),
				                   settings.bind_port[i]);
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
                std::shared_ptr<purrito_paste_file> pfile,
                uWS::HttpResponse<SSL> *res) {
	/* calculate the correct number of characters allowed in the paste */
	uint_fast64_t max_chars = settings.max_paste_size;

	/* keep a counter on how much was already read */
	auto read_count = std::make_unique<std::uint_fast64_t>(0);

	/* get the paste_url */
	std::unique_ptr<std::string> paste_url =
	    std::make_unique<std::string>(settings.domain + pfile->slug);

	/* Log that we are starting to read the paste */
	syslog(LOG_INFO, "(%" PRIuFAST64 ") Starting to read the paste",
	       session_id);

	res->onData([=, read_count = std::move(read_count),
	             paste_url = std::move(paste_url)](std::string_view chunk,
	                                               bool is_last) {
		if (chunk.size() > max_chars - *read_count) {
			syslog(LOG_WARNING,
			       "(%" PRIuFAST64
			       ") WARNING: paste was too large, "
			       "forced to close the request",
			       session_id);
			res->close();
			return;
		}

		/* remember to increment the read count */
		*read_count = chunk.size() + *read_count;

		int write_count = fwrite(chunk.data(), sizeof(char),
		                         chunk.size(), pfile->file);

		if (write_count < 0) {
			syslog(LOG_WARNING,
			       "(%" PRIuFAST64
			       ") WARNING: error (%d) while writing to file",
			       session_id, write_count);
			res->close();
			return;
		}

		if (is_last) {
			/* Log that we finished reading the paste */
			syslog(
			    LOG_INFO,
			    "(%" PRIuFAST64
			    ") Finished reading a paste of size %" PRIuFAST64,
			    session_id, *read_count);

			/* print out the separator */
			syslog(LOG_INFO,
			       "(%" PRIuFAST64 ") Sent paste url back: %s",
			       session_id, paste_url->c_str());

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

#endif  //_PURRITO
