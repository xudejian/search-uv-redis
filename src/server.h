#ifndef __ADS_SERVER_H_
#define __ADS_SERVER_H_

#include <uv.h>

typedef struct query_params_t
{
	int magic;
	char slot_id[12];
} query_params_t;

#define REQUEST_BUF_SIZE sizeof(query_params_t)
#define RESPONSE_BUF_SIZE 65535
typedef struct conn_ctx_t {
  uv_work_t worker;
  uv_write_t write;
  uv_stream_t client;

  int request_len;
  union {
    query_params_t params;
    char buf[REQUEST_BUF_SIZE];
  } request;
  union {
    char buf[RESPONSE_BUF_SIZE];
  } _response;
  uv_buf_t response_buf;

  int status;
  void *data;
} conn_ctx_t;

extern uv_tcp_t * server_listen(const char *ip, int port, uv_loop_t *loop);

#endif
