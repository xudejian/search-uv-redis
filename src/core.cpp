#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <hiredis/hiredis.h>

#include "core.h"

extern Conf_t g_conf;

int server_init() {
	return 0;
}

void server_ctx_clean(conn_ctx_t *ctx) {
  if (ctx->data) {
    redisFree((redisContext *)(ctx->data));
    ctx->data = NULL;
  }
}

redisContext *get_redis_ctx(conn_ctx_t *ctx) {
  if (!ctx->data) {
    redisContext *redis_ctx = redisConnect("127.0.0.1", 6379);
    if (redis_ctx != NULL && redis_ctx->err) {
      DEBUG_LOG("redis connect error: %s", redis_ctx->errstr);
      redisFree(redis_ctx);
      redis_ctx = NULL;
    }
    ctx->data = (void *)redis_ctx;
  }
  return (redisContext *)(ctx->data);
}

void empty_response_buf(uv_buf_t *buf) {
  buf->base[0] = '\0';
  buf->len = 0;
}

int handle_search(conn_ctx_t *ctx) {
  struct timeval tvstart, tvend
#ifdef DEBUG_TIME
    , tv1, tv2
#endif
    ;
  u_int timeused = 0;
  int status = OK;

  GetTimeCurrent(tvstart);

  // do some other things
#ifdef DEBUG_TIME
  GetTimeCurrent(tv1);
#endif

  redisContext * redis_ctx = get_redis_ctx(ctx);
  if (redis_ctx == NULL) {
    return -1;
  }

  redisReply *reply = (redisReply *) redisCommand(redis_ctx, "GET foo");
  if (reply == NULL) {
    return -1;
  }
  DEBUG_LOG("reply %p", reply->str);

  empty_response_buf(&ctx->response_buf);
  char *buf = ctx->response_buf.base;
  int len = ctx->response_buf.len;
  int size = RESPONSE_BUF_SIZE;
  char *res = NULL;
  if (reply->str != NULL) {
    res = reply->str;
  } else {
    res = "not found";
  }

  len += snprintf(buf+len, size-len, "%s", res);
  freeReplyObject(reply);
  ctx->response_buf.len = len;
  status = 0;
#ifdef DEBUG_TIME
  GetTimeCurrent(tv2);
  SetTimeUsed(timeused, tv1, tv2);
  DEBUG_LOG("as_search TU:%u", timeused);
#endif
  return status;
}
