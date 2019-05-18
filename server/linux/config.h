#pragma once
#include <stdio.h>

struct lmpc_cfg
{
	char* address;
	char* port;
	char* secret;
	char* command;
};

char* cfg_get_path();
struct lmpc_cfg* cfg_load_path(const char*);
struct lmpc_cfg* cfg_load_fp(FILE*);
void cfg_free(struct lmpc_cfg*);
