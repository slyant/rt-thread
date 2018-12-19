#ifndef __doorinfo_H__
#define __doorinfo_H__

#include <na_queue.h>

typedef struct doorinfo_s{
	unsigned int id;
	unsigned int status;
	unsigned int card_num;
	na_queue_t queue;
}doorinfo_t;

int doorinfo_get_by_id(doorinfo_t *e, int id);
int doorinfo_get_all(na_queue_t * q);
int doorinfo_add(doorinfo_t * e);
int doorinfo_del(int id);
int doorinfo_del_all(void);
int doorinfo_update(doorinfo_t * e);
void doorinfo_free_queue(na_queue_t *h);
void doorinfo_print_queue(na_queue_t *q);
int doorinfo_bind(sqlite3_stmt * stmt,void * arg);
int doorinfo_queue_bind(sqlite3_stmt * stmt,void * arg);
void doorinfo_foreach(na_queue_t *q, void (*foreach_handle)(doorinfo_t *e));

#endif
