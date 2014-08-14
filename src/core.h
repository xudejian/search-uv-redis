#ifndef __SERVER_CORE_H_
#define __SERVER_CORE_H_

#include <signal.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "define.h"
#include "server.h"

typedef struct tag_Conf_t
{
	int unexpect_exit;
	int magic;
	int merge_weight;

	// socket
	int read_tmout;
	int write_tmout;
	short listen_port;                 //ÕìÌý¶Ë¿Ú

	// other
	long free_size;

	int pidfd;
	short server_group_id;
}Conf_t;

int server_init();
int handle_search(conn_ctx_t *ctx);
void server_ctx_clean(conn_ctx_t *ctx);

#endif
