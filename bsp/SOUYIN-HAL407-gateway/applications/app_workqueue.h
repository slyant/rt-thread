#ifndef __APP_WORKQUEUE_H__
#define __APP_WORKQUEUE_H__

#include <ipc/workqueue.h>

#define SQLITE_WORKQUEUE_MAX_LENGTH     50      //工作队列最大长度

typedef void (*void_dowork_t)(void);

struct update_door_sta
{
    rt_uint8_t group_index;
    rt_uint16_t sta;
    rt_uint32_t stamp;
};
typedef struct update_door_sta *update_door_sta_t;

void app_workqueue_startup(void);
void app_workqueue_exe_void(void_dowork_t dowork);
void app_workqueue_exe_sql(char *sql);
void app_workqueue_update_door(rt_uint8_t group_index, rt_uint16_t sta);
rt_uint16_t app_workqueue_get_length(void);

#endif
