#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "define.h"
#include "server.h"
#include "core.h"

#ifndef container_of
#define container_of(ptr, type, field)                                        \
  ((type *) ((char *) (ptr) - offsetof(type, field)))
#endif

conn_ctx_t *alloc_conn_context() {
  conn_ctx_t *ctx = (conn_ctx_t *) malloc(sizeof(conn_ctx_t));
  ctx->data = NULL;
  DEBUG_LOG("alloc conn context");
  return ctx;
}

uv_buf_t alloc_request_buf(uv_handle_t *handle, size_t suggested_size) {
  conn_ctx_t *ctx = container_of(handle, conn_ctx_t, client);
  ctx->request_buf.base = ctx->_request_buf;
  ctx->request_buf.len = REQUEST_BUF_SIZE;
  return ctx->request_buf;
}

void context_clean_cb(uv_handle_t* client) {
  conn_ctx_t *ctx = container_of(client, conn_ctx_t, client);
  server_ctx_clean(ctx);
  free(ctx);
  DEBUG_LOG("free context");
}

void response_send_cb(uv_write_t *res, int status) {
  if (status < 0) {
    conn_ctx_t *ctx = container_of(res, conn_ctx_t, res);
    uv_loop_t *loop = ctx->client.loop;
    WARNING_LOG("Write error %s", uv_err_name(uv_last_error(loop)));
  }
  DEBUG_LOG("response send done");
}

void handle_request(uv_work_t *req) {
  conn_ctx_t *ctx = container_of(req, conn_ctx_t, req);
  ctx->response_buf.base = ctx->_response_buf;
  ctx->response_buf.len = RESPONSE_BUF_SIZE;
  int rv = handle_search(ctx);
  DEBUG_LOG("done in work %d", rv);
}

void send_response(uv_work_t *req, int status) {
  uv_loop_t *loop = req->loop;
  if (status < 0 && uv_last_error(loop).code == UV_ECANCELED) {
    return;
  }
  conn_ctx_t *ctx = container_of(req, conn_ctx_t, req);
  DEBUG_LOG("send response ...\n[%s]", ctx->response_buf.base);
  uv_write(&ctx->res, &ctx->client, &ctx->response_buf, 1, response_send_cb);
}

void on_request(uv_stream_t *client, ssize_t nread, uv_buf_t buf) {
  conn_ctx_t *ctx = container_of(client, conn_ctx_t, client);
  uv_loop_t *loop = client->loop;
  if (nread < 0) {
    if (uv_last_error(loop).code != UV_EOF) {
      WARNING_LOG("Read error %s", uv_err_name(uv_last_error(loop)));
    }
    DEBUG_LOG("nread %ld < 0 and close client", nread);
    uv_cancel((uv_req_t*) &ctx->req);
    uv_close((uv_handle_t*) client, context_clean_cb);
    return;
  }

  buf.base[nread] = '\0';
  buf.len = nread;
  ctx->request_buf.len = nread;
  DEBUG_LOG("read %ld [%s]", nread, buf.base);

  uv_queue_work(loop, &ctx->req, handle_request, send_response);
}

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    // error!
    return;
  }

  uv_loop_t *loop = server->loop;
  conn_ctx_t *ctx = alloc_conn_context();
  uv_tcp_init(loop, (uv_tcp_t*) &ctx->client);
  if (uv_accept(server, (uv_stream_t*) &ctx->client) == 0) {
    INFO_LOG("accept once");
    uv_read_start((uv_stream_t*) &ctx->client, alloc_request_buf, on_request);
  } else {
    DEBUG_LOG("accept fail");
    uv_close((uv_handle_t*) &ctx->client, context_clean_cb);
  }
}

uv_tcp_t *server_listen(const char *ip, int port, uv_loop_t *loop) {
  uv_tcp_t *server = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, server);
  struct sockaddr_in bind_addr = uv_ip4_addr(ip, port);
  uv_tcp_bind(server, bind_addr);
  int rv = uv_listen((uv_stream_t*) server, 128, on_new_connection);
  if (rv) {
    free(server);
    return NULL;
  }

  return server;
}
