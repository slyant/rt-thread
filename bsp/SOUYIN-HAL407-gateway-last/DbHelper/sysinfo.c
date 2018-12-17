#include <rtthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <dbhelper.h>
#include <sysinfo.h>

//将实体信息绑定到sqlite上下文
static int sysinfo_bind_for_insert(sqlite3_stmt * stmt,int index,void * arg)
{
	int rc;
	sysinfo_t * e = arg;
	sqlite3_bind_int(stmt,1,e->id);
	sqlite3_bind_text(stmt,2,e->sys_title,strlen(e->sys_title),NULL);
	sqlite3_bind_int(stmt,3,e->open_timeout);
	sqlite3_bind_int(stmt,4,e->node_count);
	sqlite3_bind_int(stmt,5,e->door_count);
	sqlite3_bind_blob(stmt,6,e->key_a,sizeof(e->key_a),NULL);
	sqlite3_bind_blob(stmt,7,e->key_b,sizeof(e->key_b),NULL);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将实体信息绑定到sqlite上下文
static int sysinfo_bind_for_update(sqlite3_stmt * stmt,int index,void * arg)
{
	int rc;
	sysinfo_t * e = arg;
	sqlite3_bind_text(stmt,1,e->sys_title,strlen(e->sys_title),NULL);
	sqlite3_bind_int(stmt,2,e->open_timeout);
	sqlite3_bind_int(stmt,3,e->node_count);
	sqlite3_bind_int(stmt,4,e->door_count);
	sqlite3_bind_blob(stmt,5,e->key_a,sizeof(e->key_a),NULL);
	sqlite3_bind_blob(stmt,6,e->key_b,sizeof(e->key_b),NULL);
	sqlite3_bind_int(stmt,7,e->id);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将一条查询结果绑定到实体
int sysinfo_bind(sqlite3_stmt * stmt,void * arg)
{	
	sysinfo_t * e = arg;
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
		e->node_count = db_stmt_get_int(stmt,3);
		e->door_count = db_stmt_get_int(stmt,4);
		db_stmt_get_blob(stmt,5,e->key_a);
		db_stmt_get_blob(stmt,6,e->key_b);
	}
	return ret;
}

//将查询结果绑定到队列，返回队列的成员个数
int sysinfo_queue_bind(sqlite3_stmt * stmt,void * arg)
{
	na_queue_t * q = arg;
	na_queue_init(q);
	sysinfo_t * e;
	int ret,count = 0;
	ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	do
	{
		e = rt_calloc(sizeof(sysinfo_t),1);
		if(!e)
		{
			goto CREATE_sysinfo_FAIL;
		}
		
		e->id = db_stmt_get_int(stmt,0);
		db_stmt_get_text(stmt,1,e->sys_title);
		e->open_timeout = db_stmt_get_int(stmt,2);
		e->node_count = db_stmt_get_int(stmt,3);
		e->door_count = db_stmt_get_int(stmt,4);
		db_stmt_get_blob(stmt,5,e->key_a);
		db_stmt_get_blob(stmt,6,e->key_b);
		
		na_queue_insert_tail(q,&(e->queue));
		count ++;
	}
	while((ret = sqlite3_step(stmt)) == SQLITE_ROW);
	return count;
CREATE_sysinfo_FAIL:
	sysinfo_free_queue(q);
	return 0;
}
//打印队列
void sysinfo_print_queue(na_queue_t *q)
{
	sysinfo_t * e = NULL;
	na_queue_foreach(e,q,sysinfo_t,queue)
	{
		rt_kprintf("id:%d\nsys_title:%s\nopen_timeout:%d\nnode_count:%d\ndoor_count:%d\n",e->id, e->sys_title, e->open_timeout, e->node_count, e->door_count);
	}
}
//遍历队列，自定义处理方法（注意处理完成后释放队列及队列变量，使用sysinfo_free_queue(q)和rt_free(q)）
void sysinfo_foreach(na_queue_t *q, void (*foreach_handle)(sysinfo_t *e))
{
	sysinfo_t * e = NULL;
	na_queue_foreach(e,q,sysinfo_t,queue)
	{
		foreach_handle(e);
	}
}

//获取一条记录根据记录主键，返回查询到的记录数
int sysinfo_get_by_id(sysinfo_t *e, int id)
{
	int res = db_query_by_varpara("select * from sysinfo where id=?;", sysinfo_bind, e, "%d", id);
	return res;
}

//返回查询到的记录数
int sysinfo_get_all(na_queue_t * q)
{
    return db_query_by_varpara("select * from sysinfo;",sysinfo_queue_bind,q,NULL);
}

//添加一条记录，操作成功返回0
int sysinfo_add(sysinfo_t * e)
{
  return db_nonquery_operator("insert into sysinfo(id,sys_title,open_timeout,node_count,door_count,key_a,key_b) values (?,?,?,?,?,?,?);",sysinfo_bind_for_insert,e);
	//return db_nonquery_by_varpara("insert into sysinfo(userid,username) values (?,?);", "%d%s", e->userid, e->username);
}
//更新一条记录，操作成功返回0
int sysinfo_update(sysinfo_t * e)
{
	return db_nonquery_operator("update sysinfo set sys_title=?,open_timeout=?,node_count=?,door_count=?,key_a=?,key_b=? where id=?;",sysinfo_bind_for_update,e);
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
void sysinfo_free_queue(na_queue_t *q)
{
    na_queue_t *head = q,*pos,*n;
    sysinfo_t *e = RT_NULL;
    na_queue_for_each_safe(pos,n,head){
        e = na_queue_data(pos,sysinfo_t,queue);
        rt_free(e);
    }
    na_queue_init(head);
}
//遍历打印所有记录信息
static int list_all_sysinfo(void)
{
	rt_kprintf("test get all sysinfo\n");
	na_queue_t *q = rt_calloc(sizeof(sysinfo_t), 1);
	int ret = sysinfo_get_all(q);
	sysinfo_print_queue(q);
	rt_kprintf("record(s):%d\n", ret);
	sysinfo_free_queue(q);
	rt_free(q);
  return 0;
}
MSH_CMD_EXPORT(list_all_sysinfo, list all sysinfo);
