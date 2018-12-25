#ifndef __DOORINFO_H__
#define __DOORINFO_H__

#include <record_queue.h>

struct doorinfo{
	unsigned int id;
	unsigned int status;
	unsigned int card_num;
	struct record_queue queue;
};
typedef struct doorinfo *doorinfo_t;

int doorinfo_get_by_id(doorinfo_t *e, int id);
int doorinfo_get_all(record_queue_t q);
int doorinfo_add(doorinfo_t *e);
int doorinfo_del(int id);
int doorinfo_del_all(void);
int doorinfo_update(doorinfo_t *e);
void doorinfo_free_queue(record_queue_t h);
void doorinfo_print_queue(record_queue_t q);
int doorinfo_bind(sqlite3_stmt *stmt,void *arg);
int doorinfo_queue_bind(sqlite3_stmt *stmt,void *arg);
void doorinfo_foreach(record_queue_t q, void (*foreach_handle)(doorinfo_t e));

#endif
