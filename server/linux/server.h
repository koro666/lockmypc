#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include "config.h"

struct lmpc_srv_ctx
{
	const char* secret;
	const char* command;
};

struct lmpc_packet
{
	uint32_t signature;
	uint32_t time;
	uint8_t hash[32];
};

int srv_run_cfg(const struct lmpc_cfg*);
int srv_run_addrs(const struct addrinfo*, const struct lmpc_srv_ctx*);
int srv_run_socks(const int*, size_t, const struct lmpc_srv_ctx*);
void srv_handle_packet(const struct lmpc_packet*, ssize_t, const struct sockaddr*, socklen_t, const struct lmpc_srv_ctx*);
bool srv_check_packet(const struct lmpc_packet*, ssize_t, const struct lmpc_srv_ctx*);
