#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <errno.h>
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

	result = srv_run_addrs(addr, &ctx);
	freeaddrinfo(addr);
	return result;
}

int srv_run_addrs(const struct addrinfo* addr, const struct lmpc_srv_ctx* ctx)
{
	size_t depth = 0;
	for (const struct addrinfo* caddr = addr; caddr; caddr = caddr->ai_next)
		++depth;

	int fds[depth];
	size_t count = 0;

	for (const struct addrinfo* caddr = addr; caddr; caddr = caddr->ai_next)
	{
		int fd = socket(caddr->ai_family, caddr->ai_socktype | SOCK_CLOEXEC, caddr->ai_protocol);
		if (fd == -1)
		{
			perror("socket");
			continue;
		}

#ifdef __linux__
		if (caddr->ai_family == AF_INET6)
		{
			int value = 1;
			if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &value, sizeof(value)))
				perror("setsockopt");
		}
#endif

		if (bind(fd, caddr->ai_addr, caddr->ai_addrlen) == -1)
		{
			perror("bind");
			close(fd);
			continue;
		}

		fds[count++] = fd;
	}

	if (!count)
		return count != depth;

	int result = srv_run_socks(fds, count, ctx);

	for (size_t i = 0; i < count; ++i)
		close(fds[i]);

	return result;
}

int srv_run_socks(const int* fds, size_t count, const struct lmpc_srv_ctx* ctx)
{
	while (true)
	{
		fd_set rfds;
		FD_ZERO(&rfds);
		int nfds = 0;

		for (size_t i = 0; i < count; ++i)
		{
			int fd = fds[i];
			FD_SET(fd, &rfds);

			int x = fd + 1;
			if (x > nfds)
				nfds = x;
		}

		int iresult;
		do { iresult = select(nfds, &rfds, NULL, NULL, NULL); } while (iresult == -1 && errno == EINTR);

		if (iresult == -1)
		{
			perror("select");
			return 1;
		}

		for (size_t i = 0; i < count; ++i)
		{
			int fd = fds[i];
			if (!FD_ISSET(fd, &rfds))
				continue;

			struct sockaddr_storage addr;
			struct lmpc_packet packet;
			socklen_t addrlen = sizeof(struct sockaddr_storage);

			ssize_t sresult;
			do { sresult = recvfrom(fd, &packet, sizeof(packet), MSG_TRUNC, (struct sockaddr*)&addr, &addrlen); } while (sresult == -1 && errno == EINTR);

			if (sresult == -1)
			{
				perror("recvfrom");
				return 1;
			}

			srv_handle_packet(&packet, sresult, (struct sockaddr*)&addr, addrlen, ctx);
		}
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
