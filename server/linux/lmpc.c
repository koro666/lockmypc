#include "config.h"
#include "server.h"

int main(int argc, char** argv)
{
	struct lmpc_cfg* cfg = cfg_load_path(argc >= 2 ? argv[1] : NULL);
	if (!cfg)
		return 1;

	int result = srv_run_cfg(cfg);

	cfg_free(cfg);
	return result;
}
