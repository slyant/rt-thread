#include <rtthread.h>
#include <stdio.h>
#include <db_include.h>
#include <sqlite_workqueue.h>

struct rt_workqueue *wq = RT_NULL;

void sqlite_work(struct rt_work *work, void *work_data)
{
    char *sql = (char *)work_data;
    int rc = db_nonquery_by_varpara(sql, RT_NULL);
    rt_kprintf("sqlite_work(rc=%d):%s\n", rc, sql);
    rt_free(sql);
    rt_free(work);    
}

void sqlite_workqueue_startup(void)
{
    wq = rt_workqueue_create("wq", 10240, 25);
    
    for(int i = 0; i < 50; i++)
    {
        struct rt_work *work_1 = rt_calloc(1, sizeof(struct rt_work));
        char *sql = rt_calloc(1, 256);
        rt_sprintf(sql, "update doorinfo set card_num=%d,status=%d where id=%d;", i, i, i);
        rt_work_init(work_1, sqlite_work, sql);
        rt_workqueue_dowork(wq, work_1);
        rt_thread_mdelay(10);
    }
}
