#ifndef __SERVER_CONF_H_
#define __SERVER_CONF_H_

typedef struct {
	int unexpect_exit;
	int magic;
	int merge_weight;

	// socket
	int read_tmout;
	int write_tmout;
	short listen_port;                 //侦听端口

	// other
	long free_size;

	int pidfd;
	short server_group_id;
} Conf_t;

int load_conf(Conf_t *conf, const char *conf_filename);

#endif
