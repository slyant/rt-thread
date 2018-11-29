#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#include <na_queue.h>

#define ENTITY_TYPE		sysinfo_t

typedef struct sysinfo_s{
	unsigned int id;
	char sys_title[60];
	unsigned int open_timeout;
	unsigned int door_count;
	unsigned char key_a[6];
	unsigned char key_b[6];
	na_queue_t queue;
}ENTITY_TYPE;

int sysinfo_get_by_id(ENTITY_TYPE *e, int id);
int sysinfo_get_all(na_queue_t * q);
int sysinfo_add(ENTITY_TYPE * e);
int sysinfo_del(int id);
int sysinfo_del_all(void);
int sysinfo_update(ENTITY_TYPE * e);
void sysinfo_free_queue(na_queue_t *h);
void sysinfo_print_queue(na_queue_t *q);

#endif

