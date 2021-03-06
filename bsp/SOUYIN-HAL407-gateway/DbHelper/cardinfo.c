#include <rtthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <dbhelper.h>
#include <cardinfo.h>

//将实体信息绑定到sqlite上下文
static int cardinfo_bind_for_insert(sqlite3_stmt * stmt,int index,void * arg)
{
	int rc;
	cardinfo_t * e = arg;
	sqlite3_bind_int(stmt,1,e->num);
	sqlite3_bind_int(stmt,2,e->id);
	sqlite3_bind_text(stmt,3,e->pwd,strlen(e->pwd),NULL);
	sqlite3_bind_int(stmt,4,e->type);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将实体信息绑定到sqlite上下文
static int cardinfo_bind_for_update(sqlite3_stmt * stmt,int index,void * arg)
{
	int rc;
	cardinfo_t * e = arg;	
	sqlite3_bind_int(stmt,1,e->id);
	sqlite3_bind_text(stmt,2,e->pwd,strlen(e->pwd),NULL);
	sqlite3_bind_int(stmt,3,e->type);
	sqlite3_bind_int(stmt,4,e->num);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		return rc;
	return SQLITE_OK;
}
//将一条查询结果绑定到实体
int cardinfo_bind(sqlite3_stmt * stmt,void * arg)
{	
	cardinfo_t * e = arg;
	int ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	else
	{		
		e->num = db_stmt_get_int(stmt,0);
		e->id = db_stmt_get_int(stmt,1);
		db_stmt_get_text(stmt,2,e->pwd);
		e->type = db_stmt_get_int(stmt,3);
	}
	return ret;
}

//将查询结果绑定到队列，返回队列的成员个数
int cardinfo_queue_bind(sqlite3_stmt * stmt,void * arg)
{
	na_queue_t * q = arg;
	na_queue_init(q);
	cardinfo_t * e;
	int ret,count = 0;
	ret = sqlite3_step(stmt);
	if(ret != SQLITE_ROW)
	{
		return 0;
	}
	do
	{
		e = rt_calloc(sizeof(cardinfo_t),1);
		if(!e)
		{
			goto CREATE_cardinfo_FAIL;
		}
		
		e->num = db_stmt_get_int(stmt,0);
		e->id = db_stmt_get_int(stmt,1);
		db_stmt_get_text(stmt,2,e->pwd);
		e->type = db_stmt_get_int(stmt,3);
		
		na_queue_insert_tail(q,&(e->queue));
		count ++;
	}
	while((ret = sqlite3_step(stmt)) == SQLITE_ROW);
	return count;
CREATE_cardinfo_FAIL:
	cardinfo_free_queue(q);
	return 0;
}
//打印队列
void cardinfo_print_queue(na_queue_t *q)
{
	cardinfo_t * e = NULL;
	na_queue_foreach(e,q,cardinfo_t,queue)
	{
		rt_kprintf("num:%d\nid:%d\npwd:%s\ntype:%d\n",e->num, e->id, e->pwd, e->type);
	}
}
//遍历队列，自定义处理方法（注意处理完成后释放队列及队列变量，使用cardinfo_free_queue(q)和rt_free(q)）
void cardinfo_foreach(na_queue_t *q, void (*foreach_handle)(cardinfo_t *e))
{
	cardinfo_t * e = NULL;
	na_queue_foreach(e,q,cardinfo_t,queue)
	{
		foreach_handle(e);
	}
}

//获取一条记录根据记录主键，返回查询到的记录数
int cardinfo_get_by_id(cardinfo_t *e, int id)
{
	int res = db_query_by_varpara("select * from cardinfo where num=?;", cardinfo_bind, e, "%d", id);
	return res;
}

//返回查询到的记录数
int cardinfo_get_all(na_queue_t * q)
{
    return db_query_by_varpara("select * from cardinfo;",cardinfo_queue_bind,q,NULL);
}

//添加一条记录，操作成功返回0
int cardinfo_add(cardinfo_t * e)
{
  return db_nonquery_operator("insert into cardinfo(num,id,pwd,type) values (?,?,?,?);",cardinfo_bind_for_insert,e);
	//return db_nonquery_by_varpara("insert into cardinfo(userid,username) values (?,?);", "%d%s", e->userid, e->username);
}
//更新一条记录，操作成功返回0
int cardinfo_update(cardinfo_t * e)
{
	return db_nonquery_operator("update cardinfo set id=?,pwd=?,type=? where num=?;",cardinfo_bind_for_update,e);
}

//删除指定主键的记录，操作成功返回0
int cardinfo_del(int id)
{
	return db_nonquery_by_varpara("delete from cardinfo where num=?;", "%d", id);
}
FINSH_FUNCTION_EXPORT(cardinfo_del, cardinfo del);

//删除所有记录，操作成功返回0
int cardinfo_del_all(void)
{
	return db_nonquery_operator("delete from cardinfo;", 0, 0);
}
MSH_CMD_EXPORT(cardinfo_del_all, cardinfo del all);
//释放队列
void cardinfo_free_queue(na_queue_t *q)
{
    na_queue_t *head = q,*pos,*n;
    cardinfo_t *e = RT_NULL;
    na_queue_for_each_safe(pos,n,head){
        e = na_queue_data(pos,cardinfo_t,queue);
        rt_free(e);
    }
    na_queue_init(head);
}
//遍历打印所有记录信息
static int list_all_cardinfo(void)
{
	rt_kprintf("test get all cardinfo\n");
	na_queue_t *q = rt_calloc(sizeof(cardinfo_t), 1);
	int ret = cardinfo_get_all(q);
	cardinfo_print_queue(q);
	rt_kprintf("record(s):%d\n", ret);
	cardinfo_free_queue(q);
	rt_free(q);
  return 0;
}
MSH_CMD_EXPORT(list_all_cardinfo, list all cardinfo);
