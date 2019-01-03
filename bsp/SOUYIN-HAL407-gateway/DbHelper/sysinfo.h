#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#include <record_queue.h>

struct sysinfo{
	unsigned int id;
	char sys_title[61];
	unsigned char en_driver_card;
	unsigned short open_timeout;
	unsigned char node_count;
	unsigned char door_count;
	unsigned char key_a[6];
	unsigned char key_b[6];
	struct record_queue queue;
};
typedef struct sysinfo *sysinfo_t;

int sysinfo_get_by_id(sysinfo_t e, int id);
int sysinfo_get_count_by_id(int id);
int sysinfo_get_all(record_queue_t q);
int sysinfo_add(sysinfo_t e);
int sysinfo_del(int id);
int sysinfo_del_all(void);
int sysinfo_update(sysinfo_t e);
void sysinfo_free_queue(record_queue_t h);
void sysinfo_print_queue(record_queue_t q);
int sysinfo_bind(sqlite3_stmt *stmt,void *arg);
int sysinfo_queue_bind(sqlite3_stmt *stmt,void *arg);
void sysinfo_foreach(record_queue_t q, void (*foreach_handle)(sysinfo_t e));

#endif
