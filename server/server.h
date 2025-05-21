#pragma once

#define BACKLOG 5
#define MAXSOCK 1

void sigint_handler();
void *handle_request(void *arg);
