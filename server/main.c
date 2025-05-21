#include "main.h"

#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "send_and_receive.h"
#include "server.h"

volatile sig_atomic_t interrupted = 0;

int main() {
  fprintf(stderr, "Starting server...\n\n");

  fprintf(stderr, "Setting up signal handler...\n");
  setup_signal_handler_or_die();
  fprintf(stderr, "Signal handler setup complete.\n\n");

  fprintf(stderr, "Initializing listening sockets...\n");
  // See `man getaddrinfo` Examples
  struct addrinfo hints, *res, *res0;
  int error;
  int s[MAXSOCK];  // Sockets
  int nsock;
  const char *cause = NULL;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;      // IPv4 and IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = AI_PASSIVE;      // Wildcard
  error = getaddrinfo(NULL, "http", &hints, &res0);
  if (error) {
    errx(EXIT_FAILURE, "%s", gai_strerror(error));
  }
  nsock = 0;
  for (res = res0; res && nsock < MAXSOCK; res = res->ai_next) {
    // Create socket
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
      cause = "socket";
      continue;
    }
    // Socket Option
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      cause = "setsockopt";
      close(sock);
      continue;
    }
    // Bind
    if (bind(sock, res->ai_addr, res->ai_addrlen) < 0) {
      cause = "bind";
      close(sock);
      continue;
    }
    // Listen
    if (listen(sock, BACKLOG) < 0) {
      cause = "listen";
      close(sock);
      continue;
    }
    s[nsock++] = sock;
  }
  if (nsock == 0) {
    err(EXIT_FAILURE, "%s", cause);
  }
  freeaddrinfo(res0);
  fprintf(stderr, "Listening sockets initialization complete.\n\n");

  fprintf(stderr, "Listening on port 80...\n\n");
  while (interrupted == 0) {
    fd_set readfds;
    FD_ZERO(&readfds);
    int maxfd = -1;
    for (int i = 0; i < nsock; i++) {
      FD_SET(s[i], &readfds);
      if (s[i] > maxfd) {
        maxfd = s[i];
      }
    }

    int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
    if (ready < 0) {
      if (interrupted) {
        break;
      }
      perror("select failed");
      continue;
    }
    for (int i = 0; i < nsock; i++) {
      if (FD_ISSET(s[i], &readfds)) {
        // Accept
        struct sockaddr_storage addr;
        socklen_t len = sizeof(addr);
        int *cs = malloc(sizeof(int));
        if (cs == NULL) {
          perror("malloc failed");
          continue;
        }
        *cs = accept(s[i], (struct sockaddr *)&addr, &len);
        if (*cs < 0) {
          perror("accept failed");
          free(cs);
          continue;
        }
        // Create thread to handle request
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_request, cs) != 0) {
          perror("pthread_create failed");
          close(*cs);
          free(cs);
          continue;
        }
        pthread_detach(thread);
      }
    }
  }

  fprintf(stderr, "\nShutting down server...\n");
  for (int i = 0; i < nsock; i++) {
    close(s[i]);
  }
  fprintf(stderr, "Server stopped.\n");
  return 0;
}

void setup_signal_handler_or_die() {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigint_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) < 0) {
    err(EXIT_FAILURE, "setup_signal_handler_or_die failed");
  }
}

void sigint_handler() { interrupted = 1; }

void *handle_request(void *arg) {
  int cs = *(int *)arg;
  free(arg);

  // Receive
  char *buf = NULL;
  int received = receive_all(cs, &buf);
  if (received < 0) {
    perror("recv failed");
    goto cleanup;
  }
  fprintf(stderr, "Received:\n%s\n", buf);

  // Create HTTP Response
  char *response =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/html\r\n"
      "Connection: close\r\n"
      "\r\n"
      "<html><body><h1>Hello from server</h1></body></html>"
      "\r\n\r\n";

  // Send HTTP Response
  if (send_all(cs, response) < 0) {
    perror("send failed");
    goto cleanup;
  }
  fprintf(stderr, "Sent:\n%s\n", response);

cleanup:
  close(cs);
  fprintf(stderr, "Client connection closed\n");
  pthread_exit(NULL);
}
