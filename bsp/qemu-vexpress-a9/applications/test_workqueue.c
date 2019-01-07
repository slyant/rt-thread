#include <rtthread.h>


void work_thread(struct rt_work *work, void *work_data)
{
    char *msg = (char*)work_data;
    int i = 0;
    while(1)
    {
        rt_kprintf("%d-%s\n", i, msg);
        if(i >= 10)
            break;
        rt_thread_mdelay(2000);
    }
}

void test_workqueue(void)
{
    struct rt_workqueue *wq = rt_workqueue_create("wq", 2048, 25);
    struct rt_work wk1; 
    
    rt_kprintf("test_workqueue\n");


    rt_work_init(&wk1, work_thread, "work1");
    rt_workqueue_dowork(wq, &wk1);

    // struct rt_work wk2;
    // rt_work_init(&wk2, work_thread, "work2");
    // rt_workqueue_dowork(wq, &wk2);
}
