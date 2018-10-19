#include <rtthread.h>
#ifndef __NRF_DATA_H__

typedef char    BYTE;
typedef uint8_t BOOL;  

#define u8	uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define TRUE	1
#define FALSE	0
#define NULL	0

#define ACK				1
#define NO_ACK			0

#define MAC_LEN				(5)
#define PAYLOAD_LEN			(32)
#define PALYOAD_HEADER_LEN	(sizeof(PayloadHeader))
#define PAYLOAD_DATA_LEN	(PAYLOAD_LEN-PALYOAD_HEADER_LEN)

#define DATA_BUF_COUNT		(50)
#define MAX_NODE_NUM		(32) 	//�ڵ���
#define MAX_HOP_NUM			(MAX_NODE_NUM)

#define DEFAULT_CHANNEL		(124)
#define GATEWAY_ADDR		(1)
#define ASSIGN_ADDR_START	(31)
#define ASSIGN_ADDR_END		(ASSIGN_ADDR_START+MAX_NODE_NUM)

#define MAX_FAILED_COUNT	(3)		//����ʧ�ܴ���
#define MAX_RETRY_COUNT		(5)
#define MAX_ACK_TIME		(100)
#define HEART_PACK_TIME 	(100)	//������ɨ����(10ms*100)

#define MULTI_PACK_INTERVAL	(10000)	//�����ظ������,��λ(10��)
#ifdef USING_NRF24L01_GATEWAY
	#define HEART_LINK_INTERVAL (20000)	//�������������,��λ(20��)
#else
	#define HEART_LINK_INTERVAL (25000)	//���������ճ�ʱʱ��,��λ(25��)
#endif
#define WAIT_CONNECT_INTERVAL (20000)  //�ȴ����Ӽ��,��λ(20��)

enum LINKSTATUS{DISCONNECT,WAIT_CONNECT,CONNECTED,WAIT_CHANGE};
enum PAYLOADSTATUS
{
IDLE,
WAIT_USE,
WAIT_SEND,
WAIT_RETRY,
WAIT_SEND_RETRY,
SEND_FAILED,
SEND_SUCCESS,
SEND_BROADCAST,
WAIT_ACK,
WAIT_RECV,
RECV_FAILED,
WAIT_HANDLER
};
enum PACKAGETYPE
{
DEL_NODE,		//0
ASSIGN_ADDR,	//1
START_LINK,		//2
BUILD_LINK,		//3
BUILD_LINK_ACK,	//4
CHECK_LINK,		//5
CHECK_LINK_ACK,	//6
APP_DATA,		//7
APP_DATA_ACK	//8
};

#define PACKAGE_BROADCAST	0x02
#define PACKAGE_SUCCESS   	0x01
#define PACKAGE_FAILED    	0x00

typedef void (*send_callback_t)(u8 src_addr,u8 dst_addr,u8 pack_type,u16 num,u8 *buf,u8 status);
typedef void (*recv_callback_t)(u8 src_addr,u8 dst_addr,u8 pack_type,u8 *buf);

#pragma pack(1)
typedef struct 
{
	u32 PID;     //����ID
	u32 SID;     //����payload ID
	u8 the_addr; //���η��͵�ַ
	u8 src_addr; //Դ���͵�ַ
	u8 dst_addr; //Ŀ�ĵ�ַ
	u8 ass_addr; //�������ַ
	u8 hop_addr; //��һ����ַ
	u8 hop_count;//��Ծ����
	u8 pack_type;//���ݰ�����
}PayloadHeader,*LPayloadHeader;//payloadͷ�ṹ,����Ϊ15

typedef struct
{
	PayloadHeader header;    //payloadͷ
	u8 buf[PAYLOAD_DATA_LEN];//���ݻ�����
}Payload,*LPayload;//payload�ṹ 

typedef struct
{
	u16 time_count;		 //��ʱ����
	u8 status;         	 //���ݰ�����״̬
	u8 isBroadcast;		 //�Ƿ�㲥
	u8 send_failed_count;//����ʧ�ܴ���
	u8 retry_count;		 //��ǰ���ݰ��ط���
	u8 pipe_num;
	send_callback_t SendCallBack;//�ص�
	Payload payload;     //���ݰ�
}DATA_BUF,*LPDATA_BUF;//ͬ�����ݰ�

typedef struct 
{
	u8 addr;	//�ڵ��ַ
	u8 path[MAX_HOP_NUM];//·����
}RoutrTable,*LRoutrTable;//·�ɱ�

typedef struct
{
	u8 set_tag;
	u32 time;  
	u16 RID;
	u8 the_addr;
	u8 src_addr;
	u8 dst_addr;
	u8 pack_type;
}HANDLEMULTIPACK;//�����ظ��İ�

typedef struct 
{
	u8 idx;
	u8 addr;
}PathTable;//���ҽڵ����·��

#ifdef USING_NRF24L01_GATEWAY
typedef struct
{
	u32 shift_failed_count; //�л�ʧ�ܼ���
	u32 shift_count;        //�л��ɹ�������
	u32 recv_count;         //�������ݰ���
	u32 pwd;
	u8 addr;           		//��ǰ��ַ
	u8 assign_addr;			//�����ַ
	u8 channel;             //��ǰ�ŵ�
	u8 heart_status;  		//������״̬
	u8 node_mac[MAC_LEN];
}NODESTATUS;//�ڵ�״̬�ṹ

typedef struct
{
	u8 addr;
	u8 count;	//������
}LOST_CONNECTION;//ͳ�Ƶ��ߴ���

typedef struct 
{
	u32 link_time;
	u8 addr;	//�ڵ��ַ
}HeartTable;//������

typedef struct
{
	u32 tag;
	u8 assign_addr;		   		//�����ַ
}NRFSTOREDATA,*LPNRFSTOREDATA;
#endif

#ifdef USING_NRF24L01_NODE
typedef struct
{
	u32 shift_failed_count; //�л�ʧ�ܼ���
	u32 shift_count;        //�л��ɹ�������
	u32 recv_count;         //�������ݰ���
	u32 pwd;
	u8 addr;           		//��ǰ��ַ
	u8 route_addr;
	u8 channel;             //��ǰ�ŵ�
	u8 link_status;  		//������״̬
	u8 send_failed_count;
	u8 node_mac[MAC_LEN];
}NODESTATUS;//�ڵ�״̬�ṹ

typedef struct
{
	u32 tag;
	u32 pwd;
	u8 addr;		   		//�ڵ��ַ
}NRFSTOREDATA,*LPNRFSTOREDATA;

#endif

#pragma pack()

typedef void (*on_connect_callback_t)(void* args);
typedef void (*on_disconnect_callback_t)(void* args);

#endif
