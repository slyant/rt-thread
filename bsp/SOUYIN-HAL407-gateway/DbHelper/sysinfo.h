#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#include <na_queue.h>
#include <sqlite3.h>

typedef struct sysinfo_s{
	unsigned int id;
	char sys_title[60];
	unsigned int open_timeout;
	unsigned int node_count;
	unsigned int door_count;
	unsigned char key_a[6];
	unsigned char key_b[6];
	na_queue_t queue;
}sysinfo_t;

int sysinfo_get_by_id(sysinfo_t *e, int id);
int sysinfo_get_all(na_queue_t * q);
int sysinfo_add(sysinfo_t * e);
int sysinfo_del(int id);
int sysinfo_del_all(void);
int sysinfo_update(sysinfo_t * e);
void sysinfo_free_queue(na_queue_t *h);
void sysinfo_print_queue(na_queue_t *q);
int sysinfo_bind(sqlite3_stmt * stmt,void * arg);
int sysinfo_queue_bind(sqlite3_stmt * stmt,void * arg);
void sysinfo_foreach(na_queue_t *q, void (*foreach_handle)(sysinfo_t *e));

#endif
