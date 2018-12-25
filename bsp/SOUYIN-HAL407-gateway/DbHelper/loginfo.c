#include <rtthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <dbhelper.h>
#include <loginfo.h>

//将实体信息绑定到sqlite上下文
static int loginfo_bind_for_insert(sqlite3_stmt *stmt,int index,void *arg)
{
	int rc;
	loginfo_t e = arg;
	sqlite3_bind_int(stmt,1,e->id);
	sqlite3_bind_int(stmt,2,e->date_time);
	sqlite3_bind_int(stmt,3,e->card_num);
	sqlite3_bind_text(stmt,4,e->log_inf,strlen(e->log_inf),NULL);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将实体信息绑定到sqlite上下文
static int loginfo_bind_for_update(sqlite3_stmt *stmt,int index,void *arg)
{
	int rc;
	loginfo_t e = arg;	
	sqlite3_bind_int(stmt,1,e->id);
	sqlite3_bind_int(stmt,2,e->date_time);
	sqlite3_bind_int(stmt,3,e->card_num);
	sqlite3_bind_text(stmt,4,e->log_inf,strlen(e->log_inf),NULL);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将一条查询结果绑定到实体
int loginfo_bind(sqlite3_stmt *stmt,void *arg)
{	
	loginfo_t e = arg;
	int ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	else
	{		
		e->id = db_stmt_get_int(stmt,0);
		e->date_time = db_stmt_get_int(stmt,1);
		e->card_num = db_stmt_get_int(stmt,2);
		db_stmt_get_text(stmt,3,e->log_inf);
	}
	return ret;
}

//将查询结果绑定到队列，返回队列的成员个数
int loginfo_queue_bind(sqlite3_stmt *stmt,void *arg)
{
	record_queue_t q = arg;
	rc_queue_init(q);
	loginfo_t e;
	int ret,count = 0;
	ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	do
	{
		e = rt_calloc(1, sizeof(struct loginfo));
		if(!e)
		{
			goto CREATE_loginfo_FAIL;
		}
		
		e->id = db_stmt_get_int(stmt,0);
		e->date_time = db_stmt_get_int(stmt,1);
		e->card_num = db_stmt_get_int(stmt,2);
		db_stmt_get_text(stmt,3,e->log_inf);
		
		rc_queue_insert_tail(q,&(e->queue));
		count ++;
	}
	while((ret = sqlite3_step(stmt)) == SQLITE_ROW);
	return count;
CREATE_loginfo_FAIL:
	loginfo_free_queue(q);
	return 0;
}
//打印队列
void loginfo_print_queue(record_queue_t q)
{
	loginfo_t e = NULL;
	rc_queue_foreach(e,q,struct loginfo,queue)
	{
		rt_kprintf("id:%d\ndate_time:%d\ncard_num:%d\nlog_inf:%s\n",e->id, e->date_time, e->card_num, e->log_inf);
	}
}
//遍历队列，自定义处理方法（注意处理完成后释放队列及队列变量，使用loginfo_free_queue(q)和rt_free(q)）
void loginfo_foreach(record_queue_t q, void (*foreach_handle)(loginfo_t e))
{
	loginfo_t e = NULL;
	rc_queue_foreach(e,q,struct loginfo,queue)
	{
		foreach_handle(e);
	}
}

//获取一条记录根据记录主键，返回查询到的记录数
int loginfo_get_by_id(loginfo_t e, int id)
{
	int res = db_query_by_varpara("select * from loginfo where id=?;", loginfo_bind, e, "%d", id);
	return res;
}

//返回查询到的记录数
int loginfo_get_all(record_queue_t q)
{
    return db_query_by_varpara("select * from loginfo;",loginfo_queue_bind,q,NULL);
}

//添加一条记录，操作成功返回0
int loginfo_add(loginfo_t e)
{
  return db_nonquery_operator("insert into loginfo(id,date_time,card_num,log_inf) values (?,?,?,?);",loginfo_bind_for_insert,e);
	//return db_nonquery_by_varpara("insert into loginfo(userid,username) values (?,?);", "%d%s", e->userid, e->username);
}
//更新一条记录，操作成功返回0
int loginfo_update(loginfo_t e)
{
	return db_nonquery_operator("update loginfo set date_time=?,card_num=?,log_inf=? where id=?;",loginfo_bind_for_update,e);
}

//删除指定主键的记录，操作成功返回0
int loginfo_del(int id)
{
	return db_nonquery_by_varpara("delete from loginfo where id=?;", "%d", id);
}
FINSH_FUNCTION_EXPORT(loginfo_del, loginfo del);

//删除所有记录，操作成功返回0
int loginfo_del_all(void)
{
	return db_nonquery_operator("delete from loginfo;", 0, 0);
}
MSH_CMD_EXPORT(loginfo_del_all, loginfo del all);
//释放队列
void loginfo_free_queue(record_queue_t q)
{
    record_queue_t head = q,pos,n;
    loginfo_t e = RT_NULL;
    rc_queue_for_each_safe(pos,n,head){
        e = rc_queue_data(pos,struct loginfo,queue);
        rt_free(e);
    }
    rc_queue_init(head);
}
//遍历打印所有记录信息
static int list_all_loginfo(void)
{
	rt_kprintf("test get all loginfo\n");
	record_queue_t q = rt_calloc(1, sizeof(struct record_queue));
	int ret = loginfo_get_all(q);
	loginfo_print_queue(q);
	rt_kprintf("record(s):%d\n", ret);
	loginfo_free_queue(q);
	rt_free(q);
	return 0;
}
MSH_CMD_EXPORT(list_all_loginfo, list all loginfo);
