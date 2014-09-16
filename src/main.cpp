#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <execinfo.h>

#include <uv.h>

#include "conf.h"
#include "server.h"
#include "core.h"
#include "deleted_list.h"

Conf_t g_conf;
char g_sys_path[MAX_PATH_LEN];

void showV()
{
}

void usage(char * prname)
{
	printf("\n");
	printf("Project    : %s\n", PROJECT_NAME);
	printf("Version    : %s\n", SERVER_VERSION);
	printf("Usage: %s [-h -v -V -p]\n", prname);
	printf("	[-v]  verbose\n");
	printf("	[-h]  show version info\n");
	printf("	[-V]  show file version info\n");
	printf("	[-p]  set work path\n");
}

int init()
{
  signal(SIGPIPE, SIG_IGN);
	// load_sysconf
	if (load_conf(&g_conf, "../conf/server.conf") < 0) {
		return -1;
	}

	//if (server_init() < 0) {
		//return -1;
	//}

	srand(time(NULL));

	return 0;
}

void get_work_dir(const char *path)
{
    realpath(path, g_sys_path);
    char *p = dirname(g_sys_path);
    if (NULL == p) {
      sprintf(g_sys_path, "..");
      return;
    }
    p = dirname(p);
    if (NULL == p) {
      sprintf(g_sys_path, "..");
      return;
    }
    if (p[1] == '\0' && p[0] == '.') {
      sprintf(g_sys_path, "..");
      return;
    }
    sprintf(g_sys_path, "%s", p);
}

int main_parse_option(int argc, char **argv)
{
	memset(&g_conf, 0, sizeof(g_conf));
  get_work_dir(argv[0]);
	char opt = 0;
	const struct option long_options[] = {
		{"path",        1, NULL, 'p'},
		{"help",        0, NULL, 'h'},
		{"Version",     0, NULL, 'V'},
		{NULL,          0, NULL, 0}
	};

	while( (opt=getopt_long(argc, argv, "p:hV", long_options, NULL)) != -1 ) {
		switch( opt ) {
			case 'p':
				snprintf(g_sys_path, sizeof(g_sys_path), "%s", optarg);
				break;
			case 'V':
				showV();
				exit(1);
				break;
			case 'h':
				usage(argv[0]);
				exit(1);
				break;
		}
	}

	char path[MAX_PATH_LEN];
	snprintf(path, sizeof(path), "%s/pid", g_sys_path);
	g_conf.pidfd = open(path, O_CREAT | O_WRONLY, 0644);
	if (g_conf.pidfd == -1) {
		WARNING_LOG("open %s fail %s", path, strerror(errno));
		exit(-1);
		return -1;
	}

	struct flock    lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	if (-1 == fcntl(g_conf.pidfd, F_SETLK, &lock)) {
		WARNING_LOG("lock pidfile fail %s", strerror(errno));
		exit(-1);
		return -1;
	}
	ftruncate(g_conf.pidfd, 0);
	sprintf(path, "%d\n", getpid());
	write(g_conf.pidfd, path, strlen(path));

  return 0;
}

int main(int argc, char *argv[])
{
  main_parse_option(argc, argv);

  if (init() < 0) {
    WARNING_LOG("fail to init!");
    return EXIT_FAILURE;
  }

  system_context_t *sys_ctx = (system_context_t *)malloc(sizeof(system_context_t));
  if (!sys_ctx) {
    WARNING_LOG("mem fail, malloc system_context_t fail");
    return EXIT_FAILURE;
  }
	uv_loop_t *loop = uv_loop_new();
  sys_ctx->loop = loop;
  loop->data = sys_ctx;

  init_deleted_list(sys_ctx);

  const char *ip = "0.0.0.0";
  const int port = g_conf.listen_port;
  uv_tcp_t *server = server_listen(ip, port, loop);
  if (!server) {
    DEBUG_LOG("Listen error %s", uv_err_name(uv_last_error(loop)));
    return EXIT_FAILURE;
  }
  DEBUG_LOG("listening %s:%d", ip, port);
  uv_run(loop, UV_RUN_DEFAULT);
  return EXIT_SUCCESS;
}
