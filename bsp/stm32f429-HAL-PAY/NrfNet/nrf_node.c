#include <rtthread.h>
#include <nrf_node.h>
#include <drv_spi_nrf24l01.h>
#include <stdio.h>

//#define USING_NRF_LOG
#ifdef USING_NRF_LOG
	#define NRF_LOG		rt_kprintf("[NRF_NODE]:");rt_kprintf
	#define NRF_LOGS	rt_kprintf
#else
	#define NRF_LOG(...)
	#define NRF_LOGS(...)
#endif

typedef char    BYTE;
typedef uint8_t BOOL;  

#define u8	uint8_t
#define u16 uint16_t
#define u32 uint32_t

enum {
    eTimLed,
    eTimLinkTimeOut,
    eTimMax,
};

//���index��Ӧ��ʱ�䳬ʱ�����㲢����1��δ��ʱ����0
#define IS_TIMEOUT_10MS(index, count)    ((g_TimArray[(index)] >= (count))?  \
                                        ((g_TimArray[(index)] = 0) == 0): 0)
static NODESTATUS NodeStatus;//�ڵ�״̬
static RoutrTable route_table[MAX_NODE_NUM];
static PathTable path_table[MAX_NODE_NUM];
static HANDLEMULTIPACK IsHandle[DATA_BUF_COUNT];
static DATA_BUF DataBuf[DATA_BUF_COUNT];//���ݻ���,��󳤶�PAYLOAD_DATA_LEN
static u32 heart_time=0,wait_time=0;
static u16 link_time=0;
static u8 isLink=0;
static u8 pipe1_mac[MAC_LEN]={0};//ͨ��1��ַ(������ڵ�)
static u8 pipe2_mac[MAC_LEN]={0xFF,0xB9,0x7F,0x55,0xB0};//ͨ��2��ַ(���нڵ�),�����ַ
static recv_callback_t RecvCallback=NULL;
static send_callback_t SendCallback=NULL;
static on_connect_callback_t OnConnectCallback=NULL;
static on_disconnect_callback_t OnDisconnectCallback=NULL;
static volatile int g_TimArray[eTimMax] = {0};
static rt_mutex_t nrf_thread_mutex_lock = RT_NULL;

static void nrf_print_nodestatus(void)
{
	int i;
	NRF_LOG("\nNodeStatus.pwd:%08X", NodeStatus.pwd);
	NRF_LOGS("\nNodeStatus.addr:%d", NodeStatus.addr);
	NRF_LOGS("\nNodeStatus.channel:%d", NodeStatus.channel);
	NRF_LOGS("\nNodeStatus.node_mac:");
	for(i=0;i<5;i++)
	{
		NRF_LOGS("%02X",NodeStatus.node_mac[i]);
	}
	NRF_LOGS("\npipe1_mac:");
	for(i=0;i<5;i++)
	{
		NRF_LOGS("%02X",pipe1_mac[i]);
	}
	NRF_LOGS("\npipe2_mac:");
	for(i=0;i<5;i++)
	{
		NRF_LOGS("%02X",pipe2_mac[i]);
	}	
	NRF_LOGS("\n\n");
}
static void NRF_SetPipe1(u8 *addr, u32 pwd)
{
	NRF_GetMacAddr(addr,0xff,pwd);//����MAC��ַ
	if(addr[0]==0xFF)
	{
		addr[0]-=1;
	}
	NRF_SetPipe1Mac(addr);
}
static void NRF_SetPipe2(u8 *addr)
{
	NRF_SetPipe2Mac(addr);
}
static u8 NRF_CmpMultiPack(const u8 src_addr,const u8 dst_addr,const u8 pack_type,const u16 ID)
{
	u8 i;
	for(i=0;i<DATA_BUF_COUNT;i++)
	{
		if(Get_RunTime()-IsHandle[i].time<rt_tick_from_millisecond(MULTI_PACK_INTERVAL))
		{
			if(ID==IsHandle[i].RID&&pack_type==IsHandle[i].pack_type&&
				src_addr==IsHandle[i].src_addr&&dst_addr==IsHandle[i].dst_addr)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}
static u8 NRF_SearchHandleTimeoutId(void)
{
	u8 i, r = 0xFF;
	for(i=0;i<DATA_BUF_COUNT;i++)
	{
		if(IsHandle[i].set_tag==0)
		{
			r = i;
			break;
		}
		else if(Get_RunTime()-IsHandle[i].time>rt_tick_from_millisecond(MULTI_PACK_INTERVAL))
		{
			r = i;
		}
	}
	return r;
}
static u8 NRF_UpdateToRoutrTablePath(u8 idx,LPayload pPayLoad)//����·����
{
	u8 k;
	
	rt_memcpy((BYTE *)route_table[idx].path,(BYTE *)pPayLoad->buf,sizeof(pPayLoad->buf));
	if(pPayLoad->header.hop_addr)//��һ����ַ��Ч
	{
		for(k=0; k<MAX_HOP_NUM; k++)//����·����
		{
			if(!route_table[idx].path[k])//�ҵ���λ
			{
				route_table[idx].path[k]=pPayLoad->header.hop_addr;//����һ����ַ��ӵ�·�������
				return TRUE;
			}
		}
	}
	return FALSE;
}

//����0,�б�����; ����1,��ӳɹ�;
static u8 NRF_AddNodeToRoutrTable(LPayload pPayLoad)//���1���ڵ㵽·�ɱ�
{
	u8 i;
	for(i=0; i<MAX_NODE_NUM; i++)//����·�ɱ�
	{
		if(route_table[i].addr==pPayLoad->header.the_addr)//�ڵ��Ѵ���
		{
			NRF_UpdateToRoutrTablePath(i,pPayLoad);//����·����
			return TRUE;
		}
	}
	for(i=0; i<MAX_NODE_NUM; i++)//û���ҵ���ַ
	{
		if(!route_table[i].addr)//�ҵ���λ
		{
			route_table[i].addr=pPayLoad->header.the_addr;//��ӵ�·�ɱ�
			NRF_UpdateToRoutrTablePath(i,pPayLoad);//����·����
			return TRUE;
		}
	}
	return FALSE;
}
static u8 NRF_MallocIdleDataBufId(void)//�õ�һ������DATABUF,��������Ϊ����
{
	u8 i;	
	for(i=0;i<DATA_BUF_COUNT;i++)
	{
		if(IDLE==DataBuf[i].status)
		{
			DataBuf[i].status=WAIT_USE;
			return i;
		}
	}
	return 0xFF;
}	
static u8 NRF_SearchDataByPackType(u8 addr,u8 type)//����addr��type����
{
	u8 i;	
	for(i=0;i<DATA_BUF_COUNT;i++)
	{
		if(addr==DataBuf[i].payload.header.dst_addr&&
			type==DataBuf[i].payload.header.pack_type)
		{
			return i;
		}
	}
	return 0xFF;
}
static void NRF_RelayPackage(LPayload pPayLoad)//ת��
{
	u8 idx;	
	if(pPayLoad->header.PID==NodeStatus.pwd)//����ID
	{
		idx=NRF_MallocIdleDataBufId();
		if(0xFF!=idx)
		{
			DataBuf[idx].status=WAIT_SEND;
			rt_memcpy((BYTE*)&DataBuf[idx].payload,(BYTE *)pPayLoad,sizeof(Payload));
			switch(pPayLoad->header.pack_type)
			{
				case START_LINK:
					DataBuf[idx].payload.header.hop_count+=1;//��Ծ������1
					DataBuf[idx].payload.header.hop_addr=DataBuf[idx].payload.header.the_addr;//��һ����ַ=�����ݰ��ķ��͵�ַ
					DataBuf[idx].payload.header.the_addr=NodeStatus.addr;//���η��͵�ַ=���ڵ��ַ
					break;
				default:
					break;
			}
		}		
		NRF_LOG("***Ready To Relay-Package***\n\n");
	}
}
static void NRF_DelNodePath(u8 node)//ɾ���ڵ�
{
	u8 i,j,k;

	for(i=0; i<MAX_NODE_NUM; i++)//����·�ɱ�
	{
		if(route_table[i].addr)
		{
			if(route_table[i].addr==node)
			{
				rt_memset((BYTE *)&route_table[i],0,sizeof(RoutrTable));//��·�ɱ�ɾ��1���ڵ�
			}
			else
			{
				for(j=0; j<MAX_HOP_NUM; j++)//����·����
				{
					if(route_table[i].path[j]==node)//�ҵ��ڵ�
					{
						route_table[i].path[j]=0;//ɾ���ڵ�
						for(k=j+1; k<MAX_HOP_NUM; k++)//��ɾ���ڵ�λ�ÿ�ʼ����·����,��������нڵ�����1λ
						{
							route_table[i].path[k-1]=route_table[i].path[k];//����
						}
					}
				}
			}
		}
	}
	for(i=0; i<MAX_NODE_NUM; i++)//·�ɱ���������
	{
		if(!route_table[i].addr)
		{
			for(j=i+1; j<MAX_NODE_NUM; j++)
			{
				if(route_table[j].addr)//�е�ַ���Ƶ���ǰ��
				{
					route_table[i].addr=route_table[j].addr;
					rt_memcpy((BYTE *)route_table[i].path,(BYTE *)route_table[j].path,sizeof(route_table[i].path));
					rt_memset((BYTE *)&route_table[j],0,sizeof(RoutrTable));//��·�ɱ�ɾ��1���ڵ�
					break;
				}
			}
		}
	}
}
static void NRF_PreSendData(const u8 dst_addr,const u8 type,const u8 *buf,send_callback_t fun)//Ԥ�������ݽӿ�
{
	static u32 id=0;
	u8 idx;
	if(dst_addr!=NodeStatus.addr)
	{
		idx=NRF_MallocIdleDataBufId();
		if(0xFF==idx)
		{
			return;
		}
		DataBuf[idx].status=WAIT_SEND;
		DataBuf[idx].SendCallBack=fun;
		DataBuf[idx].payload.header.PID=NodeStatus.pwd;
		DataBuf[idx].payload.header.SID=id++;//��ȡΨһ��ID
		DataBuf[idx].payload.header.the_addr=NodeStatus.addr;
		DataBuf[idx].payload.header.src_addr=NodeStatus.addr;
		DataBuf[idx].payload.header.dst_addr=dst_addr;
		DataBuf[idx].payload.header.ass_addr=dst_addr;
		DataBuf[idx].payload.header.hop_addr=0;
		DataBuf[idx].payload.header.hop_count=0;
		DataBuf[idx].payload.header.pack_type=type;
		rt_memcpy((BYTE *)DataBuf[idx].payload.buf,(BYTE *)buf,PAYLOAD_DATA_LEN);
		if(0xFFFFFFFF==id)
		{
			id=0;
		}
	}
}

static void NRF_HandlePackage(LPayload pPayLoad)
{
	switch(pPayLoad->header.pack_type)
	{
		case START_LINK://�ڵ�->���� ·��
			NRF_AddNodeToRoutrTable(pPayLoad);//��ӽڵ㵽·�ɱ�
			NRF_RelayPackage(pPayLoad);//ת��
			break;
		case ASSIGN_ADDR://����->�ڵ� ·��
			NRF_AddNodeToRoutrTable(pPayLoad);//��ӽڵ㵽·�ɱ�
			if(WAIT_CONNECT==NodeStatus.link_status)
			{
				if(pPayLoad->header.ass_addr>=ASSIGN_ADDR_START&&pPayLoad->header.ass_addr<ASSIGN_ADDR_END)//�Ϸ���ַ
				{
					NodeStatus.addr=pPayLoad->header.ass_addr;//��ȡ��ַ
					NodeStatus.pwd=pPayLoad->header.PID;//��ȡ����ID
					NRF_GetMacAddr(NodeStatus.node_mac,NodeStatus.addr,NodeStatus.pwd);//����MAC��ַ
					NRF_SetPipe1(pipe1_mac,NodeStatus.pwd);	
					
					if(NRF_StoreParam(&NodeStatus))
					{//���浽FLASH
						NRF_LOG("NRF Store Param Success!\n\n");
					}
					else
					{
						NRF_LOG("NRF Store Param Failed!\n\n");
					}				
					link_time=300;
					NodeStatus.link_status=DISCONNECT;
					IS_TIMEOUT_10MS(eTimLinkTimeOut, 0);
				}
			}
			else
			{
				NRF_RelayPackage(pPayLoad);//ת��
			}
			break;
		case DEL_NODE:
			NRF_DelNodePath(pPayLoad->header.ass_addr);//ɾ���ڵ�
			NRF_RelayPackage(pPayLoad);//ת��
			if(pPayLoad->header.ass_addr==NodeStatus.addr)
			{				
				link_time=300;
				NodeStatus.link_status=DISCONNECT;
				IS_TIMEOUT_10MS(eTimLinkTimeOut, 0);
				if(OnDisconnectCallback)
						OnDisconnectCallback(&NodeStatus);
			}
			break;
		default:
			if(pPayLoad->header.dst_addr==NodeStatus.addr)
			{
				u8 idx;
				switch(pPayLoad->header.pack_type)
				{
					case CHECK_LINK:
						heart_time=Get_RunTime();
						NRF_PreSendData(NodeStatus.route_addr,CHECK_LINK_ACK,(u8 *)"",SendCallback);//Ԥ�������ݽӿ�
						break;
					case BUILD_LINK:
						NRF_AddNodeToRoutrTable(pPayLoad);//��ӽڵ㵽·�ɱ�
						heart_time=Get_RunTime();
						NodeStatus.link_status=CONNECTED;
						isLink=1;
						NRF_PreSendData(NodeStatus.route_addr,BUILD_LINK_ACK,(u8 *)"",SendCallback);//Ԥ�������ݽӿ�
						break;
					case APP_DATA:
						idx=NRF_MallocIdleDataBufId();
						if(0xFF!=idx)
						{
							rt_memcpy((BYTE*)&DataBuf[idx].payload,(BYTE *)pPayLoad,PAYLOAD_LEN);
							DataBuf[idx].status=WAIT_HANDLER;
							NRF_PreSendData(pPayLoad->header.src_addr,APP_DATA_ACK,(u8 *)"",SendCallback);//Ԥ�������ݽӿ�
						}
						break;
					case APP_DATA_ACK:
						idx=NRF_SearchDataByPackType(pPayLoad->header.src_addr,APP_DATA);//����addr��type����
						if(0xFF!=idx)
						{
							DataBuf[idx].status=SEND_SUCCESS;
						}
						break;	
					default:break;
				}
			}
			else
			{
				NRF_RelayPackage(pPayLoad);//ת��
			}
			break;
	}
}
static void nrf_hw_recv_callback(rt_uint8_t pipe_num, rt_uint8_t *rxdata, rt_uint8_t length)
{
	u8 id;
	Payload *pload = (Payload *)rxdata;
	NodeStatus.recv_count++;
	
	NRF_LOG("Pipe-Recv[%d]:\r\n",pipe_num);	
	NRF_LOG("src_addr=%d,dst_addr=%d,the_addr=%d,type=%d\r\npid=%08X,sid=%d,ass_addr=%d,hop_addr=%d,hop_count=%d\n\n",
			pload->header.src_addr, pload->header.dst_addr, pload->header.the_addr, pload->header.pack_type,
			pload->header.PID, pload->header.SID, pload->header.ass_addr, pload->header.hop_addr,
			pload->header.hop_count);
	
	if(pload->header.src_addr==NodeStatus.addr)//���Լ����͵İ�,����
	{
		NRF_RxMode(NodeStatus.node_mac, NodeStatus.channel);
		return;
	}

	rt_mutex_take(nrf_thread_mutex_lock, RT_WAITING_FOREVER);
	if(ASSIGN_ADDR==pload->header.pack_type||pload->header.PID==NodeStatus.pwd)
	{
		//10���ڴ�����İ����ٴ���(�ڹ㲥ʱ���յ��ܶ��ظ��İ�)
		if(NRF_CmpMultiPack(pload->header.src_addr,pload->header.dst_addr,
							 pload->header.pack_type,pload->header.SID)==FALSE)
		{
			id=NRF_SearchHandleTimeoutId();
			if(0xFF!=id)
			{
				IsHandle[id].RID=pload->header.SID;
				IsHandle[id].time=Get_RunTime();
				IsHandle[id].src_addr=pload->header.src_addr;
				IsHandle[id].dst_addr=pload->header.dst_addr;
				IsHandle[id].pack_type=pload->header.pack_type;
				NRF_HandlePackage(pload);
			}
		}
	}
	NRF_RxMode(NodeStatus.node_mac, NodeStatus.channel);
	rt_mutex_release(nrf_thread_mutex_lock);
}
static void link_timeout_callback(void* parameter)
{
	rt_uint8_t i;
	for(i=0;i<eTimMax;i++)
	{
		g_TimArray[i]++;
	}
}
static int nrf_env_init(void)
{
	if(!NRF_Check())
	{
		NRF_LOG("NRF24L01+ Module not found!\n");
		return -1;
	}
	NRF_LOG("Find NRF24L01+ Module!\n");

	rt_memset((BYTE *)&NodeStatus,0,sizeof(NODESTATUS));//��ʼ���ڵ�״̬
	rt_memset((BYTE *)&DataBuf,0,sizeof(DATA_BUF)*DATA_BUF_COUNT);//��ʼ��������Ϊ����״̬
	rt_memset((BYTE *)&IsHandle,0,sizeof(HANDLEMULTIPACK)*DATA_BUF_COUNT);
	rt_memset((BYTE *)&route_table,0,sizeof(RoutrTable)*MAX_NODE_NUM);
	
	NRF_ReadParam(&NodeStatus);	
	NRF_GetMacAddr(NodeStatus.node_mac, NodeStatus.addr, NodeStatus.pwd);	
	NodeStatus.route_addr=GATEWAY_ADDR;//���ص�ַ
	NodeStatus.channel=DEFAULT_CHANNEL;//��ʼ�ŵ�	
	
	NRF_Config(NodeStatus.node_mac,NodeStatus.channel);//Ӳ����ʼ��
	NRF_SetPipe1(pipe1_mac,NodeStatus.pwd);
	NRF_SetPipe2(pipe2_mac);
	nrf_print_nodestatus();
	
	if(NodeStatus.addr>=ASSIGN_ADDR_START&&NodeStatus.addr<ASSIGN_ADDR_END)//�жϵ�ַ�Ƿ�Ϸ�
	{
		NodeStatus.link_status=DISCONNECT;
		link_time = 300;
		IS_TIMEOUT_10MS(eTimLinkTimeOut, 0);
		NRF_LOG("Check Address OK\n");
	}
	else
	{
		NodeStatus.link_status=WAIT_CONNECT;
		wait_time=Get_RunTime();
		NRF_LOG("Check Address Error! Wait Assign Address...\n");
	}		
	return 0;
}
static u8 NRF_SearchDataBuf(const u8 status)//����״̬Ϊstatus��DataBuf,��������
{
	u8 i;	
	for(i=0;i<DATA_BUF_COUNT;i++)
	{
		if(status==DataBuf[i].status)
		{
			return i;
		}
	}
	return 0xFF;
}
static u8 NRF_FindNodePath(u8 node)//���ҽڵ����·��
{
	u8 i,j,k,n,min,addr;

	rt_memset((BYTE *)&path_table,0,sizeof(PathTable)*MAX_NODE_NUM);
	for(i=0; i<MAX_NODE_NUM; i++)//�����ڵ��б�
	{
		if(route_table[i].addr)
		{
			if(route_table[i].addr==node)
			{
				return route_table[i].addr;//���·�� ��������
			}
			else
			{
				for(j=0; j<MAX_HOP_NUM; j++)//����·����
				{
					if(route_table[i].path[j]==node)//��·�����ҵ���ַ
					{
						n=0;
						for(k=j+1; k<MAX_HOP_NUM; k++)//����nֵ
						{
							if(route_table[i].path[k])//�е�ַ
							{
								n++;//nֵԽ�����ԽԶ
							}
						}
						for(k=0; k<MAX_NODE_NUM; k++)//����·��������
						{
							if(!path_table[k].addr)//�ҵ���λ
							{
								path_table[k].idx=n;//����nֵ
								path_table[k].addr=route_table[i].addr;//����ڵ��ַ
								break;
							}
						}
						break;	
					}
				}
			}
		}
	}
	if(!path_table[0].addr)//û�нڵ��ַ
		return 0xFF;
	
	min=path_table[0].idx;
	addr=path_table[0].addr;
	for(i=0; i<MAX_NODE_NUM; i++)//����·��������
	{
		if(path_table[i].addr)//�нڵ��ַ
		{
			if(path_table[i].idx<min)//������С������
			{
				min=path_table[i].idx;
				addr=path_table[i].addr;
			}
		}
	}
	return addr;//���ؽڵ��ַ(��һ����ַ)
}
static void NRF_FreeDataBuf(u8 id)//�ͷű�ʹ�õ�DataBuf,���ı�Ϊ����״̬
{
	if(id>(DATA_BUF_COUNT-1)||id==0xFF)
	{
		return;
	}
	rt_memset((BYTE *)&DataBuf[id],0,sizeof(DATA_BUF));
}
static void NRF_AutoLink(void)//�Զ�����
{
	static u8 iFlag=0;
	static u8 nCnt=0;	
	switch(NodeStatus.link_status)
	{
		case DISCONNECT:
			if(NodeStatus.addr>=ASSIGN_ADDR_START&&NodeStatus.addr<ASSIGN_ADDR_END)
			{
				link_time++;
				if(IS_TIMEOUT_10MS(eTimLinkTimeOut, 3000)&&!iFlag)//30s
				{
					iFlag = 1;
					if(nCnt<3)
					{						
						nCnt++;
					}
					else
					{
						nrf_env_init();
						nCnt = 0;
					}
				}
				if(link_time>300)//3��
				{
					u8 path[MAX_HOP_NUM]={0};					
					NRF_PreSendData(NodeStatus.route_addr,START_LINK,path,SendCallback);//�������ݽӿ�
					link_time=0;
				}
			}
			break;
		case WAIT_CONNECT:
			if(Get_RunTime()-wait_time>=rt_tick_from_millisecond(WAIT_CONNECT_INTERVAL))//���20��
			{
				if(NodeStatus.addr>=ASSIGN_ADDR_START&&NodeStatus.addr<ASSIGN_ADDR_END)
				{	
					NodeStatus.link_status=DISCONNECT;
					link_time=300;
					IS_TIMEOUT_10MS(eTimLinkTimeOut, 0);
					NRF_LOG("Check Address OK\n");
				}
				else
				{
					NodeStatus.link_status=WAIT_CONNECT;
					wait_time=Get_RunTime();
					nrf_env_init();
					NRF_LOG("Wait Connect Timeout! Wait Connect Resume...\n");
				}
			}
			break;
		case CONNECTED:
			if(isLink)
			{
				if(Get_RunTime()-heart_time>=rt_tick_from_millisecond(1000))//���Ӻ�ȴ�1��
				{
					isLink=0;
					if(OnConnectCallback)
						OnConnectCallback(&NodeStatus);
				}
			}
			if(Get_RunTime()-heart_time>rt_tick_from_millisecond(HEART_LINK_INTERVAL))//����25��δ�����������Ͽ�����
			{
				nCnt=0;
				iFlag=0;
				IS_TIMEOUT_10MS(eTimLinkTimeOut, 0);
				NodeStatus.link_status=DISCONNECT;
				link_time=300;
				if(OnDisconnectCallback)
					OnDisconnectCallback(&NodeStatus);			
			}				
			break;
		default:break;
	}
}

static void NRF_NrfSendData(void)//���ݻ�����״̬��������,����ǰ�ȼ�����
{
	u8 index, n;	
	index=NRF_SearchDataBuf(WAIT_SEND);
	if(0xFF==index)
	{
		index=NRF_SearchDataBuf(WAIT_SEND_RETRY);
	}
	if(0xFF!=index)//���ݻ������д���������
	{
		if(!NRF_DisturbanceDetect())//����Ƿ��������ڵ��ڷ�������
		{
			u8 send_mac[MAC_LEN]={0};
			u8 dst_addr;
			
			switch(DataBuf[index].payload.header.pack_type)
			{
				case DEL_NODE:
					DataBuf[index].isBroadcast=1;
					break;
				default:
					dst_addr=NRF_FindNodePath(DataBuf[index].payload.header.dst_addr);//Ѱ�����·��
					if(0xFF!=dst_addr)
					{
						NRF_GetMacAddr(send_mac,dst_addr,NodeStatus.pwd);//��ȡMAC��ַ
					}
					else//·�ɱ�û�е�ַ,�㲥
					{
						DataBuf[index].isBroadcast=1;
					}
					break;
			}
			NodeStatus.shift_failed_count=0;
			NodeStatus.shift_count++;
			if(!DataBuf[index].isBroadcast)
			{				
				NRF_LOG("NRF Send To %d,channel:%d,mac:",
						DataBuf[index].payload.header.dst_addr,
						NodeStatus.channel);
				for(n=0;n<MAC_LEN;n++)
				{
					NRF_LOGS("%02X",send_mac[n]);
				}
				NRF_LOGS("\n");
				if(NRF_SendPayload(ACK,send_mac,NodeStatus.channel,
									(u8 *)&DataBuf[index].payload)==RT_EOK)//ͨ��0��������
				{	
					switch(DataBuf[index].payload.header.pack_type)
					{
						case APP_DATA:
							DataBuf[index].status=WAIT_ACK;
							break;
						default:
							DataBuf[index].status=SEND_SUCCESS;
							break;
					}
				}
				else
				{
					DataBuf[index].status=WAIT_SEND_RETRY;
					DataBuf[index].send_failed_count++;
				}
			}
			else//�㲥
			{
				NRF_LOG("NRF Broadcast To %s,channel:%d,mac:",
				DataBuf[index].payload.header.pack_type==ASSIGN_ADDR?"pipe2":"pipe1",
				NodeStatus.channel);
				for(n=0;n<MAC_LEN;n++)
				{
					NRF_LOGS("%02X",
					DataBuf[index].payload.header.pack_type==ASSIGN_ADDR?
					pipe2_mac[n]:pipe1_mac[n]);
				}
				NRF_LOGS("\n");
				switch(DataBuf[index].payload.header.pack_type)
				{
					case ASSIGN_ADDR:
						NRF_SendPayload(NO_ACK,pipe2_mac,NodeStatus.channel,
										(u8 *)&DataBuf[index].payload);//ͨ��2��������(�����нڵ�㲥)
						break;
					default:
						NRF_SendPayload(NO_ACK,pipe1_mac,NodeStatus.channel,
										(u8 *)&DataBuf[index].payload);//ͨ��1��������(������ڵ�㲥)
						break;
				}
				switch(DataBuf[index].payload.header.pack_type)
				{
					case APP_DATA:
						DataBuf[index].status=WAIT_ACK;
						break;
					default:
						DataBuf[index].status=SEND_BROADCAST;
						break;
				}
			}			
		}
		else
		{			
			NodeStatus.shift_failed_count++;
			if(NodeStatus.shift_failed_count>300)
			{
				NodeStatus.shift_failed_count=0;
				//NRF_SetChannel(Pro_AppropriateChannelDetect());//���ӵ���̶���͵��ŵ�
				nrf_env_init();
			}
		}
		NRF_RxMode(NodeStatus.node_mac,NodeStatus.channel);
	}
}

static void NRF_HandleBuf(void)//���ݻ�����״̬��������
{
	u8 i;	
	for(i=0;i<DATA_BUF_COUNT;i++)
	{
		switch(DataBuf[i].status)
		{
			case WAIT_SEND_RETRY:
				if(DataBuf[i].send_failed_count>=MAX_FAILED_COUNT)
				{
					if(DataBuf[i].payload.header.pack_type==APP_DATA)
					{
						DataBuf[i].status=SEND_FAILED;
					}
					else
					{
						DataBuf[i].isBroadcast=1;
					}
				}
				break;
			case WAIT_ACK:
				DataBuf[i].time_count++;
				if(DataBuf[i].time_count>MAX_ACK_TIME)
				{
					DataBuf[i].time_count=0;
					DataBuf[i].send_failed_count=0;
					DataBuf[i].retry_count++;
					DataBuf[i].status=WAIT_RETRY;
					if(DataBuf[i].retry_count>MAX_RETRY_COUNT)
					{
						DataBuf[i].status=SEND_FAILED;
					}
				}
				break;				
			case WAIT_HANDLER:
				if(RecvCallback)
				{
					RecvCallback(DataBuf[i].payload.header.src_addr,
								DataBuf[i].payload.header.dst_addr,
								DataBuf[i].payload.header.pack_type,
								DataBuf[i].payload.buf);
				}
				NRF_FreeDataBuf(i);
				break;
			case SEND_SUCCESS:
				if(DataBuf[i].SendCallBack && DataBuf[i].payload.header.pack_type==APP_DATA)
				{
					DataBuf[i].SendCallBack(DataBuf[i].payload.header.src_addr,
												DataBuf[i].payload.header.dst_addr,
												DataBuf[i].payload.header.pack_type,
												DataBuf[i].payload.header.SID,
												DataBuf[i].payload.buf,PACKAGE_SUCCESS);
				}
				NRF_FreeDataBuf(i);
				break;
			case SEND_FAILED:
				if(DataBuf[i].SendCallBack && DataBuf[i].payload.header.pack_type==APP_DATA)
				{
					DataBuf[i].SendCallBack(DataBuf[i].payload.header.src_addr,
												DataBuf[i].payload.header.dst_addr,
												DataBuf[i].payload.header.pack_type,
												DataBuf[i].payload.header.SID,
												DataBuf[i].payload.buf,PACKAGE_FAILED);
				}
				NRF_FreeDataBuf(i);
				break;
			case SEND_BROADCAST:
				if(DataBuf[i].SendCallBack && DataBuf[i].payload.header.pack_type==APP_DATA)//���͹㲥�ص�
				{
					DataBuf[i].SendCallBack(DataBuf[i].payload.header.src_addr,
												DataBuf[i].payload.header.dst_addr,
												DataBuf[i].payload.header.pack_type,
												DataBuf[i].payload.header.SID,
												DataBuf[i].payload.buf,PACKAGE_BROADCAST);
				}
				NRF_FreeDataBuf(i);
				break;
			default:
				break;
		}
	}
}
static void nrf_handle_thread_entry(void* param)
{
	while(1)
	{
		rt_thread_delay(rt_tick_from_millisecond(10));
		rt_mutex_take(nrf_thread_mutex_lock, RT_WAITING_FOREVER);
		NRF_AutoLink();
		NRF_NrfSendData();
		NRF_HandleBuf();
		rt_mutex_release(nrf_thread_mutex_lock);
	}
}

void nrf_get_nodestatus(NODESTATUS* nodestatus)
{
	rt_memcpy(nodestatus, &NodeStatus, sizeof(NODESTATUS));
}

void nrf_reg_callback(on_connect_callback_t on_conn_cb, on_disconnect_callback_t on_disconn_cb,
	send_callback_t on_send_cb, recv_callback_t on_recv_cb)
{
	OnConnectCallback = on_conn_cb;
	OnDisconnectCallback = on_disconn_cb;
	RecvCallback = on_recv_cb;
	SendCallback = on_send_cb;
}

void nrf_reset(void)
{
	NRF_ClearStroreData();
	nrf_env_init();
}

void nrf_send_data(rt_uint8_t addr, void *data)
{
	if(NodeStatus.link_status==CONNECTED)
		NRF_PreSendData(addr,APP_DATA,data,SendCallback);
}

static int nrf_node_init(void)//��ʼ���ڵ�
{
	NRF_LOG("NRF node init start...\n");
	if(nrf_env_init()!=0)return -1;
	
	nrf_thread_mutex_lock = rt_mutex_create("node_lock", RT_IPC_FLAG_FIFO);
	RT_ASSERT(nrf_thread_mutex_lock != RT_NULL);
	
	nrf_reg_hw_recv_callback(nrf_hw_recv_callback);
	
	rt_timer_t linktime = rt_timer_create("linktime",link_timeout_callback,RT_NULL,
												rt_tick_from_millisecond(10),RT_TIMER_FLAG_PERIODIC);
	if (linktime != RT_NULL) rt_timer_start(linktime);
	
	rt_thread_t nrf_thread = rt_thread_create("nrf_handle", nrf_handle_thread_entry, RT_NULL,
												512, 11, 5);
	if (nrf_thread != RT_NULL) rt_thread_startup(nrf_thread);	
	return 0;
}
INIT_COMPONENT_EXPORT(nrf_node_init);
