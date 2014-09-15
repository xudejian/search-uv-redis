#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <hiredis/hiredis.h>
#include <unordered_map>

#include "define.h"
#include "deleted_list.h"

static long int get_deleted_list_updated_time();

void check_deleted_list_update(uv_timer_t *timer, int status)
{
  DEBUG_LOG("check deleted list update");
  long int at = 0;
  system_context_t *sctx = container_of(timer, system_context_t, timer_deleted_list);
  at = get_deleted_list_updated_time();
  if (at != sctx->deleted_list_updated_at) {
    if (!load_deleted_list(sctx)) {
      sctx->deleted_list_updated_at = at;
    }
  }

  uv_timer_set_repeat(timer, 60000);
  uv_timer_again(timer);
}

int init_deleted_list(system_context_t *sctx)
{
  new (&sctx->list_0) deleted_map();
  new (&sctx->list_1) deleted_map();
  sctx->cur_deleted_list = 0;
  sctx->deleted_list_updated_at = get_deleted_list_updated_time();
  load_deleted_list(sctx);
  uv_timer_init(sctx->loop, &sctx->timer_deleted_list);
  uv_timer_start(&sctx->timer_deleted_list, check_deleted_list_update, 60000, 0);
  return 0;
}

static long int get_deleted_list_updated_time()
{
  long int rv = 0;
  redisContext *redis = redisConnect("127.0.0.1", 6379);
  if (redis != NULL && redis->err) {
    WARNING_LOG("redis connect error: %s", redis->errstr);
    redisFree(redis);
    redis = NULL;
    return -1;
  }

  redisReply *reply = (redisReply *) redisCommand(redis, "get ad_deleted_docs_version");
  if (reply == NULL) {
    WARNING_LOG("redis get ad deleted docs error: %s", redis->errstr);
    redisFree(redis);
    redis = NULL;
    return -1;
  }

  rv = atol(reply->str);

  freeReplyObject(reply);
  reply = NULL;
  redisFree(redis);
  redis = NULL;
  return rv;
}

int load_deleted_list(system_context_t *sctx)
{
  redisContext *redis = redisConnect("127.0.0.1", 6379);
  if (redis != NULL && redis->err) {
    WARNING_LOG("redis connect error: %s", redis->errstr);
    redisFree(redis);
    redis = NULL;
    return -1;
  }

  redisReply *reply = (redisReply *) redisCommand(redis, "ZRANGE ad_deleted_docs 0 -1");
  if (reply == NULL) {
    WARNING_LOG("redis get ad deleted docs error: %s", redis->errstr);
    redisFree(redis);
    redis = NULL;
    return -1;
  }

  if (reply->type != REDIS_REPLY_ARRAY) {
    WARNING_LOG("redis ad deleted docs should be an array");
    freeReplyObject(reply);
    redisFree(redis);
    redis = NULL;
    return -1;
  }

  if (sctx->cur_deleted_list) {
    sctx->list_0.clear();
  } else {
    sctx->list_1.clear();
  }
  long int id = 0;
  for (size_t i=0; i < reply->elements; i++) {
    if (!reply->element[i]->str) {
      continue;
    }
    id = atol(reply->element[i]->str);
    DEBUG_LOG("redis get ad deleted docs: %ld", id);
    if (sctx->cur_deleted_list) {
      sctx->list_0[id] = true;
    } else {
      sctx->list_1[id] = true;
    }
  }
  sctx->cur_deleted_list = !sctx->cur_deleted_list;
  DEBUG_LOG("cur_deleted_list: %d", sctx->cur_deleted_list);
  freeReplyObject(reply);
  reply = NULL;
  redisFree(redis);
  redis = NULL;
  return 0;
}

bool is_deleted_adid(uv_loop_t *loop, long int adid)
{
  system_context_t *sctx = (system_context_t *)loop->data;
  if (sctx->cur_deleted_list) {
    return sctx->list_1.find(adid) != sctx->list_1.end();
  }
  return sctx->list_0.find(adid) != sctx->list_0.end();
}
