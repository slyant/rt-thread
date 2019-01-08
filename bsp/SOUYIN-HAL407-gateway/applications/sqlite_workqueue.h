#ifndef __SQLITE_WORKQUEUE_H__
#define __SQLITE_WORKQUEUE_H__

#include <ipc/workqueue.h>

extern struct rt_workqueue *wq;

void sqlite_work(struct rt_work *work, void *work_data);

#endif
