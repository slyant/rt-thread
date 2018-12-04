#ifndef __QR_CLIENT_H__
#define __QR_CLIENT_H__
#include <rtthread.h>
struct qr_client
{
    rt_device_t device;
    char *recv_buffer;
	char *last_buffer;
    rt_size_t recv_bufsz;
    rt_size_t cur_recv_len;
    rt_sem_t rx_notice;
	rt_mq_t qr_mq;
    rt_thread_t parser;
};
typedef struct qr_client *qr_client_t;
int qr_device_init(const char *dev_name, rt_size_t recv_bufsz, rt_size_t recv_msgs);
int qr_mq_recv(char* qr_code, int size);

#endif
