#include "send_and_receive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int send_all(int sockfd, char *buf) {
  int total_sent = 0;
  int len = strlen(buf);
  while (total_sent < len) {
    int sent = send(sockfd, buf + total_sent, len - total_sent, 0);
    if (sent == -1) {
      perror("failed to send");
      return -1;
    }
    total_sent += sent;
  }
  return total_sent;
}

int receive_all(int sockfd, char **buf) {
  size_t buf_size = INITIAL_BUF_SIZE;
  size_t total_received = 0;
  ssize_t received;
  char tmp_buf[512];

  *buf = malloc(buf_size);
  if (*buf == NULL) {
    perror("failed to allocate buf");
    return -1;
  }
  while (1) {
    received = recv(sockfd, tmp_buf, sizeof(tmp_buf) - 1, 0);
    if (received == -1) {
      perror("recv failed");
      free(*buf);
      return -1;
    }
    tmp_buf[received] = '\0';
    // Double the buffer size when it is full
    // to leave space for the null terminator
    if (total_received + received >= buf_size) {
      buf_size *= 2;
      char *new_buf = realloc(*buf, buf_size);
      if (new_buf == NULL) {
        perror("failed to reallocate buf");
        free(*buf);
        return -1;
      }
      *buf = new_buf;
    }
    memcpy(*buf + total_received, tmp_buf, received);
    total_received += received;
    if (strstr(*buf, "\r\n\r\n") != NULL) {
      break;
    }
  }
  (*buf)[total_received] = '\0';
  return total_received;
}
