#include <rtthread.h>
#include <dfs_posix.h>
#include <dbhelper.h>
#include <sysinfo.h>

#define SYSINFO_ID	1
const char* SYS_TITLE = "武汉公交自助收银柜管理系统 V2.0\0";
const unsigned char FACTORY_KEY_A[] = {0x25, 0xFD, 0xC4, 0x96, 0xAA, 0x06};
const unsigned char FACTORY_KEY_B[] = {0xA5, 0xF2, 0x3D, 0x90, 0xF7, 0x43};

static int create_sqlite_db(void)
{
	int ret = 0;
	int fd = open(DB_NAME, O_RDONLY);
	if(fd<0)
	{
		const char *sysinfo_sql = "CREATE TABLE sysinfo(		\
				id int not null,				\
				sys_title varchar(60),		\
				open_timeout int,					\
				door_count int,						\
				key_a blob(6),						\
				key_b blob(6));";			
		
		ret = db_create_database(sysinfo_sql);
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
	e.door_count = 8;
	rt_memcpy(e.key_a,FACTORY_KEY_A,sizeof(FACTORY_KEY_A));
	rt_memcpy(e.key_a,FACTORY_KEY_B,sizeof(FACTORY_KEY_B));
	return sysinfo_add(&e);
}
MSH_CMD_EXPORT(init_data, init sqlite db data);

int app_sqlite_init(void)
{	
	db_helper_init();
	return 0;
}
INIT_APP_EXPORT(app_sqlite_init);

