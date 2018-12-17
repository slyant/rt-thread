#include <rtthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <dbhelper.h>
#include <doorinfo.h>

//将实体信息绑定到sqlite上下文
static int doorinfo_bind_for_insert(sqlite3_stmt * stmt,int index,void * arg)
{
	int rc;
	doorinfo_t * e = arg;
	sqlite3_bind_int(stmt,1,e->id);
	sqlite3_bind_int(stmt,2,e->status);
	sqlite3_bind_int(stmt,3,e->card_num);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将实体信息绑定到sqlite上下文
static int doorinfo_bind_for_update(sqlite3_stmt * stmt,int index,void * arg)
{
	int rc;
	doorinfo_t * e = arg;	
	sqlite3_bind_int(stmt,1,e->id);
	sqlite3_bind_int(stmt,2,e->status);
	sqlite3_bind_int(stmt,3,e->card_num);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将一条查询结果绑定到实体
int doorinfo_bind(sqlite3_stmt * stmt,void * arg)
{	
	doorinfo_t * e = arg;
	int ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	else
	{		
		e->id = db_stmt_get_int(stmt,0);
		e->status = db_stmt_get_int(stmt,1);
		e->card_num = db_stmt_get_int(stmt,2);
	}
	return ret;
}

//将查询结果绑定到队列，返回队列的成员个数
int doorinfo_queue_bind(sqlite3_stmt * stmt,void * arg)
{
	na_queue_t * q = arg;
	na_queue_init(q);
	doorinfo_t * e;
	int ret,count = 0;
	ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	do
	{
		e = rt_calloc(sizeof(doorinfo_t),1);
		if(!e)
		{
			goto CREATE_doorinfo_FAIL;
		}
		
		e->id = db_stmt_get_int(stmt,0);
		e->status = db_stmt_get_int(stmt,1);
		e->card_num = db_stmt_get_int(stmt,2);
		
		na_queue_insert_tail(q,&(e->queue));
		count ++;
	}
	while((ret = sqlite3_step(stmt)) == SQLITE_ROW);
	return count;
CREATE_doorinfo_FAIL:
	doorinfo_free_queue(q);
	return 0;
}
//打印队列
void doorinfo_print_queue(na_queue_t *q)
{
	doorinfo_t * e = NULL;
	na_queue_foreach(e,q,doorinfo_t,queue)
	{
		rt_kprintf("id:%d\nstatus:%d\ncard_num:%d\n",e->id, e->status, e->card_num);
	}
}
//遍历队列，自定义处理方法（注意处理完成后释放队列及队列变量，使用doorinfo_free_queue(q)和rt_free(q)）
void doorinfo_foreach(na_queue_t *q, void (*foreach_handle)(doorinfo_t *e))
{
	doorinfo_t * e = NULL;
	na_queue_foreach(e,q,doorinfo_t,queue)
	{
		foreach_handle(e);
	}
}

//获取一条记录根据记录主键，返回查询到的记录数
int doorinfo_get_by_id(doorinfo_t *e, int id)
{
	int res = db_query_by_varpara("select * from doorinfo where id=?;", doorinfo_bind, e, "%d", id);
	return res;
}

//返回查询到的记录数
int doorinfo_get_all(na_queue_t * q)
{
    return db_query_by_varpara("select * from doorinfo;",doorinfo_queue_bind,q,NULL);
}

//添加一条记录，操作成功返回0
int doorinfo_add(doorinfo_t * e)
{
  return db_nonquery_operator("insert into doorinfo(id,status,card_num) values (?,?,?);",doorinfo_bind_for_insert,e);
	//return db_nonquery_by_varpara("insert into doorinfo(userid,username) values (?,?);", "%d%s", e->userid, e->username);
}
//更新一条记录，操作成功返回0
int doorinfo_update(doorinfo_t * e)
{
	return db_nonquery_operator("update doorinfo set status=?,card_num=? where id=?;",doorinfo_bind_for_update,e);
}

//删除指定主键的记录，操作成功返回0
int doorinfo_del(int id)
{
	return db_nonquery_by_varpara("delete from doorinfo where id=?;", "%d", id);
}
FINSH_FUNCTION_EXPORT(doorinfo_del, doorinfo del);

//删除所有记录，操作成功返回0
int doorinfo_del_all(void)
{
	return db_nonquery_operator("delete from doorinfo;", 0, 0);
}
MSH_CMD_EXPORT(doorinfo_del_all, doorinfo del all);
//释放队列
void doorinfo_free_queue(na_queue_t *q)
{
    na_queue_t *head = q,*pos,*n;
    doorinfo_t *e = RT_NULL;
    na_queue_for_each_safe(pos,n,head){
        e = na_queue_data(pos,doorinfo_t,queue);
        rt_free(e);
    }
    na_queue_init(head);
}
//遍历打印所有记录信息
static int list_all_doorinfo(void)
{
	rt_kprintf("test get all doorinfo\n");
	na_queue_t *q = rt_calloc(sizeof(doorinfo_t), 1);
	int ret = doorinfo_get_all(q);
	doorinfo_print_queue(q);
	rt_kprintf("record(s):%d\n", ret);
	doorinfo_free_queue(q);
	rt_free(q);
  return 0;
}
MSH_CMD_EXPORT(list_all_doorinfo, list all doorinfo);
