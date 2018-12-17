#ifndef _CMD_PROCESS_H
#define _CMD_PROCESS_H

#define NOTIFY_TOUCH_PRESS      0X01   //����������֪ͨ
#define NOTIFY_TOUCH_RELEASE  0X03  //�������ɿ�֪ͨ
#define NOTIFY_WRITE_FLASH_OK  0X0C  //дFLASH�ɹ�
#define NOTIFY_WRITE_FLASH_FAILD  0X0D  //дFLASHʧ��
#define NOTIFY_READ_FLASH_OK  0X0B  //��FLASH�ɹ�
#define NOTIFY_READ_FLASH_FAILD  0X0F  //��FLASHʧ��
#define NOTIFY_MENU                        0X14  //�˵��¼�֪ͨ
#define NOTIFY_TIMER                       0X43  //��ʱ����ʱ֪ͨ
#define NOTIFY_CONTROL                0XB1  //�ؼ�����֪ͨ
#define NOTIFY_READ_RTC               0XF7  //��ȡRTCʱ��
#define MSG_GET_CURRENT_SCREEN 0X01//����ID�仯֪ͨ
#define MSG_GET_DATA                            0X11//�ؼ�����֪ͨ

#define PTR2U16(PTR) ((((unsigned char *)(PTR))[0]<<8)|((unsigned char *)(PTR))[1])  //�ӻ�����ȡ16λ����
#define PTR2U32(PTR) ((((unsigned char *)(PTR))[0]<<24)|(((unsigned char *)(PTR))[1]<<16)|(((unsigned char *)(PTR))[2]<<8)|((unsigned char *)(PTR))[3])  //�ӻ�����ȡ32λ����

enum CtrlType
{
	kCtrlUnknown=0x0,
	kCtrlButton=0x10,  //��ť
	kCtrlText,  //�ı�
	kCtrlProgress,  //������
	kCtrlSlider,    //������
	kCtrlMeter,  //�Ǳ�
	kCtrlDropList, //�����б�
	kCtrlAnimation, //����
	kCtrlRTC, //ʱ����ʾ
	kCtrlGraph, //����ͼ�ؼ�
	kCtrlTable, //���ؼ�
	kCtrlMenu,//�˵��ؼ�
	kCtrlSelector,//ѡ��ؼ�
	kCtrlQRCode,//��ά��
};

typedef struct
{
	unsigned char    cmd_head;  //֡ͷ

	unsigned char    cmd_type;  //��������(UPDATE_CONTROL)	
	unsigned char    ctrl_msg;   //CtrlMsgType-ָʾ��Ϣ������
	unsigned char    screen_id_high;  //������Ϣ�Ļ���ID
	unsigned char    screen_id_low;
	unsigned char    control_id_high;  //������Ϣ�Ŀؼ�ID
	unsigned char    control_id_low;
	unsigned char    control_type; //�ؼ�����

	unsigned char    param[256];//�ɱ䳤�Ȳ��������256���ֽ�

	unsigned char    cmd_tail[4];   //֡β
}CTRL_MSG,*PCTRL_MSG;

//static void ProcessMessage( PCTRL_MSG msg, unsigned short size );

//static void NotifyScreen(unsigned short screen_id);

//static void NotifyTouchXY(unsigned char press,unsigned short x,unsigned short y);

//static void NotifyButton(unsigned short screen_id, unsigned short control_id, unsigned char  state);

//static void NotifyText(unsigned short screen_id, unsigned short control_id, unsigned char *str);

//static void NotifyProgress(unsigned short screen_id, unsigned short control_id, unsigned long value);

//static void NotifySlider(unsigned short screen_id, unsigned short control_id, unsigned long value);

//static void NotifyMeter(unsigned short screen_id, unsigned short control_id, unsigned long value);

//static void NotifyMenu(unsigned short screen_id, unsigned short control_id, unsigned char  item, unsigned char  state);

//static void NotifySelector(unsigned short screen_id, unsigned short control_id, unsigned char  item);

//static void NotifyTimer(unsigned short screen_id, unsigned short control_id);

//static void NotifyReadFlash(unsigned char status,unsigned char *_data,unsigned short length);

//static void NotifyWriteFlash(unsigned char status);

//tatic void NotifyReadRTC(unsigned char year,unsigned char month,unsigned char week,unsigned char day,unsigned char hour,unsigned char minute,unsigned char second);

//static void SetTextValueInt32(unsigned short screen_id, unsigned short control_id,long value);

#endif
