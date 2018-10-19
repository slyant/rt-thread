#include <rtthread.h>
#include <nrf_gateway.h>
#include <drv_spi_nrf24l01.h>

//#define USING_NRF_LOG
#ifdef USING_NRF_LOG
	#define NRF_LOG		rt_kprintf("[NRF_GATEWAY]:");rt_kprintf
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
	
NODESTATUS NodeStatus;//�ڵ�״̬
static HeartTable heart_table[MAX_NODE_NUM];
static RoutrTable route_table[MAX_NODE_NUM];
static PathTable path_table[MAX_NODE_NUM];
static LOST_CONNECTION lost_conn[MAX_NODE_NUM];
static HANDLEMULTIPACK IsHandle[DATA_BUF_COUNT];
static DATA_BUF DataBuf[DATA_BUF_COUNT];//���ݻ���,��󳤶�PAYLOAD_DATA_LEN
static u8 pipe1_mac[MAC_LEN]={0};//ͨ��1��ַ(������ڵ�)
static u8 pipe2_mac[MAC_LEN]={0xFF,0xB9,0x7F,0x55,0xB0};//ͨ��2��ַ(���нڵ�),�����ַ
static recv_callback_t RecvCallback=NULL;
static send_callback_t SendCallback=NULL;
static on_connect_callback_t OnConnectCallback=NULL;
static on_disconnect_callback_t OnDisconnectCallback=NULL;
static rt_mutex_t nrf_thread_mutex_lock = RT_NULL;

static void nrf_print_nodestatus(void)
{
	int i;
	NRF_LOG("\r\nNodeStatus.pwd:%08X", NodeStatus.pwd);
	NRF_LOGS("\r\nNodeStatus.addr:%d", NodeStatus.addr);
	NRF_LOGS("\r\nNodeStatus.assign_addr:%d", NodeStatus.assign_addr);
	NRF_LOGS("\r\nNodeStatus.channel:%d", NodeStatus.channel);
	NRF_LOGS("\r\nNodeStatus.node_mac:");
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
	NRF_LOGS("\r\n");
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
		if(pPayLoad->header.hop_count<MAX_HOP_NUM)
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
	}
	return FALSE;
}
static u8 NRF_HeartTableFindValidNode(u8 node)//�����������Ч�ڵ�
{
	u8 i;
	
	for(i=0; i<MAX_NODE_NUM; i++)//�ڵ��б�
	{
		if(heart_table[i].addr==node)
		{
			return i;
		}
	}
	return 0xFF;
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
				for(j=0; j<MAX_HOP_NUM; j++)//��·����ɾ��1���ڵ�
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
			for(j=i+1; j<MAX_NODE_NUM; j++)//�е�ַ���Ƶ���ǰ��
			{
				if(route_table[j].addr)
				{
					rt_memcpy((BYTE *)&route_table[i],(BYTE *)&route_table[j],sizeof(RoutrTable));
					rt_memset((BYTE *)&route_table[j],0,sizeof(RoutrTable));//��·�ɱ�ɾ��1���ڵ�
					break;
				}
			}
		}
	}
	for(i=0; i<MAX_NODE_NUM; i++)//��������ɾ��1���ڵ�
	{
		if(heart_table[i].addr==node)
		{
			u8 tmp=0;
			
			rt_memset((BYTE *)&heart_table[i],0,sizeof(HeartTable));//ɾ���ڵ�

			/*******************ͳ�Ƶ��ߴ���********************/
			for(k=0; k<MAX_NODE_NUM; k++)//�����б��Ƿ�������ڵ�
			{
				if(lost_conn[k].addr==node)
				{
					tmp=node;
					break;
				}
			}
			if(tmp)//������ڵ�
			{
				for(k=0; k<MAX_NODE_NUM; k++)//���ߴ���+1
				{
					if(lost_conn[k].addr==tmp)
					{
						lost_conn[k].count++;
						if(lost_conn[k].count>=0xFE)
						{
							lost_conn[k].count=0xFE;
						}
						break;
					}
				}
			}
			else//û������ڵ㣬�ѽڵ㱣�浽�б�
			{
				for(k=0; k<MAX_NODE_NUM; k++)
				{
					if(!lost_conn[k].addr)
					{
						lost_conn[k].addr=node;
						lost_conn[k].count++;
						break;
					}
				}
			}
			/***************************************************/
		}
	}
	for(i=0; i<MAX_NODE_NUM; i++)//��������������
	{
		if(!heart_table[i].addr)
		{
			for(j=i+1; j<MAX_NODE_NUM; j++)
			{
				if(heart_table[j].addr)//�е�ַ���Ƶ���ǰ��
				{
					heart_table[i].addr=heart_table[j].addr;
					heart_table[i].link_time=heart_table[j].link_time;
					heart_table[j].addr=0;
					heart_table[j].link_time=0;
					break;
				}
			}
		}
	}
	NRF_PreSendData(node,DEL_NODE,(u8 *)"",SendCallback);//�������ݽӿ�
}
//����0,�б�������ڵ��Ѵ���; ����1,��ӳɹ�
static u8 NRF_AddNodeToHeartTable(u8 node)
{
	u8 i,idx;	
	idx=NRF_HeartTableFindValidNode(node);
	if(0xFF==idx)//û���ҵ��ڵ�(������ظ��ڵ�)
	{
		for(i=0; i<MAX_NODE_NUM; i++)//�ڵ��б�
		{
			if(!heart_table[i].addr)//��������ҿ��нڵ�,��ӵ�������
			{
				heart_table[i].addr=node;
				heart_table[i].link_time=Get_RunTime();
				return TRUE;
			}
		}
		NRF_DelNodePath(node);//ɾ���ڵ�
		return FALSE;
	}
	return FALSE;
}
static void NRF_UpdateHearTable(void)
{
	u8 i,j,k,pos=0;//pos�����������ѭ�������һ�ν�����λ��
	for(i=0; i<MAX_NODE_NUM; i++)//����·�ɱ�
	{
		if(route_table[i].addr)//·�ɱ������Ч�ڵ�
		{
			NRF_AddNodeToHeartTable(route_table[i].addr);
			for(j=0; j<MAX_HOP_NUM; j++)//����·����
			{
				if(route_table[i].path[j])//������Ч��ַ
				{
					NRF_AddNodeToHeartTable(route_table[i].path[j]);
				}
			}
		}
	}
	//ð������
	k=MAX_NODE_NUM-1;
	for(i=0;i<MAX_NODE_NUM-1;i++)//һ��Ҫ����n��  
	{  
		//ÿ�α�����־λ��Ҫ����Ϊ0�������жϺ����Ԫ���Ƿ����˽���  
		u8 flag=0;
		for(j=0;j<k;j++)//ѡ��������������ֵ�����ƶ�
		{  
			if(heart_table[j].addr>heart_table[j+1].addr)
			{
				u32 time=heart_table[j].link_time;
				u8 tmp=heart_table[j].addr;

				heart_table[j].link_time=heart_table[j+1].link_time;
				heart_table[j+1].link_time=time;
				heart_table[j].addr=heart_table[j+1].addr;
				heart_table[j+1].addr=tmp;
				flag=1;//ֻҪ�з����˽�����flag����Ϊ1
				pos=j;//ѭ�������һ�ν�����λ��j����pos
			}
		}
		k=pos;
		//�жϱ�־λ�Ƿ�Ϊ0�����Ϊ0��˵�������Ԫ���Ѿ����򣬾�ֱ��return
		if(flag==0)
		{
			break;
		}
	}
	for(i=0; i<MAX_NODE_NUM; i++)//��������������
	{
		if(!heart_table[i].addr)
		{
			for(j=i+1; j<MAX_NODE_NUM; j++)
			{
				if(heart_table[j].addr)//�е�ַ���Ƶ���ǰ��
				{
					heart_table[i].addr=heart_table[j].addr;
					heart_table[i].link_time=heart_table[j].link_time;
					heart_table[j].addr=0;
					heart_table[j].link_time=0;
					break;
				}
			}
		}
	}
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
			NRF_UpdateHearTable();
			return TRUE;
		}
	}
	for(i=0; i<MAX_NODE_NUM; i++)//û���ҵ���ַ
	{
		if(!route_table[i].addr)//�ҵ���λ
		{
			route_table[i].addr=pPayLoad->header.the_addr;//��ӵ�·�ɱ�
			NRF_UpdateToRoutrTablePath(i,pPayLoad);//����·����
			NRF_UpdateHearTable();
			return TRUE;
		}
	}
	return FALSE;
}
static u8 NRF_UpdateHeartTime(u8 node)
{
	u8 id;
	id=NRF_HeartTableFindValidNode(node);
	if(0xFF!=id)
	{
		heart_table[id].link_time=Get_RunTime();
		return TRUE;
	}
	return FALSE;
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
	if(pPayLoad->header.PID==NodeStatus.pwd)//�Ǳ������ID
	{
		idx=NRF_MallocIdleDataBufId();
		if(0xFF!=idx)
		{
			DataBuf[idx].status=WAIT_SEND;
			rt_memcpy((BYTE*)&DataBuf[idx].payload,(BYTE *)pPayLoad,sizeof(Payload));
		}
		NRF_LOG("***Ready To Relay-Package***\n\n");
	}
}
static u8 NRF_SearchDataBuf(u8 status)//����״̬Ϊstatus��DataBuf,��������
{
	u8 i=0;	
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
		if(route_table[i].addr)//��ַ��Ч
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
		else//�˳�ѭ��
		{
			break;	
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
static void nrf_hw_recv_callback(rt_uint8_t pipe_num, rt_uint8_t *rxdata, rt_uint8_t length)
{
	u8 id,idx;
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
	if(NodeStatus.pwd==pload->header.PID)//�Ǳ������ID
	{
		//10���ڴ�����İ����ٴ���(�ڹ㲥ʱ���յ��ܶ��ظ��İ�)
		if(NRF_CmpMultiPack(pload->header.src_addr,pload->header.dst_addr,
							 pload->header.pack_type,pload->header.SID)==FALSE)
		{
			id=NRF_SearchHandleTimeoutId();
			if(0xFF!=id)
			{
				IsHandle[id].set_tag = 1;
				IsHandle[id].RID=pload->header.SID;
				IsHandle[id].time=Get_RunTime();
				IsHandle[id].src_addr=pload->header.src_addr;
				IsHandle[id].dst_addr=pload->header.dst_addr;
				IsHandle[id].pack_type=pload->header.pack_type;
				
				if(pload->header.dst_addr==NodeStatus.addr)//Ŀ�ĵ�ַ�Ǳ��ڵ�
				{
					if(START_LINK==pload->header.pack_type)
					{
						u8 path[MAX_HOP_NUM]={0};//�յ�·����						
						NRF_AddNodeToRoutrTable(pload);//��ӽڵ㵽·�ɱ�
						NRF_PreSendData(pload->header.src_addr,BUILD_LINK,(u8 *)path,SendCallback);//�������ݽӿ�
					}
					switch(pload->header.pack_type)
					{
						case BUILD_LINK_ACK:
							NRF_UpdateHearTable();//����������
							if(OnConnectCallback)
								OnConnectCallback(&pload->header.src_addr);
							break;
						case CHECK_LINK_ACK:
							NRF_UpdateHeartTime(pload->header.src_addr);//���ݵ�ַ��������ʱ��
							idx=NRF_SearchDataByPackType(pload->header.src_addr,CHECK_LINK);//����addr��type����
							if(0xFF!=idx)
							{
								DataBuf[idx].status=SEND_SUCCESS;
							}
							break;
						case APP_DATA:
							idx=NRF_MallocIdleDataBufId();
							if(0xFF!=idx)
							{
								rt_memcpy((BYTE*)&DataBuf[idx].payload,(BYTE *)pload,PAYLOAD_LEN);
								DataBuf[idx].pipe_num=pipe_num;
								DataBuf[idx].status=WAIT_HANDLER;
								NRF_PreSendData(pload->header.src_addr,APP_DATA_ACK,(u8 *)"",SendCallback);//Ԥ�������ݽӿ�
							}
							break;
						case APP_DATA_ACK:
							idx=NRF_SearchDataByPackType(pload->header.src_addr,APP_DATA);//����addr��type����
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
					NRF_RelayPackage(pload);
				}
			}
		}
	}
	NRF_RxMode(NodeStatus.node_mac, NodeStatus.channel);
	rt_mutex_release(nrf_thread_mutex_lock);
}
static void NRF_HeartCallBack(u8 src_addr,u8 dst_addr,u8 pack_type,u16 id,u8 *buf,u8 status)
{
	NodeStatus.heart_status=0;
	if(status==PACKAGE_SUCCESS)
	{
		NRF_LOG("ID=%d Heart To %d ok\n\n", id, dst_addr);
	}
	else
	{
		NRF_LOG("ID=%d Heart To %d fail\n\n", id, dst_addr);
	}
}
static int nrf_env_init(void)
{
	if(!NRF_Check())
	{
		NRF_LOG("NRF24L01+ Module not found!\r\n");
		return -1;
	}
	NRF_LOG("Find NRF24L01+ Module!\r\n");	
	
	rt_memset((BYTE *)&NodeStatus,0,sizeof(NODESTATUS));//��ʼ���ڵ�״̬
	rt_memset((BYTE *)&DataBuf,0,sizeof(DATA_BUF)*DATA_BUF_COUNT);//��ʼ��������Ϊ����״̬
	rt_memset((BYTE *)&IsHandle,0,sizeof(HANDLEMULTIPACK)*DATA_BUF_COUNT);
	rt_memset((BYTE *)&lost_conn,0,sizeof(LOST_CONNECTION)*MAX_NODE_NUM);
	rt_memset((BYTE *)&heart_table,0,sizeof(HeartTable)*MAX_NODE_NUM);
	rt_memset((BYTE *)&route_table,0,sizeof(RoutrTable)*MAX_NODE_NUM);
	
	NRF_ReadParam(&NodeStatus);
	
	NodeStatus.addr=GATEWAY_ADDR;//��ʼ��ַ
	NodeStatus.channel=DEFAULT_CHANNEL;//��ʼ�ŵ�
	NodeStatus.pwd=Get_NetworkID();
	NodeStatus.heart_status=0;
	NRF_GetMacAddr(NodeStatus.node_mac,NodeStatus.addr,NodeStatus.pwd);//��ȡmac��ַ(RX)
	
	NRF_Config(NodeStatus.node_mac,NodeStatus.channel);//Ӳ����ʼ��
	NRF_SetPipe1(pipe1_mac,NodeStatus.pwd);
	nrf_print_nodestatus();
	
	return 0;
}
static void NRF_SendHeartPack(void)//����������
{
	static u8 timer=0,cnt=0;
	static u8 nCnt=0;	
	if(NodeStatus.heart_status==0)
	{
		timer++;
		if(timer>=HEART_PACK_TIME)//(10����*100)ɨ��1��;(1��)
		{
			u32 heart_time;
			u8 i;
			timer=0;
			heart_time=Get_RunTime();
			for(i=0; i<MAX_NODE_NUM; i++)//�ڵ��б�
			{
				if(heart_table[i].addr)
				{
					cnt=0;
					nCnt=0;
					if(heart_time-heart_table[i].link_time>rt_tick_from_millisecond(HEART_LINK_INTERVAL))
					{
						heart_table[i].link_time=heart_time;
						NRF_PreSendData(heart_table[i].addr,CHECK_LINK,(u8 *)"",NRF_HeartCallBack);
						NodeStatus.heart_status = 1;
					}
				}
			}
			cnt++;
			if(cnt>=30)//û�нڵ�,30��
			{
				cnt=0;
				if(nCnt<3)//û�нڵ�,��֤3��
				{
					nCnt++;
				}
				else
				{
					nCnt = 0;
					nrf_env_init();//��ʼ��
				}
				NRF_PreSendData(0xFF,0xFF,(u8 *)"",NULL);//30��NRF����1������
			}
		}
	}
}
static void NRF_NrfSendData(void)//���ݻ�����״̬��������,����ǰ�ȼ�����
{
	u8 index, n;	
	index=NRF_SearchDataBuf(WAIT_SEND);
	if(0xFF==index)
	{
		index=NRF_SearchDataBuf(WAIT_RETRY);
	}
	if(0xFF==index)
	{
		index=NRF_SearchDataBuf(WAIT_SEND_RETRY);
	}
	if(0xFF!=index)
	{
		if(!NRF_DisturbanceDetect())//����Ƿ��������ڵ��ڷ�������
		{
			u8 new_mac[MAC_LEN]={0};
			u8 dst_addr;
			
			dst_addr=NRF_FindNodePath(DataBuf[index].payload.header.dst_addr);//Ѱ��·��
			if(0xFF==dst_addr)//·�ɱ�û�е�ַ,�㲥
			{
				DataBuf[index].isBroadcast=1;
			}
			else 
			{
				NRF_GetMacAddr(new_mac,dst_addr, NodeStatus.pwd);//��ȡMAC��ַ
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
					NRF_LOGS("%02X",new_mac[n]);
				}
				NRF_LOGS("\n");
				if(NRF_SendPayload(ACK,new_mac,NodeStatus.channel,
					(u8* )&DataBuf[index].payload)==RT_EOK)//ͨ��0��������
				{
					switch(DataBuf[index].payload.header.pack_type)
					{
						case CHECK_LINK:
							DataBuf[index].status=WAIT_ACK;
							break;
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
			else //�㲥
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
						NRF_SendPayload(NO_ACK,pipe2_mac,NodeStatus.channel,(u8* )&DataBuf[index].payload);//ͨ��2��������(�����нڵ�㲥)
						break;
					default:
						NRF_SendPayload(NO_ACK,pipe1_mac,NodeStatus.channel,(u8* )&DataBuf[index].payload);//ͨ��1��������(������ڵ�㲥)
						break;
				}
				switch(DataBuf[index].payload.header.pack_type)
				{
					case CHECK_LINK:
						DataBuf[index].status=WAIT_ACK;
						break;
					case APP_DATA:
						DataBuf[index].status=WAIT_ACK;
						break;
					default:
						DataBuf[index].status=SEND_BROADCAST;
						break;
				}
			}
			NRF_RxMode(NodeStatus.node_mac, NodeStatus.channel);
		}
		else
		{
			NodeStatus.shift_failed_count++;
			if(NodeStatus.shift_failed_count>300)
			{
				NodeStatus.shift_failed_count=0;
				//NRF_SetChannel(NRF_AppropriateChannelDetect());//���ӵ���̶���͵��ŵ�
				nrf_env_init();
			}
		}
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
				if(DataBuf[i].send_failed_count>=MAX_FAILED_COUNT)//����ʧ��3��,תΪ�㲥��
				{
					if(DataBuf[i].payload.header.pack_type==CHECK_LINK
						||DataBuf[i].payload.header.pack_type==APP_DATA)
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
				if(DataBuf[i].SendCallBack && (
					DataBuf[i].payload.header.pack_type==APP_DATA||
					DataBuf[i].payload.header.pack_type==CHECK_LINK))//������ɻص�
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
				if(CHECK_LINK==DataBuf[i].payload.header.pack_type)
				{
					NRF_DelNodePath(DataBuf[i].payload.header.dst_addr);//ɾ���ڵ�
					if(OnDisconnectCallback)
						OnDisconnectCallback(&DataBuf[i].payload.header.dst_addr);
				}
				if(DataBuf[i].SendCallBack && (
					DataBuf[i].payload.header.pack_type==APP_DATA||
					DataBuf[i].payload.header.pack_type==CHECK_LINK))//����ʧ�ܻص�
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
		NRF_SendHeartPack();
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

void nrf_assign_addr(void)
{
	if(NodeStatus.assign_addr<ASSIGN_ADDR_START || NodeStatus.assign_addr>=ASSIGN_ADDR_END)
	{
		NodeStatus.assign_addr = ASSIGN_ADDR_START;
	}
	NRF_PreSendData(NodeStatus.assign_addr,ASSIGN_ADDR,(rt_uint8_t*)"",SendCallback);
	NodeStatus.assign_addr++;
	NRF_StoreParam(&NodeStatus);
}
void nrf_reset(void)
{
	NRF_ClearStroreData();
	nrf_env_init();
}
void nrf_send_data(rt_uint8_t addr, void *data)
{
	NRF_PreSendData(addr,APP_DATA,data,SendCallback);
}
int nrf_gateway_init(void)//��ʼ������
{
	NRF_LOG("NRF gateway init start...\n");
	if(nrf_env_init()!=0)return -1;
	
	nrf_thread_mutex_lock = rt_mutex_create("gate_lock", RT_IPC_FLAG_FIFO);
	RT_ASSERT(nrf_thread_mutex_lock != RT_NULL);
	
	nrf_reg_hw_recv_callback(nrf_hw_recv_callback);
	rt_thread_t nrf_thread = rt_thread_create("nrf_handle", nrf_handle_thread_entry, RT_NULL,
												512, 11, 5);
	if (nrf_thread != RT_NULL) rt_thread_startup(nrf_thread);	
	return 0;
}
INIT_COMPONENT_EXPORT(nrf_gateway_init);
