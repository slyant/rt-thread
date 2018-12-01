#ifndef __loginfo_H__
#define __loginfo_H__

#include <na_queue.h>

typedef struct loginfo_s{
	unsigned int id;
	unsigned int date_time;
	unsigned int card_num;
	char log_inf[255];
	na_queue_t queue;
}loginfo_t;

int loginfo_get_by_id(loginfo_t *e, int id);
int loginfo_get_all(na_queue_t * q);
int loginfo_add(loginfo_t * e);
int loginfo_del(int id);
int loginfo_del_all(void);
int loginfo_update(loginfo_t * e);
void loginfo_free_queue(na_queue_t *h);
void loginfo_print_queue(na_queue_t *q);
int loginfo_bind(sqlite3_stmt * stmt,void * arg);
int loginfo_queue_bind(sqlite3_stmt * stmt,void * arg);
void loginfo_foreach(na_queue_t *q, void (*foreach_handle)(loginfo_t *e));

#endif
