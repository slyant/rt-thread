#ifndef __CARDINFO_H__
#define __CARDINFO_H__

#include <record_queue.h>

struct cardinfo{
	unsigned int num;
	unsigned int id;
	char pwd[17];
	unsigned int type;
	struct record_queue queue;
};
typedef struct cardinfo *cardinfo_t;

int cardinfo_get_by_num(cardinfo_t e, int num);
int cardinfo_get_by_cardid(cardinfo_t e, int card_id);
int cardinfo_get_by_type(record_queue_t q, int type);
int cardinfo_count_by_num(int num);
int cardinfo_count_by_cardid(int card_id);
int cardinfo_count_by_type(int type);
int cardinfo_get_all(record_queue_t q);
int cardinfo_add(cardinfo_t e);
int cardinfo_del(int id);
int cardinfo_del_all(void);
int cardinfo_update(cardinfo_t e);
void cardinfo_free_queue(record_queue_t h);
void cardinfo_print_queue(record_queue_t q);
int cardinfo_bind(sqlite3_stmt *stmt,void *arg);
int cardinfo_queue_bind(sqlite3_stmt *stmt,void *arg);
void cardinfo_foreach(record_queue_t q, void(*foreach_handle)(cardinfo_t e));

#endif
