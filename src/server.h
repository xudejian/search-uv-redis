#ifndef __ADS_SERVER_H_
#define __ADS_SERVER_H_

#include <uv.h>

#define REQUEST_BUF_SIZE 8192
#define RESPONSE_BUF_SIZE 65535
typedef struct conn_ctx_t {
  uv_work_t req;
  uv_write_t res;
  uv_stream_t client;

  uv_buf_t request_buf;
  uv_buf_t response_buf;
  char _request_buf[REQUEST_BUF_SIZE];
  char _response_buf[RESPONSE_BUF_SIZE];

  int status;
  void *data;
} conn_ctx_t;

extern uv_tcp_t * server_listen(const char *ip, int port, uv_loop_t *loop);

#endif
