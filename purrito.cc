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


#include <chrono>
#include <ctime>
#include <random>
#include <filesystem>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>

#include <err.h>
#include <errno.h>

#include "purrito.hh"

/*
 * high precision timer and random number generator
 * see: https://codeforces.com/blog/entry/61587
 * it is also thread safe, so useful for async
 */
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

/* generate a random slug of required length */
std::string
random_slug(const int&);

/* dispatch connections */
void
dispatch_connection(const int&, const purrito_settings*);

/* handle incoming connections */
void
*handle_connection(void *);

/*
 * used for the sole purpose of handing down information
 * to a child thread which is handling the connection
 */
class connection_info {
public:
  const int sockd;
  const purrito_settings *settings;
  const struct sockaddr_in address;

  connection_info(int sockd, const purrito_settings *settings, const sockaddr_in address): sockd(sockd), settings(settings), address(address){}
};

/*****************************************/

/*
 * linear time generation of random slug
 */
std::string
random_slug(const int &slug_size) {
  /* we generate only alpha-num slugs */
  std::string alphanum = "0123456789abcdefghijklmnopqrstuvwxyz";
  /* get the size, cuz 10+26 is too hard */
  size_t len = alphanum.size();
  /* work around variable length array iso dumbass */
  char *rslug = new char [slug_size + 1];
  /* finally generate the random string by sampling */
  for(int i = 0; i < slug_size; i++){
    rslug[i] = alphanum[rng() % len];
  }
  /* add the final character for converting back to string */
  rslug[slug_size] = '\0';
  std::string new_slug(rslug);
  /* definitely learning some weird paradigms in c++ */
  delete[] rslug;
  return new_slug;
}

/*
 * simple server which creates a thread for each incoming connection
 * and each thread handles their own i/o making it dank af
 */
void
purrito::start_server() {
  /* get the socket descriptor by creating it */
  int sockd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockd < 0)
    err(sockd, "Error: failed to create the socket");

  /* lets add some options to the socket */
  int opt = 1;
  int sockopt = setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
  if (sockopt != 0)
    err(sockopt, "Error: couldn't prepare the socket");

  /* now we need to bind to this socket, so create an address */
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(settings.bind_ip.c_str());
  address.sin_port = htons(settings.bind_port);

  int bindv = bind(sockd, (struct sockaddr *) &address, sizeof(address));
  if (bindv != 0)
    err(bindv, "Error: could not bind to port %d on ip %s", settings.bind_port, settings.bind_ip.c_str());

  /* we need to listen on the socket */
  int listenv = listen(sockd, 128);
  if (listenv != 0)
    err(listenv, "Error: could not start listening on the socket");

  printf("Purrito: Started listening on %s:%d\n", settings.bind_ip.c_str(), settings.bind_port);
  printf("------------------------------\n");

  while (1) {
    dispatch_connection(sockd, &settings);
  }
}

/*
 * will block until there is a connection
 * and then spawn a thread to handle it
 */
void dispatch_connection(const int &sockd, const purrito_settings *settings){
  struct sockaddr_in address;

  socklen_t addlen = sizeof(address);

  const int newsockd = accept(sockd, (struct sockaddr *) &address, &addlen);

  printf("Purrito: Got a new connection\n");

  if (newsockd < 0){
    warn("Warning: could not accept incoming connection: %d\n", newsockd);
    return;
  }
  const struct timeval timeout = { 5, 0 };

  /* get receive timeout value */
  int rcvtimev = setsockopt(newsockd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  if (rcvtimev != 0)
    warn("Warning: couldn't set a receive timeout: %d\n", rcvtimev);

  /* get send timeout value */
  int sndtimev = setsockopt(newsockd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  if (sndtimev != 0)
    warn("Warning: couldn't set a send timeout: %d\n", sndtimev);

  /* create a tuple for values to send in thread */
  connection_info *connection = new connection_info(newsockd, settings, address);

  /* spawn a new thread to handle this connection */
  pthread_t id;

  /* try to create the thread and get the value */
  int threadv = pthread_create(&id, NULL, &handle_connection, connection);
  if (threadv != 0){
    warn("Warning: couldn't spawn a thread to handle the connection: %d\n", threadv);
    return;
  }

  pthread_detach(id);
}

void *handle_connection(void *args) {
  connection_info *connection = reinterpret_cast <connection_info *>(args);

  /* get client's IP */
  const char *ip = inet_ntoa(connection->address.sin_addr);

  /* get client's hostname */
  char hostname[1024];
  int hostnamev = getnameinfo((struct sockaddr *)&connection->address, sizeof(connection->address),
                              hostname, sizeof(hostname), NULL, 0, 0);
  /* it is fine if we couldn't resolve a hostname */
  if (hostnamev != 0)
    strcpy(hostname, "n/a");

  /* print status on this connection */
  {
    auto date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    printf("Purrito: Incoming connection from: %s (%s) @ %s\n", ip, hostname, std::ctime(&date));
  }

  // Create a buffer
  uint8_t *buffer = new uint8_t [connection->settings->max_paste_size];
  memset(buffer, 0, connection->settings->max_paste_size);

  printf("Purrito: receving paste\n");

  //const int bytes = read(connection->sockd, buffer, connection->settings->max_paste_size);
  const int bytes = recv(connection->sockd, buffer, connection->settings->max_paste_size, MSG_WAITALL);
  if (bytes <= 0) {

    printf("Purrito: no data received from the client\n");
    printf("------------------------------\n");

    // Close the socket
    close(connection->sockd);

    // Cleanup
    free(connection);
    delete[] buffer;
    pthread_exit(NULL);

    return 0;
  }

  printf("Purrito: received paste of size %d\n", bytes);

  std::string slug = random_slug(connection->settings->slug_size);

  std::filesystem::path ofile = connection->settings->storage_directory;
  ofile /= slug;
  FILE *f = fopen(ofile.c_str(), "w");
  buffer[connection->settings->max_paste_size - 1] = 0;
  fprintf(f, "%s", buffer);
  fclose(f);

  printf("Purrito: wrote it to file %s\n", ofile.c_str());
  printf("------------------------------\n");


  std::string slug_url = connection->settings->domain + slug;
  std::string return_message = "Your paste is available at: " + slug_url + "\n";
  static_cast<void>(write(connection->sockd, return_message.c_str(), return_message.size()));

  close(connection->sockd);

  free(connection);
  delete[] buffer;
  pthread_exit(NULL);

  return NULL;
}
