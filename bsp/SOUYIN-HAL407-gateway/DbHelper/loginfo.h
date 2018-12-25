#ifndef __LOGINFO_H__
#define __LOGINFO_H__

#include <record_queue.h>

struct loginfo{
	unsigned int id;
	unsigned int date_time;
	unsigned int card_num;
	char log_inf[255];
	struct record_queue queue;
};
typedef struct loginfo *loginfo_t;

int loginfo_get_by_id(loginfo_t e, int id);
int loginfo_get_all(record_queue_t q);
int loginfo_add(loginfo_t e);
int loginfo_del(int id);
int loginfo_del_all(void);
int loginfo_update(loginfo_t e);
void loginfo_free_queue(record_queue_t h);
void loginfo_print_queue(record_queue_t q);
int loginfo_bind(sqlite3_stmt *stmt,void *arg);
int loginfo_queue_bind(sqlite3_stmt *stmt,void *arg);
void loginfo_foreach(record_queue_t q, void (*foreach_handle)(loginfo_t e));

#endif
