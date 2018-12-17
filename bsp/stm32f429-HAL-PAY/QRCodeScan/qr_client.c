#include <rtthread.h>
#include <drv_usart.h>
#include <rtdevice.h>
#include <qr_client.h>

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME	"qr.client"
#define DBG_LEVEL           DBG_INFO
#include <rtdbg.h>

#define PRIORITY 8        
#define STACK_SIZE 512
#define TIMESLICE 10

static struct qr_client client;
static rt_uint8_t recv_run_tag = 0;

static rt_err_t qr_client_rx_ind(rt_device_t dev, rt_size_t size)
{
	if (client.device == dev && size > 0)
	{
		rt_sem_release(client.rx_notice);
	}
    return RT_EOK;
}

static void qr_client_getchar(qr_client_t client, char *ch)
{
    while (rt_device_read(client->device, 0, ch, 1) == 0)
    {
        rt_sem_control(client->rx_notice, RT_IPC_CMD_RESET, RT_NULL);
        rt_sem_take(client->rx_notice, RT_WAITING_FOREVER);
    }
}

static int qr_recv_readline(qr_client_t client)
{
    rt_size_t read_len = 0;
    char ch = 0, last_ch = 0;
    rt_bool_t is_full = RT_FALSE;

    rt_memset(client->recv_buffer, 0x00, client->recv_bufsz);
    client->cur_recv_len = 0;

    while (1)
    {
        qr_client_getchar(client, &ch);

        if (read_len < client->recv_bufsz)
        {
            client->recv_buffer[read_len++] = ch;
            client->cur_recv_len = read_len;
        }
        else
        {
            is_full = RT_TRUE;
        }

        /* is newline */
        if ((ch == 0x0A && last_ch == 0x0D) || is_full)
        {
            if (is_full)
            {
                LOG_E("read line failed. The line data length is out of buffer size(%d)!", client->recv_bufsz);
                rt_memset(client->recv_buffer, 0x00, client->recv_bufsz);
                client->cur_recv_len = 0;
                return -RT_EFULL;
            }
            break;
        }
        last_ch = ch;
    }
    return read_len;
}

static void client_parser(qr_client_t client)
{
    while(1)
    {
        if (qr_recv_readline(client) > 0)
        {            
			client->recv_buffer[client->cur_recv_len-1] = '\0';
			if(recv_run_tag==0)
			{
				LOG_W("The function qr_mq_recv not running!mq:%s", client->recv_buffer);
				continue;
			}
			if(rt_memcmp(client->recv_buffer, client->last_buffer, client->cur_recv_len)!=0)
			{
				//发送消息
				rt_mq_send(client->qr_mq, client->recv_buffer, client->cur_recv_len);
				rt_memcpy(client->last_buffer, client->recv_buffer, client->cur_recv_len);
			}
			else
			{
				LOG_W("The QR Code repetition!mq:%s", client->recv_buffer);
			}
        }
    }
}

/* initialize the client object parameters */
static int qr_client_para_init(qr_client_t client, rt_size_t recv_mqsz, rt_size_t recv_msgs)
{
	int result = RT_EOK;
    client->cur_recv_len = 0;
    client->recv_buffer = (char *) rt_calloc(1, client->recv_bufsz);
	client->last_buffer = (char *) rt_calloc(1, client->recv_bufsz);
    client->rx_notice = rt_sem_create("qr_sem", 0, RT_IPC_FLAG_FIFO);
    if (client->rx_notice == RT_NULL)
    {
        LOG_E("QR client initialize failed! at_client_notice semaphore create failed!");
        result = -RT_ENOMEM;
        goto __exit;
    }	
	client->qr_mq = rt_mq_create("qr_mq", recv_mqsz, recv_msgs, RT_IPC_FLAG_FIFO);
	if (client->qr_mq == RT_NULL)
    {
        LOG_E("QR client initialize failed! at_client_qr_mq msg queue create failed!");
        result = -RT_ENOMEM;
        goto __exit;
    }
    if (client->recv_buffer == RT_NULL)
    {
        LOG_E("QR client initialize failed! No memory for receive buffer.");
        result = -RT_ENOMEM;
        goto __exit;
    }
    client->parser = rt_thread_create("qr_pars",
                                     (void (*)(void *parameter))client_parser,
                                     client,
                                     STACK_SIZE,
                                     PRIORITY,
                                     TIMESLICE);
    if (client->parser == RT_NULL)
    {
        result = -RT_ENOMEM;
        goto __exit;
    }

__exit:
    if (result != RT_EOK)
    {
        if (client->device)
        {
            rt_device_close(client->device);
        }
        if (client->recv_buffer)
        {
            rt_free(client->recv_buffer);
        }		
		if (client->rx_notice)
        {
            rt_sem_delete(client->rx_notice);
        }
		if (client->qr_mq)
        {
            rt_mq_delete(client->qr_mq);
        }
        rt_memset(client, 0x00, sizeof(struct qr_client));
    }
    return result;
}

/**
 * AT client initialize.
 *
 * @param dev_name QR client device name
 * @param recv_bufsz the maximum number of receive buffer length
 *
 * @return 0 : initialize success
 *        -1 : initialize failed
 *        -5 : no memory
 */
int qr_client_init(const char *dev_name, rt_size_t recv_bufsz, rt_size_t recv_msgs)
{
    int result = RT_EOK;
    rt_err_t open_result = RT_EOK;

    RT_ASSERT(dev_name);
    RT_ASSERT(recv_bufsz > 0);

    client.recv_bufsz = recv_bufsz;

    /* find and open command device */
    client.device = rt_device_find(dev_name);
    if (client.device)
    {
        RT_ASSERT(client.device->type == RT_Device_Class_Char);
		uart_set_baud_rate(dev_name, BAUD_RATE_9600);
        /* using DMA mode first */
        open_result = rt_device_open(client.device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX);
        /* using interrupt mode when DMA mode not supported */
        if (open_result == -RT_EIO)
        {
            open_result = rt_device_open(client.device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
        }
        RT_ASSERT(open_result == RT_EOK);

        rt_device_set_rx_indicate(client.device, qr_client_rx_ind);
    }
    else
    {
        LOG_E("QR client initialize failed! Not find the device(%s).", dev_name);
        result = -RT_ERROR;
        goto __exit;
    }

    result = qr_client_para_init(&client, recv_bufsz, recv_msgs);
    if (result != RT_EOK)
    {
        goto __exit;
    }

__exit:
    if (result == RT_EOK)
    {
        rt_thread_startup(client.parser);
        LOG_I("QR client on device %s initialize success.", dev_name);
    }
    else
    {
        LOG_E("QR client on device %s initialize failed(%d).", dev_name, result);
    }

    return result;
}

int qr_mq_recv(char* qr_code, int size)
{	
	rt_err_t result;
	recv_run_tag = 1;
	result = rt_mq_recv(client.qr_mq, qr_code, size, RT_WAITING_FOREVER);
	return result;
}

int qr_device_init(const char *dev_name, rt_size_t recv_bufsz, rt_size_t recv_msgs)
{
	return qr_client_init(dev_name, recv_bufsz, recv_msgs);
}
