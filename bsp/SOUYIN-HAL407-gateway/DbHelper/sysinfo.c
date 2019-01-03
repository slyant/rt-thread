#include <rtthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <dbhelper.h>
#include <sysinfo.h>

//将实体信息绑定到sqlite上下文
static int sysinfo_bind_for_insert(sqlite3_stmt *stmt,int index,void *arg)
{
	int rc;
	sysinfo_t e = arg;
	sqlite3_bind_int(stmt,1,e->id);
	sqlite3_bind_text(stmt,2,e->sys_title,strlen(e->sys_title),NULL);
	sqlite3_bind_int(stmt,3,e->en_driver_card);
	sqlite3_bind_int(stmt,4,e->open_timeout);
	sqlite3_bind_int(stmt,5,e->node_count);
	sqlite3_bind_int(stmt,6,e->door_count);
	sqlite3_bind_blob(stmt,7,e->key_a,sizeof(e->key_a),NULL);
	sqlite3_bind_blob(stmt,8,e->key_b,sizeof(e->key_b),NULL);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将实体信息绑定到sqlite上下文
static int sysinfo_bind_for_update(sqlite3_stmt *stmt,int index,void *arg)
{
	int rc;
	sysinfo_t e = arg;
	sqlite3_bind_text(stmt,1,e->sys_title,strlen(e->sys_title),NULL);
	sqlite3_bind_int(stmt,2,e->en_driver_card);
	sqlite3_bind_int(stmt,3,e->open_timeout);
	sqlite3_bind_int(stmt,4,e->node_count);
	sqlite3_bind_int(stmt,5,e->door_count);
	sqlite3_bind_blob(stmt,6,e->key_a,sizeof(e->key_a),NULL);
	sqlite3_bind_blob(stmt,7,e->key_b,sizeof(e->key_b),NULL);
	sqlite3_bind_int(stmt,8,e->id);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将一条查询结果绑定到实体
int sysinfo_bind(sqlite3_stmt *stmt,void *arg)
{	
	sysinfo_t e = arg;
	int ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	else
	{		
		e->id = db_stmt_get_int(stmt,0);
		db_stmt_get_text(stmt,1,e->sys_title);
		e->en_driver_card = db_stmt_get_int(stmt,2);
		e->open_timeout = db_stmt_get_int(stmt,3);
		e->node_count = db_stmt_get_int(stmt,4);
		e->door_count = db_stmt_get_int(stmt,5);
		db_stmt_get_blob(stmt,6,e->key_a);
		db_stmt_get_blob(stmt,7,e->key_b);
	}
	return 1;
}

//将查询结果绑定到队列，返回队列的成员个数
int sysinfo_queue_bind(sqlite3_stmt *stmt,void *arg)
{
	record_queue_t q = arg;
	rc_queue_init(q);
	sysinfo_t e;
	int ret,count = 0;
	ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	do
	{
		e = rt_calloc(1, sizeof(struct sysinfo));
		if(!e)
		{
			goto CREATE_sysinfo_FAIL;
		}
		
		e->id = db_stmt_get_int(stmt,0);
		db_stmt_get_text(stmt,1,e->sys_title);
		e->en_driver_card = db_stmt_get_int(stmt,2);
		e->open_timeout = db_stmt_get_int(stmt,3);
		e->node_count = db_stmt_get_int(stmt,4);
		e->door_count = db_stmt_get_int(stmt,5);
		db_stmt_get_blob(stmt,6,e->key_a);
		db_stmt_get_blob(stmt,7,e->key_b);
		
		rc_queue_insert_tail(q,&(e->queue));
		count ++;
	}
	while((ret = sqlite3_step(stmt)) == SQLITE_ROW);
	return count;
CREATE_sysinfo_FAIL:
	sysinfo_free_queue(q);
	return 0;
}
//打印队列
void sysinfo_print_queue(record_queue_t q)
{
	sysinfo_t e = NULL;
	rc_queue_foreach(e,q,struct sysinfo,queue)
	{
		rt_kprintf("id:%d\nsys_title:%s\nen_driver_card:%d\nopen_timeout:%d\nnode_count:%d\ndoor_count:%d\n",\
		e->id, e->sys_title, e->en_driver_card, e->open_timeout,e->node_count, e->door_count);
		rt_kprintf("keya:%02x%02x%02x%02x%02x%02x\nkeyb:%02x%02x%02x%02x%02x%02x",\
		e->key_a[0],e->key_a[1],e->key_a[2],e->key_a[03],e->key_a[04],e->key_a[5],\
		e->key_b[0],e->key_b[1],e->key_b[2],e->key_b[3],e->key_b[4],e->key_b[5]);
	}
}
//遍历队列，自定义处理方法（注意处理完成后释放队列及队列变量，使用sysinfo_free_queue(q)和rt_free(q)）
void sysinfo_foreach(record_queue_t q, void (*foreach_handle)(sysinfo_t e))
{
	sysinfo_t e = NULL;
	rc_queue_foreach(e,q,struct sysinfo,queue)
	{
		foreach_handle(e);
	}
}

//获取一条记录根据记录主键，返回查询到的记录数
int sysinfo_get_by_id(sysinfo_t e, int id)
{
	return db_query_by_varpara("select * from sysinfo where id=?;", sysinfo_bind, e, "%d", id);
}

int sysinfo_get_count_by_id(int id)
{
	return db_query_count_result("select count(*) from sysinfo where id=?;", "%d", id);
}

//返回查询到的记录数
int sysinfo_get_all(record_queue_t q)
{
    return db_query_by_varpara("select * from sysinfo;",sysinfo_queue_bind,q,NULL);
}

//添加一条记录，操作成功返回0
int sysinfo_add(sysinfo_t e)
{
  return db_nonquery_operator("insert into sysinfo(id,sys_title,en_driver_card,open_timeout,node_count,door_count,key_a,key_b) values (?,?,?,?,?,?,?,?);",sysinfo_bind_for_insert,e);
	//return db_nonquery_by_varpara("insert into sysinfo(userid,username) values (?,?);", "%d%s", e->userid, e->username);
}
//更新一条记录，操作成功返回0
int sysinfo_update(sysinfo_t e)
{
	return db_nonquery_operator("update sysinfo set sys_title=?,en_driver_card=?,open_timeout=?,node_count=?,door_count=?,key_a=?,key_b=? where id=?;",sysinfo_bind_for_update,e);
}

//删除指定主键的记录，操作成功返回0
int sysinfo_del(int id)
{
	return db_nonquery_by_varpara("delete from sysinfo where id=?;", "%d", id);
}
FINSH_FUNCTION_EXPORT(sysinfo_del, sysinfo del);

//删除所有记录，操作成功返回0
int sysinfo_del_all(void)
{
	return db_nonquery_operator("delete from sysinfo;", 0, 0);
}
MSH_CMD_EXPORT(sysinfo_del_all, sysinfo del all);
//释放队列
void sysinfo_free_queue(record_queue_t q)
{
    record_queue_t head = q,pos,n;
    sysinfo_t e = RT_NULL;
    rc_queue_for_each_safe(pos,n,head)
	{
        e = rc_queue_data(pos,struct sysinfo,queue);
        rt_free(e);
    }
    rc_queue_init(head);
}
//遍历打印所有记录信息
int list_all_sysinfo(void)
{
	rt_kprintf("test get all sysinfo\n");
	record_queue_t q = rt_calloc(1, sizeof(struct record_queue));
	int ret = sysinfo_get_all(q);
	sysinfo_print_queue(q);
	rt_kprintf("\nrecord(s):%d\n", ret);
	sysinfo_free_queue(q);
	rt_free(q);
	return 0;
}
MSH_CMD_EXPORT(list_all_sysinfo, list all sysinfo);
