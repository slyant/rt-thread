#ifndef __cardinfo_H__
#define __cardinfo_H__

#include <na_queue.h>

typedef struct cardinfo_s{
	unsigned int num;
	unsigned int id;
	char pwd[16];
	unsigned int type;
	na_queue_t queue;
}cardinfo_t;

int cardinfo_get_by_num(cardinfo_t *e, int num);
int cardinfo_get_by_cardid(cardinfo_t *e, int card_id);
int cardinfo_get_by_type(na_queue_t *q, int type);
int cardinfo_count_by_num(int num);
int cardinfo_count_by_cardid(int card_id);
int cardinfo_count_by_type(int type);
int cardinfo_get_all(na_queue_t * q);
int cardinfo_add(cardinfo_t * e);
int cardinfo_del(int id);
int cardinfo_del_all(void);
int cardinfo_update(cardinfo_t * e);
void cardinfo_free_queue(na_queue_t *h);
void cardinfo_print_queue(na_queue_t *q);
int cardinfo_bind(sqlite3_stmt * stmt,void * arg);
int cardinfo_queue_bind(sqlite3_stmt * stmt,void * arg);
void cardinfo_foreach(na_queue_t *q, void (*foreach_handle)(cardinfo_t *e));

#endif
