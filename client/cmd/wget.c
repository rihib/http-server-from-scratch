#include "wget.h"

#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "send_and_receive.h"

int main(int argc, char *argv[]) {
  int error;
  const char *cause = NULL;

  // Get hostname
  if (argc != 2) {
    errx(EXIT_FAILURE, "Usage: %s <hostname>", argv[0]);
  }
  const char *hostname = argv[1];

  // See `man getaddrinfo` Examples
  struct addrinfo hints, *infos;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  error = getaddrinfo(hostname, "http", &hints, &infos);
  if (error) {
    errx(EXIT_FAILURE, "%s", gai_strerror(error));
  }
  int sockfd = -1;
  for (struct addrinfo *info = infos; info; info = info->ai_next) {
    // Create socket
    sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (sockfd == -1) {
      cause = "failed to create socket";
      continue;
    }

    // Connect
    if (connect(sockfd, info->ai_addr, info->ai_addrlen) == -1) {
      cause = "failed to connect";
      close(sockfd);
      sockfd = -1;
      continue;
    }

    break;
  }
  if (sockfd == -1) {
    err(EXIT_FAILURE, "%s", cause);
  }
  freeaddrinfo(infos);
  cause = NULL;

  // Send HTTP GET Request
  const char *request_format =
      "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n";
  int request_size = snprintf(NULL, 0, request_format, hostname) + 1;
  char *request = malloc(request_size);
  if (request == NULL) {
    error = EXIT_FAILURE;
    cause = "failed to allocate request";
    goto cleanup;
  }
  snprintf(request, request_size, request_format, hostname);
  if (send_all(sockfd, request) == -1) {
    error = EXIT_FAILURE;
    cause = "failed to send";
    goto cleanup;
  }

  // Receive HTTP Response
  char *buf = NULL;
  int received = receive_all(sockfd, &buf);
  if (received == -1) {
    error = EXIT_FAILURE;
    cause = "failed to receive";
    goto cleanup;
  }
  printf("\n%s\n", buf);

cleanup:
  if (close(sockfd) == -1) {
    err(EXIT_FAILURE, "failed to close");
  }
  if (error) {
    err(EXIT_FAILURE, "%s", cause);
  }
  return 0;
}
