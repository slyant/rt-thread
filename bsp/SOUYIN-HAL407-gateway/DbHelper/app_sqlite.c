#include <rtthread.h>
#include <dfs_posix.h>
#include <dbinclude.h>

#define SYSINFO_ID	1
const char* SYS_TITLE = "武汉公交自助收银柜管理系统 V2.0";
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
	e.id = SYSINFO_ID;
	rt_strncpy(e.sys_title, SYS_TITLE, rt_strlen(SYS_TITLE));
	e.open_timeout = 60;
	e.node_count = 3;
	e.door_count = 8;
	rt_memcpy(e.key_a,DEFAULT_KEY_A,sizeof(DEFAULT_KEY_A));
	rt_memcpy(e.key_a,DEFAULT_KEY_B,sizeof(DEFAULT_KEY_B));
	return sysinfo_add(&e);
}
MSH_CMD_EXPORT(init_data, init sqlite db data);

//遍历处理
void sysinfo_for_hd(sysinfo_t* e)
{
	rt_kprintf("\nsysinfo_for_hd->\nid:%d\nsys_title:%s\nopen_timeout:%d\ndoor_count:%d\n",e->id, e->sys_title, e->open_timeout, e->door_count);
}

int app_sqlite_init(void)
{	
	db_helper_init();
/*
	//创建数据库的例子
	create_sqlite_db();
	
	// sysinfo添加一条记录的例子：
	if(db_query_count_result("select count(id) from sysinfo where id=1")==0)
	{//如果id=1的记录不存在，则添加
		init_data();
	}
	
	// 获取一条记录的例子：
	sysinfo_t sysinfo;
	sysinfo_get_by_id(&sysinfo, SYSINFO_ID);
	rt_kprintf("\nid:%d\nsys_title:%s\nopen_timeout:%d\ndoor_count:%d\n",sysinfo.id, sysinfo.sys_title, sysinfo.open_timeout, sysinfo.door_count);			
	
	// 自定义查询的例子：
	na_queue_t *q = rt_calloc(sizeof(sysinfo_t), 1);
	int rcount = db_query_by_varpara("select * from sysinfo where id=? and door_count=?", sysinfo_queue_bind, q, "%d%d", SYSINFO_ID, 8);
	rt_kprintf("\nrcount:%d\n", rcount);
	sysinfo_foreach(q, sysinfo_for_hd);
	sysinfo_free_queue(q);
	rt_free(q);

	// nodeinfo添加一条记录
	nodeinfo_t nodeinfo;
	if(db_query_count_result("select count(address) from nodeinfo where address=31")==0)
	{//如果address=31的记录不存在，则添加		
		nodeinfo.address = 31;
		rt_strncpy(nodeinfo.name, "A1", 2);
		nodeinfo_add(&nodeinfo);
	}
	else
	{//如果存在，则更新
		if(nodeinfo_get_by_id(&nodeinfo, 31)>0)
		{
			rt_strncpy(nodeinfo.name, "B1", 2);
			nodeinfo_update(&nodeinfo);
		}
	}

	// 查询
	if(nodeinfo_get_by_id(&nodeinfo, 31)>0)
	{
		rt_kprintf("address:%d\nname:%s\n", nodeinfo.address, nodeinfo.name);
	}
*/
	return 0;
}
INIT_APP_EXPORT(app_sqlite_init);

