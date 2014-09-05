#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <hiredis/hiredis.h>

#include <map>

#include "core.h"

typedef struct ad_index_t {
  unsigned int doc_id;
  unsigned int tf;
  unsigned int weight;
  unsigned int valid;
  unsigned int area;
  unsigned int ad_id;
} ad_index_t;

int chrcount(const char *str, const char c) {
  int count = 0;
  while (*str) {
    if (*str == c) {
      count++;
    }
    str++;
  }
  return count;
}

int server_init() {
	return 0;
}

void server_ctx_clean(conn_ctx_t *ctx) {
  if (ctx->data) {
    redisFree((redisContext *)(ctx->data));
    ctx->data = NULL;
  }
}

int string_to_indexes(const char *str, ad_index_t **indexes, int max_num) {
  std::map<int, int> ad_map;
  std::map<int, int>::iterator it;

  if (!str) {
    return -1;
  }
  int count = atoi(str);
  if (count < 1) {
    return 0;
  }
  if (count > max_num) {
    count = max_num;
  }

  *indexes = (ad_index_t *) malloc(sizeof(ad_index_t) * count);
  ad_index_t *pindex = *indexes;
  const char *p, *pend;

#define MOVE_AFTER(ptr, c)       \
  while (*ptr && *ptr != c) {    \
    ptr++;                       \
  }                              \
  if (!*ptr) {                   \
    free(*indexes);              \
    return -1;                   \
  }                              \
  ptr++;                         \

#define NEXT_ITEM(ptr)           \
  MOVE_AFTER(ptr, '|');          \
  if (ptr > pend) {              \
    break;                       \
  }                              \

  p = str;
  MOVE_AFTER(p, '@');

  int i=0;
  while (i<count && *p) {
    pend = p;
    MOVE_AFTER(pend, ';');

    pindex[i].doc_id = atoi(p);
    NEXT_ITEM(p);
    pindex[i].tf = atoi(p);
    NEXT_ITEM(p);
    pindex[i].weight = atoi(p);
    NEXT_ITEM(p);
    //pindex[i].valid = atoi(p);
    NEXT_ITEM(p);
    //pindex[i].area = atoi(p);
    NEXT_ITEM(p);
    pindex[i].ad_id = atoi(p);

    it=ad_map.find(pindex[i].ad_id);
    if (it == ad_map.end()) {
      ad_map[pindex[i].ad_id] = i;
      i++;
    }
    p = pend;
  }
  return i;
#undef NEXT_ITEM
#undef MOVE_AFTER
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

const char *str_index(const char *str, const char c, int num) {
  while (*str) {
    if (*str == c) {
      if (num == 0) {
        return str;
      }
      num--;
    }
    str++;
  }
  return NULL;
}

#ifdef DEBUG
#define EOL "\n"
#else
#define EOL
#endif

int handle_search(conn_ctx_t *ctx) {
  char key[128];
  int i, num, count = 0;
  const char *p;
  ad_index_t *indexes = NULL;
  redisReply *reply = NULL;

  memset(&ctx->response.head, 0, sizeof(ctx->response.head));
  char *buf = ctx->response.buf;
  int len = 0;

  DEBUG_LOG("req %s", ctx->request.params.query);

  redisContext * redis_ctx = get_redis_ctx(ctx);
  if (redis_ctx == NULL) {
    goto fail;
  }

  reply = (redisReply *) redisCommand(redis_ctx, "GET slot_%s", ctx->request.params.query);
  if (reply == NULL) {
    goto fail;
  }
  if (reply->str == NULL) {
    freeReplyObject(reply);
    goto fail;
  }

  strcpy(key, "_");
  i = strlen(reply->str);
  if (i > (int)sizeof(key) - 1) {
    i = sizeof(key) - 1;
  }

  ctx->response.head.data.type = atoi(reply->str);

  strncpy(key + 1, reply->str, i);
  key[i+1] = '\0';
  p = str_index((const char *)key, '_', 1);
  if (p) {
    ctx->response.head.data.width = atoi(p+1);
  }
  p = str_index((const char *)key, '_', 2);
  if (p) {
    ctx->response.head.data.height = atoi(p+1);
  }
  num = 4;
  p = str_index((const char *)key, '_', 3);
  if (p) {
    num = atoi(p+1);
    *(char *)p = '\0';
  }
  ctx->response.head.data.capacity = num;

  p = str_index((const char *)key, '_', 4);
  if (p) {
    ctx->response.head.data.tpl = atoi(p+1);
  }

  DEBUG_LOG("get index %s", key);
  reply = (redisReply *) redisCommand(redis_ctx, "GET %s", key);
  if (reply == NULL) {
    goto fail;
  }
  count = string_to_indexes(reply->str, &indexes, num);
  freeReplyObject(reply);

  for (i=0; i<count; i++) {
    reply = (redisReply *) redisCommand(redis_ctx, "GET meterial_%u", indexes[i].ad_id);
    if (reply == NULL) {
      continue;
    }
    DEBUG_LOG("get ad %u", indexes[i].ad_id);
    len += snprintf(buf+len, RESPONSE_BUF_SIZE-len, ",%s"EOL, reply->str);
    ctx->response.head.return_num++;
    ctx->response.head.total_num++;
    ctx->response.head.res_num++;
  }
  if (indexes) {
    free(indexes);
    indexes = NULL;
  }

  ctx->response.head.len = len;
  return count;

fail:
  len = snprintf(buf, RESPONSE_BUF_SIZE, "/*fail*/"EOL);
  ctx->response.head.len = len;
  return -1;
}
