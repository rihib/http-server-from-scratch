#pragma once

#define INITIAL_BUF_SIZE 1024

int send_all(int sockfd, char *buf);
int receive_all(int sockfd, char **buf);
