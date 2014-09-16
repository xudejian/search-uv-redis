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

void request_start(conn_ctx_t *ctx) {
  DEBUG_LOG("request start");
}

void request_end(conn_ctx_t *ctx) {
  ctx->request_len = 0;
  DEBUG_LOG("request end");
}

conn_ctx_t *alloc_conn_context() {
  conn_ctx_t *ctx = (conn_ctx_t *) malloc(sizeof(conn_ctx_t));
  ctx->data = NULL;
  ctx->request_len = 0;
  DEBUG_LOG("alloc conn context");
  return ctx;
}

uv_buf_t alloc_request_buf(uv_handle_t *client, size_t suggested_size) {
  conn_ctx_t *ctx = container_of(client, conn_ctx_t, client);
  DEBUG_LOG("in alloc_request_buf");
  return uv_buf_init(ctx->request.buf, REQUEST_BUF_SIZE + RESPONSE_BUF_SIZE);
}

void context_clean_cb(uv_handle_t* client) {
  conn_ctx_t *ctx = container_of(client, conn_ctx_t, client);
  server_ctx_clean(ctx);
  free(ctx);
  DEBUG_LOG("free context");
}

static void on_request(uv_stream_t *client, ssize_t nread, uv_buf_t buf);

void response_send_cb(uv_write_t *write, int status) {
  conn_ctx_t *ctx = container_of(write, conn_ctx_t, write);
  if (status < 0) {
    uv_loop_t *loop = ctx->client.loop;
    WARNING_LOG("Write error %s", uv_err_name(uv_last_error(loop)));
  }
  DEBUG_LOG("response send done");
  request_end(ctx);
  //uv_close((uv_handle_t*) &ctx->client, context_clean_cb);
}

void handle_request(uv_work_t *worker) {
  conn_ctx_t *ctx = container_of(worker, conn_ctx_t, worker);
  int rv = handle_search(ctx);
  DEBUG_LOG("done in work %d", rv);
}

void send_response(uv_work_t *worker, int status) {
  conn_ctx_t *ctx = container_of(worker, conn_ctx_t, worker);
  if (status < 0) {
    DEBUG_LOG("send response fail, maybe canceled, status %d", status);
    request_end(ctx);
    return;
  }
  uv_loop_t *loop = worker->loop;
  DEBUG_LOG("send response [%s]", ctx->response.buf);
  ctx->response_buf.base = (char *)&ctx->response;
  ctx->response_buf.len = ctx->response.head.len + sizeof(upstream_response_head_t);
  uv_write(&ctx->write, &ctx->client, &ctx->response_buf, 1, response_send_cb);
}

static void on_request(uv_stream_t *client, ssize_t nread, uv_buf_t buf) {
  if (nread == 0) {
    DEBUG_LOG("read 0");
    return;
  }

  conn_ctx_t *ctx = container_of(client, conn_ctx_t, client);
  uv_loop_t *loop = client->loop;
  if (nread < 0) {
    if (uv_last_error(loop).code != UV_EOF) {
      WARNING_LOG("Read error %s", uv_err_name(uv_last_error(loop)));
    }
    DEBUG_LOG("nread %ld < 0 and close client, code = %d",
        nread, uv_last_error(loop).code);
    uv_cancel((uv_req_t*) &ctx->worker);
    uv_close((uv_handle_t*) client, context_clean_cb);
    return;
  }

  ctx->request_len = nread;
  if (ctx->request_len > (int)REQUEST_BUF_SIZE) {
    DEBUG_LOG("read reqlen %d > %ld", ctx->request_len, REQUEST_BUF_SIZE);
    ctx->request_len = REQUEST_BUF_SIZE;
  }

  DEBUG_LOG("read %ld/%d/%ld %ld", nread, ctx->request_len, REQUEST_BUF_SIZE, buf.len);
  if (ctx->request.params.magic != sizeof(query_params_t)) {
    WARNING_LOG("read request's magic is not correct");
    uv_close((uv_handle_t*) client, context_clean_cb);
    return;
  }

  ctx->response.head.len = 0;
  uv_queue_work(loop, &ctx->worker, handle_request, send_response);
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
    uv_read_start(&ctx->client, alloc_request_buf, on_request);
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
