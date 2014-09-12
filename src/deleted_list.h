#ifndef __DELETED_LIST_H_
#define __DELETED_LIST_H_

#include "server.h"

int init_deleted_list(system_context_t *sctx);
int load_deleted_list(system_context_t *sctx);
bool is_deleted_adid(uv_loop_t *loop, long int adid);

#endif
