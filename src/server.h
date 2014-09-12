#ifndef __ADS_SERVER_H_
#define __ADS_SERVER_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <uv.h>

typedef std::unordered_map<long int, bool> deleted_map;
#define MAX_QUERY_WORD_LEN 12
#define MAX_TEMPLATE_NAME_LEN	32

typedef struct {
	int magic;
	char  query[MAX_QUERY_WORD_LEN];
	long int slot_id;
	u_short page_no;
	u_short rn;
	u_short sort;
	u_short other;
	char  tn[MAX_TEMPLATE_NAME_LEN];
	char need_merge;
	char need_pb;
} query_params_t;

typedef struct {
	u_int type;
	u_int width;
	u_int height;
	u_int capacity;
	u_int tpl;
} upstream_response_head_data_t;

typedef struct {
	u_int status;
	u_int return_num;					//返回结果条数
	u_int res_num;						//返回结果条数
	u_int total_num;					//搜索到结果总数
	u_int len;
  upstream_response_head_data_t data;
} upstream_response_head_t;

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

  struct {
    upstream_response_head_t head;
    char buf[RESPONSE_BUF_SIZE];
  } response;
  uv_buf_t response_buf;

  int status;
  void *data;
} conn_ctx_t;

extern uv_tcp_t * server_listen(const char *ip, int port, uv_loop_t *loop);

typedef struct {
  uv_loop_t *loop;
  uv_timer_t timer_deleted_list;
  deleted_map list_0;
  deleted_map list_1;
  int cur_deleted_list;
  long long int deleted_list_updated_at;
} system_context_t;

#endif
