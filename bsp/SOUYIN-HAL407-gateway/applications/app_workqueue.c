#include <rtthread.h>
#include <app_config.h>

static struct rt_workqueue *wq = RT_NULL;
static rt_uint16_t door_sta[GROUP_MAX_COUNT];

rt_uint16_t app_workqueue_get_length(void)
{
    rt_uint16_t length = 0;
    if(wq != RT_NULL)
    {
        struct rt_list_node *node, *next;
        rt_enter_critical();
        for (node = wq->work_list.next; node != &(wq->work_list); node = next)
        {
            next = node->next;
            length++;
        }      
        rt_exit_critical();        
    }
    return length;
}

static void sqlite_dowork(struct rt_work *work, void *work_data)
{
    char *sql = (char *)work_data;
    int rc = db_nonquery_by_varpara(sql, RT_NULL);
    rt_kprintf("sqlite_dowork(rc=%d):%s\n", rc, sql);
    rt_free(sql);
    rt_free(work);
}

static void void_dowork(struct rt_work *work, void *work_data)
{
    void_dowork_t dowork = (void_dowork_t)work_data;
    if(dowork != RT_NULL)
        dowork();
    rt_free(work);
}

static void update_door_dowork(struct rt_work *work, void *work_data)
{
    rt_uint16_t i, tag = 0x0001;
    update_door_sta_t update_door_sta = (update_door_sta_t)work_data;
    if(wq != RT_NULL)
    {        
        for(i = 0; i < DOOR_MAX_COUNT; i++)
        {
            rt_uint8_t sta_old = (door_sta[update_door_sta->group_index] & (tag << i)) > 0 ? 1:0;
            rt_uint8_t sta_new = (update_door_sta->sta & (tag << i)) > 0 ? 1:0;
            if(sta_old == 1 && sta_new == 0)
            {//关门
                rt_uint16_t id = GET_GLOBAL_ID(update_door_sta->group_index, i);
                doorinfo_t doorinfo = rt_calloc(1, sizeof(struct doorinfo));
                if(doorinfo_get_by_id(doorinfo, id) > 0)
                {
                    if(doorinfo->status == DOOR_STA_INIT || doorinfo->status == DOOR_STA_OPEN_1 || doorinfo->status == DOOR_STA_OPEN_2)
                    {
                        doorinfo->status = doorinfo->status + 1;
                        //sql
                        char *sql = rt_calloc(1, 128);
                        if(doorinfo->status == DOOR_STA_LOCK)
                        { 
                            rt_sprintf(sql, "update doorinfo set status=%d,lock_stamp=%d where id=%d;", doorinfo->status, update_door_sta->stamp, doorinfo->id);
                        }
                        else if(doorinfo->status == DOOR_STA_CLOSE_1)
                        {
                            rt_sprintf(sql, "update doorinfo set status=%d,close1_stamp=%d where id=%d;", doorinfo->status, update_door_sta->stamp, doorinfo->id);                            
                        }
                        else// if(doorinfo->status == DOOR_STA_CLOSE_2)
                        {
                            rt_sprintf(sql, "update doorinfo set status=%d,close2_stamp=%d where id=%d;", doorinfo->status, update_door_sta->stamp, doorinfo->id);                            
                        }
                        int rc = db_nonquery_by_varpara(sql, RT_NULL);
                        rt_kprintf("update_door_dowork(rc=%d):%s\n", rc, sql);
                        rt_free(sql);       //更新柜门状态
                    }
                }
                rt_free(doorinfo);
            }
        }        
    }
    door_sta[update_door_sta->group_index] = update_door_sta->sta;
    rt_free(update_door_sta);
    rt_free(work);
}

void app_workqueue_exe_sql(char *sql)
{
    if(wq != RT_NULL)
    {
        char *temp = rt_calloc(1, rt_strlen(sql) + 1);  //释放多余长度的字符串空间
        rt_strncpy(temp, sql, rt_strlen(sql));
        rt_free(sql);
        if(app_workqueue_get_length() < SQLITE_WORKQUEUE_MAX_LENGTH)
        {
            struct rt_work *work = rt_calloc(1, sizeof(struct rt_work));
            rt_work_init(work, sqlite_dowork, (void*)temp);
            rt_workqueue_dowork(wq, work);    
        }
    }
}

void app_workqueue_exe_void(void_dowork_t dowork)
{
    if(wq != RT_NULL)
    {
        if(app_workqueue_get_length() < SQLITE_WORKQUEUE_MAX_LENGTH)
        {
            struct rt_work *work = rt_calloc(1, sizeof(struct rt_work));
            rt_work_init(work, void_dowork, (void*)dowork);
            rt_workqueue_dowork(wq, work);    
        }
    }
}

void app_workqueue_update_door(rt_uint8_t group_index, rt_uint16_t sta)
{    
    if(wq != RT_NULL)
    {
        if(app_workqueue_get_length() < SQLITE_WORKQUEUE_MAX_LENGTH)
        {
            struct rt_work *work = rt_calloc(1, sizeof(struct rt_work));
            update_door_sta_t update_door_sta = rt_calloc(1, sizeof(struct update_door_sta));
            update_door_sta->group_index = group_index;
            update_door_sta->sta = sta;
            update_door_sta->stamp = sys_status.get_datetime(RT_NULL);
            rt_work_init(work, update_door_dowork, (void*)update_door_sta);
            rt_workqueue_dowork(wq, work);    
        }
    }
}

void app_workqueue_startup(void)
{
    if(wq == RT_NULL)
    {
        rt_memset(door_sta, 0x00, GROUP_MAX_COUNT);
        wq = rt_workqueue_create("twq", 1024*20, 25);
    }
}
