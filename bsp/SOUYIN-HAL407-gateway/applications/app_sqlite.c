#include <rtthread.h>
#include <dfs_posix.h>
#include <db_include.h>
#include <app_config.h>

#define APP_SQLITE_DEBUG	1

static int create_sqlite_db(void)
{
	int ret = 0;
	int fd = open(DB_NAME, O_RDONLY);
	if(fd<0)
	{
		const char *sysinfo_sql = "CREATE TABLE sysinfo(		\
				id int PRIMARY KEY NOT NULL,		\
				sys_title varchar(60),				\
				en_driver_card int,					\
				open_timeout int,					\
				node_count int,						\
				door_count int,						\
				key_a blob(6),						\
				key_b blob(6));";			
		const char *cardinfo_sql = "CREATE TABLE cardinfo(		\
				num int PRIMARY KEY NOT NULL,		\
				id int,								\
				pwd varchar(17),					\
				type int);";
		const char *doorinfo_sql = "CREATE TABLE doorinfo(		\
				id int PRIMARY KEY NOT NULL,		\
				status int,							\
				card_num int,                       \
                time_stamp int);";
		const char *loginfo_sql = "CREATE TABLE loginfo(		\
				id int PRIMARY KEY NOT NULL,		\
				date_time int,						\
				card_num int,						\
				log_inf nvarchar(255));";		
		ret = db_create_database(sysinfo_sql);
		if(ret==0)
		{
			ret = db_create_database(cardinfo_sql);
		}
		if(ret==0)
		{
			ret = db_create_database(doorinfo_sql);
		}
		if(ret==0)
		{
			ret = db_create_database(loginfo_sql);
		}
		return ret;
	}	
	else
	{		
		close(fd);
		rt_kprintf("The database has already existed!\n");
	}	
	return -1;
}
MSH_CMD_EXPORT(create_sqlite_db, create sqlite db);

static int init_data(void)
{
    //初始化系统配置表
	sysinfo_t e = rt_calloc(1, sizeof(struct sysinfo));
	e->id = SYSINFO_KEY_ID;
	rt_strncpy(e->sys_title, INIT_SYS_TITLE, rt_strlen(INIT_SYS_TITLE));
	e->en_driver_card = 1;
	e->open_timeout = 300;
	e->node_count = 3;
	e->door_count = 8;
	rt_memcpy(e->key_a, INIT_SYS_KEY_A, INIT_KEY_LEN);
	rt_memcpy(e->key_b, INIT_SYS_KEY_B, INIT_KEY_LEN);
	rt_memcpy(sys_config.keya, INIT_SYS_KEY_A, INIT_KEY_LEN);
	rt_memcpy(sys_config.keyb, INIT_SYS_KEY_B, INIT_KEY_LEN);
	if(sysinfo_add(e) != 0)
    {//仅调试
        unlink(DB_NAME);
        while(1) rt_thread_mdelay(500);
    }
	rt_free(e);
    //初始化柜门表
    doorinfo_t d = rt_calloc(1, sizeof(struct doorinfo));
    for(int i = 0; i < (GROUP_MAX_COUNT * DOOR_MAX_COUNT); i++)
    {
        d->id = i;
        d->card_num = 0;
        d->status = 0;
        d->time_stamp = 0;
        doorinfo_add(d);
    }
    rt_free(d);
	return RT_EOK;
}
MSH_CMD_EXPORT(init_data, init sqlite db data);

void app_sqlite_init(void)
{	
	db_helper_init();
	//创建数据库的例子
	create_sqlite_db();
	
	if(sysinfo_get_count_by_id(SYSINFO_KEY_ID)==0)
	{//如果id=SYSINFO_DB_KEY_ID的记录不存在，则添加
		init_data();
	}	
}
