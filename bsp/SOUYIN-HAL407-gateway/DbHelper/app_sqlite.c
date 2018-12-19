#include <rtthread.h>
#include <dfs_posix.h>
#include <db_include.h>

#define APP_SQLITE_DEBUG	1

const char* SYS_TITLE = "公交自助收银管理系统 V2.0";
const unsigned char DEFAULT_KEY_A[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const unsigned char DEFAULT_KEY_B[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static int create_sqlite_db(void)
{
	int ret = 0;
	int fd = open(DB_NAME, O_RDONLY);
	if(fd<0)
	{
		const char *sysinfo_sql = "CREATE TABLE sysinfo(		\
				id int PRIMARY KEY NOT NULL,		\
				sys_title varchar(60),				\
				open_timeout int,					\
				node_count int,						\
				door_count int,						\
				key_a blob(6),						\
				key_b blob(6));";			
		const char *cardinfo_sql = "CREATE TABLE cardinfo(		\
				num int PRIMARY KEY NOT NULL,		\
				id int,								\
				pwd varchar(16),					\
				type int);";
		const char *doorinfo_sql = "CREATE TABLE doorinfo(		\
				id int PRIMARY KEY NOT NULL,		\
				status int,							\
				card_num int);";
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
	sysinfo_t e;
	rt_memset(&e,0x00,sizeof(sysinfo_t));
	e.id = SYSINFO_DB_KEY_ID;
	rt_strncpy(e.sys_title, SYS_TITLE, rt_strlen(SYS_TITLE));
	e.open_timeout = 60;
	e.node_count = 3;
	e.door_count = 8;
	rt_memcpy(e.key_a,DEFAULT_KEY_A,sizeof(DEFAULT_KEY_A));
	rt_memcpy(e.key_b,DEFAULT_KEY_B,sizeof(DEFAULT_KEY_B));
	return sysinfo_add(&e);
}
MSH_CMD_EXPORT(init_data, init sqlite db data);

void app_sqlite_init(void)
{	
	db_helper_init();
	//创建数据库的例子
	create_sqlite_db();
	
	if(sysinfo_get_count_by_id(SYSINFO_DB_KEY_ID)==0)
	{//如果id=SYSINFO_DB_KEY_ID的记录不存在，则添加
		init_data();
	}	

	sysinfo_t sysinfo;
	int count = sysinfo_get_by_id(&sysinfo, SYSINFO_DB_KEY_ID);
	RT_ASSERT(count>0);
	#if(APP_SQLITE_DEBUG)
	rt_kprintf("\nsysinfo->\nid:%d\nsys_title:%s\nopen_timeout:%d\ndoor_count:%d\r\n",sysinfo.id, sysinfo.sys_title, sysinfo.open_timeout, sysinfo.door_count);
	#endif
}
