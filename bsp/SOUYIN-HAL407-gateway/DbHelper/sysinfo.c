#include <rtthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <dbhelper.h>
#include <sysinfo.h>

static int sysinfo_bind_for_insert(sqlite3_stmt * stmt,int index,void * arg)
{
	int rc;
	ENTITY_TYPE * e = arg;
	sqlite3_bind_int(stmt,1,e->id);
	sqlite3_bind_text(stmt,2,e->sys_title,strlen(e->sys_title),NULL);
	sqlite3_bind_int(stmt,3,e->open_timeout);
	sqlite3_bind_int(stmt,4,e->door_count);
	sqlite3_bind_blob(stmt,5,e->key_a,sizeof(e->key_a),NULL);
	sqlite3_bind_blob(stmt,6,e->key_b,sizeof(e->key_b),NULL);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
static int sysinfo_bind_for_update(sqlite3_stmt * stmt,int index,void * arg)
{
	int rc;
	ENTITY_TYPE * e = arg;
	sqlite3_bind_text(stmt,1,e->sys_title,strlen(e->sys_title),NULL);
	sqlite3_bind_int(stmt,2,e->open_timeout);
	sqlite3_bind_int(stmt,3,e->door_count);
	sqlite3_bind_blob(stmt,4,e->key_a,sizeof(e->key_a),NULL);
	sqlite3_bind_blob(stmt,5,e->key_b,sizeof(e->key_b),NULL);
	sqlite3_bind_int(stmt,6,e->id);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
static int sysinfo_create(sqlite3_stmt * stmt,void * arg)
{	
	ENTITY_TYPE * e = arg;
	int ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	else
	{		
		e->id = db_stmt_get_int(stmt,0);
		db_stmt_get_text(stmt,1,e->sys_title);
		e->open_timeout = db_stmt_get_int(stmt,2);
		e->door_count = db_stmt_get_int(stmt,3);
		db_stmt_get_blob(stmt,4,e->key_a);
		db_stmt_get_blob(stmt,5,e->key_b);
	}
	return ret;
}

//返回队列的成员个数
static int sysinfo_create_queue(sqlite3_stmt * stmt,void * arg)
{
	na_queue_t * q = arg;
	na_queue_init(q);
	ENTITY_TYPE * e;
	int ret,count = 0;
	ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	do
	{
		e = rt_calloc(sizeof(ENTITY_TYPE),1);
		if(!e)
		{
			goto CREATE_sysinfo_FAIL;
		}
		
		e->id = db_stmt_get_int(stmt,0);
		db_stmt_get_text(stmt,1,e->sys_title);
		e->open_timeout = db_stmt_get_int(stmt,2);
		e->door_count = db_stmt_get_int(stmt,3);
		db_stmt_get_blob(stmt,4,e->key_a);
		db_stmt_get_blob(stmt,5,e->key_b);
		
		na_queue_insert_tail(q,&(e->queue));
		count ++;
	}
	while((ret = sqlite3_step(stmt)) == SQLITE_ROW);
	return count;
CREATE_sysinfo_FAIL:
	sysinfo_free_queue(q);
	return 0;
}

void sysinfo_print_queue(na_queue_t *q)
{
	ENTITY_TYPE * e=NULL;
	na_queue_foreach(e,q,ENTITY_TYPE,queue)
	{
		rt_kprintf("id:%d\nsys_title:%s\nopen_timeout:%d\ndoor_count:%d\n",e->id, e->sys_title, e->open_timeout, e->door_count);
	}
}

//返回查询到的记录数
int sysinfo_get_by_id(ENTITY_TYPE *e, int id)
{
	int res = db_query_by_varpara("select * from sysinfo where id=?;", sysinfo_create, e, "%d", id);
	return res;
}

//返回查询到的记录数
int sysinfo_get_all(na_queue_t * q)
{
    return db_query_by_varpara("select id,sys_title,open_timeout,door_count,key_a,key_b from sysinfo;",sysinfo_create_queue,q,NULL);
}

//操作成功返回0
int sysinfo_add(ENTITY_TYPE * e)
{
  return db_nonquery_operator("insert into sysinfo(id,sys_title,open_timeout,door_count,key_a,key_b) values (?,?,?,?,?,?);",sysinfo_bind_for_insert,e);
	//return db_nonquery_by_varpara("insert into sysinfo(userid,username) values (?,?);", "%d%s", e->userid, e->username);
}

int sysinfo_update(ENTITY_TYPE * e)
{
	return db_nonquery_operator("update sysinfo set userid=?,username=? where id=?;",sysinfo_bind_for_update,e);
}

//操作成功返回0
int sysinfo_del(int id)
{
	return db_nonquery_by_varpara("delete from sysinfo where id=?;", "%d", id);
}

//操作成功返回0
int sysinfo_del_all(void)
{
	return db_nonquery_operator("delete from sysinfo;", 0, 0);
}

void sysinfo_free_queue(na_queue_t *h)
{
    na_queue_t *head = h,*pos,*n;
    ENTITY_TYPE *p = RT_NULL;
    na_queue_for_each_safe(pos,n,head){
        p = na_queue_data(pos,ENTITY_TYPE,queue);
        rt_free(p);
    }
    na_queue_init(head);
}

static void list_all_sysinfo(void)
{
	rt_kprintf("test get all sysinfo\n");
	na_queue_t *h = rt_calloc(sizeof(ENTITY_TYPE), 1);
	int ret = sysinfo_get_all(h);
	sysinfo_print_queue(h);
	rt_kprintf("record(s):%d\n", ret);
	sysinfo_free_queue(h);
	rt_free(h);
}
static void sysinfo(uint8_t argc, char **argv)
{	
	if(argc<2)
	{
		list_all_sysinfo();
		return;
	}
	else
	{
		ENTITY_TYPE *e = rt_calloc(sizeof(ENTITY_TYPE),1);
		char* cmd = argv[1];
		if(rt_strcmp(cmd,"add")==0)
		{
			rt_tick_t ticks = rt_tick_get();
			
			ticks = rt_tick_get() - ticks;
			rt_kprintf("It takes %d ticks.\n", ticks);
		}
		else if(rt_strcmp(cmd,"del")==0)
		{
			if(argc==2)
			{
				if(sysinfo_del_all()==0)
				{
					rt_kprintf("Del all record success!\n");
				}
				else
				{
					rt_kprintf("Del all record failed!\n");
				}
			}
			else
			{
				rt_uint32_t id = atol(argv[2]);
				if(sysinfo_del(id)==0)
				{
					rt_kprintf("Del record success with id:%d\n",id);
				}
				else
				{
					rt_kprintf("Del record failed with id:%d\n",id);
				}
			}
		}
		else
		{
			rt_uint32_t id = atol(argv[1]);
			if(sysinfo_get_by_id(e, id)>0)
			{
				rt_kprintf(
				"id:%d\n						\
				sys_title:%s\n			\
				open_timeout:%d\n		\
				door_count:%d\n			\
				",e->id, e->sys_title, e->open_timeout, e->door_count);				
			}
			else
			{
				rt_kprintf("no record with id:%d\n", id);
			}
		}
		rt_free(e);
	}
}
MSH_CMD_EXPORT(sysinfo, sysinfo add del query);
