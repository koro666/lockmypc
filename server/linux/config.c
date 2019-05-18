#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

static const char cfg_filename[] = "lmpc.conf";

static const char cfg_default_address[] = "";
static const char cfg_default_port[] = "1024";
static const char cfg_default_secret[] = "default";
static const char cfg_default_command[] = "xscreensaver-command -lock";

char* cfg_get_path()
{
	char* result;
	char* env = getenv("XDG_CONFIG_DIR");
	if (env)
	{
		if (asprintf(&result, "%s/%s", env, cfg_filename) == -1)
			return NULL;
		else
			return result;
	}

	env = getenv("HOME");
	if (env)
	{
		if (asprintf(&result, "%s/.config/%s", env, cfg_filename) == -1)
			return NULL;
		else
			return result;
	}

	return NULL;
}

struct lmpc_cfg* cfg_load_path(const char* path)
{
	FILE* fp = NULL;
	struct lmpc_cfg* result = NULL;

	if (path)
	{
		fp = fopen(path, "re");
	}
	else
	{
		char* buffer = cfg_get_path();
		if (buffer)
		{
			fp = fopen(buffer, "re");
			free(buffer);
		}

		if (!fp)
			fp = fopen(cfg_filename, "re");
	}

	result = cfg_load_fp(fp);
	if (fp)
		fclose(fp);

	return result;
}

struct lmpc_cfg* cfg_load_fp(FILE* fp)
{
	struct lmpc_cfg* result = malloc(sizeof(struct lmpc_cfg));
	memset(result, 0, sizeof(struct lmpc_cfg));

	if (fp)
	{
		while (!feof(fp))
		{
			char buffer[1024];
			if (!fgets(buffer, sizeof(buffer), fp))
				break;

			char* start = buffer;
			while (isspace(*start))
				++start;

			if (*start == '#')
				continue;

			char* end = start + strcspn(start, "\n\r");
			*end = 0;

			char* separator = strchr(start, '=');
			if (!separator)
				continue;

			*separator = 0;
			char *key = start,
				*value = separator + 1;

			while (separator > start && isspace(separator[-1]))
				--separator;
			*separator = 0;

			while (isspace(*value))
				++value;

			if (!strcmp(key, "address"))
			{
				if (result->address)
					continue;

				result->address = strdup(value);
			}
			else if (!strcmp(key, "port"))
			{
				if (result->port)
					continue;

				result->port = strdup(value);
			}
			else if (!strcmp(key, "secret"))
			{
				if (result->secret)
					continue;

				result->secret = strdup(value);
			}
			else if (!strcmp(key, "command"))
			{
				if (result->command)
					continue;

				result->command = strdup(value);
			}
		}
	}

	if (!result->address)
		result->address = strdup(cfg_default_address);
	if (!result->port)
		result->port = strdup(cfg_default_port);
	if (!result->secret)
		result->secret = strdup(cfg_default_secret);
	if (!result->command)
		result->command = strdup(cfg_default_command);

	return result;
}

void cfg_free(struct lmpc_cfg* cfg)
{
	free(cfg->command);
	free(cfg->secret);
	free(cfg->port);
	free(cfg->address);
	free(cfg);
}
