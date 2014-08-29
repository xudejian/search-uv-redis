#ifndef __SERVER_CORE_H_
#define __SERVER_CORE_H_

#include <signal.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "define.h"
#include "server.h"

int server_init();
int handle_search(conn_ctx_t *ctx);
void server_ctx_clean(conn_ctx_t *ctx);

#endif
