CC = clang
CFLAGS = -Wall -Werror -Wextra -Icommon

CLIENT = client
CLIENTSRCS = $(wildcard $(CLIENT)/*.c)
CLIENTBIN = $(CLIENT)/shell.out

CMD = cmd
CMDSRCS = $(wildcard $(CLIENT)/$(CMD)/*.c)
CMDBIN = $(patsubst $(CLIENT)/$(CMD)/%.c, $(CLIENT)/$(CMD)/bin/%, $(CMDSRCS))

SERVER = server
SERVERSRCS = $(wildcard $(SERVER)/*.c)
SERVERBIN = $(SERVER)/$(SERVER).out

COMMON = common
COMMONSRCS = $(wildcard $(COMMON)/*.c)

all: $(CLIENTBIN) $(SERVERBIN)

$(CLIENTBIN): $(CLIENTSRCS) $(CMDBIN) $(COMMONSRCS)
	@$(CC) $(CFLAGS) $(CLIENTSRCS) $(COMMONSRCS) -o $@

$(CLIENT)/$(CMD)/bin/%: $(CLIENT)/$(CMD)/%.c
	@mkdir -p $(CLIENT)/$(CMD)/bin
	@$(CC) $(CFLAGS) $< $(COMMONSRCS) -o $@

$(SERVERBIN): $(SERVERSRCS) $(COMMONSRCS)
	@$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -rf $(CLIENTBIN) $(CMDBIN) $(SERVERBIN)

runs:
	@$(SERVERBIN)

runc:
	@$(CLIENTBIN)

.PHONY: all clean runs runc
