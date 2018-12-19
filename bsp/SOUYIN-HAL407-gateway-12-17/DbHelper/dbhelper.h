#ifndef __DBHELPER_H__
#define __DBHELPER_H__
#include <sqlite3.h> 

#define DB_NAME "/spi/m.db"

#define DB_SQL_MAX_LEN 1024
void db_helper_init(void);
int db_create_database(const char* sqlstr);
//ִ��û�з��ص�SQL���
int db_nonquery_operator(const char *sqlstr,int (*bind)(sqlite3_stmt *,int index,void * arg),void *param);
//ִ��û�з��ص�SQL���Ķ�ֵ����
int db_nonquery_by_varpara(const char *sql,const char *fmt,...);
//ִ��û�з��ص�SQL���
int db_nonquery_transaction(int (*exec_sqls)(sqlite3 *db,void * arg),void *arg);
//ִ�ж�ֵ���εĲ�ѯ���
int db_query_by_varpara(const char *sql,int (*create)(sqlite3_stmt *stmt,void *arg),void *arg,const char *fmt,...);
//ִ�в�ѯ�����ز�ѯ�����������
int db_query_count_result(const char *sql);
//��sqlite3_column_blob�Ķ��η�װ
int db_stmt_get_blob(sqlite3_stmt *stmt,int index,unsigned char *out);
//��sqlite3_column_text�Ķ��η�װ
int db_stmt_get_text(sqlite3_stmt *stmt,int index,char *out);
//��sqlite3_column_int�Ķ��η�װ
int db_stmt_get_int(sqlite3_stmt *stmt,int index);
//��sqlite3_column_double�Ķ��η�װ
double db_stmt_get_double(sqlite3_stmt *stmt,int index);

#endif

