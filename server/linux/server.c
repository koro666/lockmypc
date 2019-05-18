#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include "server.h"

int srv_run_cfg(const struct lmpc_cfg* cfg)
{
	struct addrinfo hints =
	{
		.ai_flags = AI_NUMERICHOST | AI_PASSIVE | AI_ADDRCONFIG,
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_DGRAM
	};

	struct addrinfo* addr;
	int result = getaddrinfo(*cfg->address ? cfg->address : NULL, cfg->port, &hints, &addr);
	if (result)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
		return 1;
	}

	struct lmpc_srv_ctx ctx =
	{
		.secret = cfg->secret,
		.command = cfg->command
	};

	result = srv_run_addr(addr, &ctx);
	freeaddrinfo(addr);
	return result;
}

int srv_run_addr(const struct addrinfo* addr, const struct lmpc_srv_ctx* ctx)
{
	int fd = socket(addr->ai_family, addr->ai_socktype | SOCK_CLOEXEC, addr->ai_protocol);
	if (fd == -1)
	{
		perror("socket");
		return 1;
	}

	if (bind(fd, addr->ai_addr, addr->ai_addrlen) == -1)
	{
		perror("bind");
		close(fd);
		return 1;
	}

	int result = srv_run_sock(fd, ctx);
	close(fd);
	return result;
}

int srv_run_sock(int fd, const struct lmpc_srv_ctx* ctx)
{
	while (true)
	{
		struct sockaddr_storage addr;
		struct lmpc_packet packet;
		socklen_t addrlen = sizeof(struct sockaddr_storage);

		ssize_t result = recvfrom(fd, &packet, sizeof(packet), MSG_TRUNC, (struct sockaddr*)&addr, &addrlen);
		if (result == -1)
		{
			perror("recvfrom");
			return 1;
		}

		srv_handle_packet(&packet, result, (struct sockaddr*)&addr, addrlen, ctx);
	}
}

void srv_handle_packet(const struct lmpc_packet* packet, ssize_t pktlen, const struct sockaddr* addr, socklen_t addrlen, const struct lmpc_srv_ctx* ctx)
{
	bool valid = srv_check_packet(packet, pktlen, ctx);

	char host[NI_MAXHOST], port[NI_MAXSERV];
	if (getnameinfo(addr, addrlen, host, sizeof(host), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV))
	{
		*host = 0;
		*port = 0;
	}

	printf("[LockMyPC] Received %s packet from %s port %s.\n", valid ? "valid" : "invalid", host, port);
	fflush(stdout);

	if (valid)
		system(ctx->command);
}

bool srv_check_packet(const struct lmpc_packet* packet, ssize_t pktlen, const struct lmpc_srv_ctx* ctx)
{
	if (pktlen != sizeof(struct lmpc_packet))
		return false;

	if (ntohl(packet->signature) != 0x4c4f434b)
		return false;

	int64_t now = time(NULL);
	int64_t then = (int32_t)ntohl(packet->time);

	if (labs(then - now) >= 60L)
		return false;

	SHA256_CTX c;
	unsigned char md[SHA256_DIGEST_LENGTH];

	SHA256_Init(&c);
	SHA256_Update(&c, &packet->time, sizeof(packet->time));
	SHA256_Update(&c, ctx->secret, strlen(ctx->secret));
	SHA256_Final(md, &c);

	return !memcmp(packet->hash, md, sizeof(packet->hash));
}
