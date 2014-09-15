/*
PROJECT_FILE_VERSION $Id: define.h 10609 2012-03-12 03:32:20Z xudejian $
*/
#ifndef __SERVER_DEFINE_H_
#define __SERVER_DEFINE_H_

#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>

#ifdef GOOGLE_PERFTOOLS
# include <google/malloc_extension.h>
#endif

#include "zwlog.h"
//#include "ty_log.h"
//#include "ty_conf.h"
//#include "ty_net.h"
//#include "ty_sign.h"
//#include "ty_dict.h"

//#include "ty_pending_pool.h"

//#include "as_search_interface.h"

#ifndef container_of
#define container_of(ptr, type, field)                                        \
  ((type *) ((char *) (ptr) - offsetof(type, field)))
#endif

#define PROJECT_NAME 	"as_search"
#define SERVER_VERSION  "1.0"

#define DEFAULT_IP_TYPE 1

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 256
#endif

#ifndef FALSE
# define FALSE 0
#endif

#ifndef TRUE
# define TRUE 1
#endif

#define DEFAULT_LOG_PATH "../log"
#define DEFAULT_LOG_EVENT 0xff
#define DEFAULT_LOG_OTHER 0x00
#define DEFAULT_LOG_SIZE 64000000

#define GetTimeCurrent(tv) gettimeofday(&tv, NULL)
#define SetTimeUsed(tused, tv1, tv2) { \
	tused  = (tv2.tv_sec-tv1.tv_sec) * 1000; \
	tused += (tv2.tv_usec-tv1.tv_usec) / 1000; \
	if (tused == 0){ tused+=1; } \
}

#define OK  			200

#define DOC_PARAM_ERROR		301
#define DATA_GATE_GET_ERROR	302
#define DATA_GATE_PUT_ERROR 303
#define DOC_NOT_EXIST		304
#define DATA_OTHER_ERROR	305
#define DOC_ALREADY_EXIST	306
#define DOC_ALREADY_HIDDEN	307
#define DATA_GATE_IN_SAVE   308

#define UI_GET_ERROR  	402
#define UI_PUT_ERROR   	403
#define SERVER_OTHER_ERROR	404
#define SERVER_INDEX_FULL	305


#define TY_GETCONFINT_M(name, val, defval) do {\
	int tmp = defval;\
	if (!ty_getconfint(&sconf, name, &tmp)) {\
		tmp = defval;\
		WARNING_LOG("set %s=%d", name, tmp);\
	}\
	val = tmp;\
} while(0)

#define TY_GETCONFINT_MOD(name, val, oldval) do {\
	int tmp = 0;\
	if (!ty_getconfint(&sconf, name, &tmp)) break;\
	if (oldval != tmp) {\
		WARNING_LOG("change %s=%d old=%d", name, tmp, oldval);\
		val = tmp;\
	}\
} while(0)

#endif
