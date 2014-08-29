#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"
#include "define.h"

char* trim(char *str)
{
  char *p = str;
  while (*p && *p == ' ') {
    p++;
  }
  str = p;

  while (*p && *p != '\n') {
    p++;
  }
  if (*p == '\n') {
    *p = '\0';
    p--;
  }
  if (*p == '\r') {
    *p = '\0';
    p--;
  }
  return str;
}

int assign_conf_item(Conf_t *conf, char *key, int key_len, char *value)
{
  if (!strncmp(key, "listen_port", key_len)) {
    conf->listen_port = atoi(value);
    return 0;
  }
  if (!strncmp(key, "write_tmout", key_len)) {
    conf->write_tmout = atoi(value);
    return 0;
  }
  if (!strncmp(key, "read_tmout", key_len)) {
    conf->read_tmout = atoi(value);
    return 0;
  }
  if (!strncmp(key, "merge_weight", key_len)) {
    conf->merge_weight = atoi(value);
    return 0;
  }
  if (!strncmp(key, "magic", key_len)) {
    conf->magic = atoi(value);
    return 0;
  }
  if (!strncmp(key, "server_group_id", key_len)) {
    conf->server_group_id = atoi(value);
    return 0;
  }
  return -1;
}

int parse_config_line(Conf_t *conf, char *line)
{
  char *key, *value, *p;
  int key_len;

  p = key = line;
  while (*p && *p != ' ') {
    p++;
  }
  key_len = p - key;

  while (*p && *p == ' ') {
    p++;
  }
  if (*p != ':') {
    return -1;
  }
  p++;

  while (*p && *p == ' ') {
    p++;
  }
  value = p;

  return assign_conf_item(conf, key, key_len, value);
}

void init_conf(Conf_t *conf)
{
  if (NULL == conf) {
    return;
  }
  conf->listen_port = 3000;
  conf->write_tmout = 500;
  conf->read_tmout = 500;
  conf->merge_weight = 500;
  conf->magic = 0;
  conf->server_group_id = 1;
}

int load_conf(Conf_t *conf, const char *conf_filename)
{
  if (NULL == conf || NULL == conf_filename) {
    return -1;
  }

  init_conf(conf);
  FILE *fp = fopen(conf_filename, "r");
  if (fp == NULL) {
    return -1;
  }

  char buf[1024];
  while (fgets(buf, sizeof(buf), fp)) {
    parse_config_line(conf, trim(buf));
  }

  fclose(fp);
  fp = NULL;
	return 0;
}
