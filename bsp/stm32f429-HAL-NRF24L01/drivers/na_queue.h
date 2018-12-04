#ifndef _NA_QUEUE_H_
#define _NA_QUEUE_H_

#include <stddef.h>

typedef struct na_queue_s na_queue_t;

struct na_queue_s {
  na_queue_t * prev;
  na_queue_t * next;
};


/* ��ʼ������ */
#define na_queue_init(q)                      \
  (q)->prev = (q);        \
  (q)->next = (q)

/* �ж϶����Ƿ�Ϊ�� */
#define na_queue_empty(h) \
  ((h) == (h)->prev)

/* ��ͷ����ڵ� */
#define na_queue_insert_head(h,x)     \
  (x)->next = (h)->next;              \
  (x)->next->prev = (x);              \
  (x)->prev = (h);                    \
  (h)->next = (x)

#define na_queue_insert_after na_queue_insert_head

/* ��ĩβ����ڵ� */
#define na_queue_insert_tail(h,x)      \
  (x)->prev = (h)->prev;               \
  (x)->prev->next = x;                 \
  (x)->next = h;                       \
  (h)->prev = x 


/* ͷָ���Ӧ��ͷ�ڵ� */
#define na_queue_head(h) (h)->next

/* ���һ���ڵ� */
#define na_queue_last(h) (h)->prev


#define na_queue_sentinel(h) (h)

/*��һ���ڵ�*/
#define na_queue_next(q) (q)->next

/* ǰһ���ڵ� */
#define na_queue_prev(q) (q)->prev

/* �Ƴ�һ���ڵ� */
#define na_queue_remove(x)             \
  (x)->next->prev = (x)->prev;         \
  (x)->prev->next = (x)->next;         \
  (x)->prev = NULL;                    \
  (x)->next = NULL

/* �з�һ������
 * h ͷָ��
 * q ��Ҫ��ֵ�ͷָ��
 * n �����ɺ�����һ�����е�ͷָ��
 */
#define na_queue_split(h,q,n)           \
    (n)->prev = (h)->prev;              \
    (n)->prev->next = n;                \
    (n)->next = q;                      \
    (h)->prev = (q)->prev;              \
    (h)->prev->next = h;                \
    (q)->prev = n;

/* �ϲ��������� */
#define na_queue_add(h,n)               \
  (h)->prev->next = (n)->next;          \
  (n)->next->prev = (h)->prev;          \
  (h)->prev = (n)->prev;                \
  (h)->prev->next = (h);

/* ���ݶ���ָ�룬�õ������˶���ָ��Ľṹ��
 * q ����ָ��
 * type ���ص���������
 * link �������ж�Ӧ�Ķ���������
 */
#define na_queue_data(q, type, link)   \
    (type *) ((unsigned char *) q - offsetof(type, link))

/* �����м�ڵ� */
na_queue_t *
na_queue_middle(na_queue_t * queue);

/* �Զ������� */
void na_queue_sort(na_queue_t *queue,int (*cmp)(const na_queue_t *, const na_queue_t *));

/*���������нڵ������
 *q:����İ����������͵Ľṹ��ָ��
 *s:���е��ڱ�ָ��
 *type:�������еĽṹ������
 *link:�����ڽṹ���е�����
*/
#define na_queue_foreach(q,s,type,link)          \
  na_queue_t * _head_ =NULL;                         \
  for(_head_=na_queue_head(s),q=na_queue_data(_head_,type,link);_head_!=s;_head_=na_queue_next(_head_),q=na_queue_data(_head_,type,link))

// add by hc
#define NA_QUEUE_INIT(name) {&(name),&(name)}
#define na_queue_is_last(head,node) ((node)->next == (head))
#define na_queue_for_each(pos,head) \
    for(pos = (head)->next;pos != head;pos = pos->next)
#define na_queue_for_each_safe(pos,n,head) \
    for(pos = (head)->next,n = pos->next;pos != (head);pos = n,n = pos->next)

#endif /* _NA_QUEUE_H_ */

