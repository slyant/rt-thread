/*! 
 *  \file hmi_driver.h
 *  \brief �����������ļ�
 *  \version 1.0
 *  \date 2012-2015
 *  \copyright ���ݴ�ʹ��Ƽ����޹�˾
 */

#include <rtthread.h>


#ifndef _HMI_DRIVER_
#define _HMI_DRIVER_

#define FIRMWARE_VER 917   //��Ļ�̼��汾��

#define CRC16_ENABLE 0         /*!< �����ҪCRC16У�鹦�ܣ��޸Ĵ˺�Ϊ1(��ʱ��Ҫ��VisualTFT��������CRCУ��)*/
#define CMD_MAX_SIZE 64        /*!<����ָ���С��������Ҫ�������������ô�һЩ*/
#define QUEUE_MAX_SIZE 512   /*!< ָ����ջ�������С��������Ҫ�������������ô�һЩ*/

/*! 
 *  \brief  ��������Ƿ����CRC16У��
 *  \param buffer ��У������ݣ�ĩβ�洢CRC16
 *  \param n ���ݳ��ȣ�����CRC16
 *  \return У��ͨ������1�����򷵻�0
 */
unsigned short CheckCRC16(unsigned char *buffer,unsigned short n);

/*! 
 *  \brief  �����豸���ã�����֮����Ҫ�����������޸Ĳ����ʡ���������������������ʽ
 */
void LockDeviceConfig(void);

/*! 
 *  \brief  �����豸����
 */
void UnlockDeviceConfig(void);

/*! 
 *  \brief     �޸Ĵ������Ĳ�����
 *  \details  ������ѡ�Χ[0~14]����Ӧʵ�ʲ�����
                   {1200,2400,4800,9600,19200,38400,57600,115200,1000000,2000000,218750,437500,875000,921800,2500000}
 *  \param  option ������ѡ��
 */
void SetCommBps(unsigned char option);

/*! 
 *  \brief  ������������
 */
void SetHandShake(void);

/*! 
 *  \brief  ����ǰ��ɫ
 *  \param  color ǰ��ɫ
 */
void SetFcolor(unsigned short color);

/*! 
 *  \brief  ���ñ���ɫ
 *  \param  color ����ɫ
 */
void SetBcolor(unsigned short color);

/*! 
 *  \brief  �������
 */
void GUI_CleanScreen(void);

/*! 
 *  \brief  �������ּ��
 *  \param  x_w ������
  *  \param  y_w ������
 */
void SetTextSpace(unsigned char x_w, unsigned char y_w);

/*! 
 *  \brief  ����������ʾ����
 *  \param  enable �Ƿ���������
 *  \param  width ���
 *  \param  height �߶�
 */
void SetFont_Region(unsigned char enable,unsigned short width,unsigned short height );

/*! 
 *  \brief  ���ù���ɫ
 *  \param  fillcolor_dwon ��ɫ�½�
 *  \param  fillcolor_up ��ɫ�Ͻ�
 */
void SetFilterColor(unsigned short fillcolor_dwon, unsigned short fillcolor_up);

/*! 
 *  \brief  ���ù���ɫ
 *  \param  x λ��X����
 *  \param  y λ��Y����
 *  \param  back ��ɫ�Ͻ�
 *  \param  font ����
 *  \param  strings �ַ�������
 */
void DisText(unsigned short x, unsigned short y,unsigned char back,unsigned char font,unsigned char *strings );

/*! 
 *  \brief    ��ʾ���
 *  \param  enable �Ƿ���ʾ
 *  \param  x λ��X����
 *  \param  y λ��Y����
 *  \param  width ���
 *  \param  height �߶�
 */
void DisCursor(unsigned char enable,unsigned short x, unsigned short y,unsigned char width,unsigned char height );

/*! 
 *  \brief      ��ʾȫ��ͼƬ
 *  \param  image_id ͼƬ����
 *  \param  masken �Ƿ�����͸������
 */
void DisFull_Image(unsigned short image_id,unsigned char masken);

/*! 
 *  \brief      ָ��λ����ʾͼƬ
 *  \param  x λ��X����
 *  \param  y λ��Y����
 *  \param  image_id ͼƬ����
 *  \param  masken �Ƿ�����͸������
 */
void DisArea_Image(unsigned short x,unsigned short y,unsigned short image_id,unsigned char masken);

/*! 
 *  \brief      ��ʾ�ü�ͼƬ
 *  \param  x λ��X����
 *  \param  y λ��Y����
 *  \param  image_id ͼƬ����
 *  \param  image_x ͼƬ�ü�λ��X����
 *  \param  image_y ͼƬ�ü�λ��Y����
 *  \param  image_l ͼƬ�ü�����
 *  \param  image_w ͼƬ�ü��߶�
 *  \param  masken �Ƿ�����͸������
 */
void DisCut_Image(unsigned short x,unsigned short y,unsigned short image_id,unsigned short image_x,unsigned short image_y,
                   unsigned short image_l, unsigned short image_w,unsigned char masken);

/*! 
 *  \brief      ��ʾGIF����
 *  \param  x λ��X����
 *  \param  y λ��Y����
 *  \param  flashimage_id ͼƬ����
 *  \param  enable �Ƿ���ʾ
 *  \param  playnum ���Ŵ���
 */
void DisFlashImage(unsigned short x,unsigned short y,unsigned short flashimage_id,unsigned char enable,unsigned char playnum);

/*! 
 *  \brief      ����
 *  \param  x λ��X����
 *  \param  y λ��Y����
 */
void GUI_Dot(unsigned short x,unsigned short y);

/*! 
 *  \brief      ����
 *  \param  x0 ��ʼλ��X����
 *  \param  y0 ��ʼλ��Y����
 *  \param  x1 ����λ��X����
 *  \param  y1 ����λ��Y����
 */
void GUI_Line(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1);

/*! 
 *  \brief      ������
 *  \param  mode ģʽ
 *  \param  dot ���ݵ�
 *  \param  dot_cnt ����
 */
void GUI_ConDots(unsigned char mode,unsigned short *dot,unsigned short dot_cnt);

/*! 
 *  \brief      ������Բ
 *  \param  x0 Բ��λ��X����
 *  \param  y0 Բ��λ��Y����
 *  \param  r �뾶
 */
void GUI_Circle(unsigned short x0, unsigned short y0, unsigned short r);

/*! 
 *  \brief      ��ʵ��Բ
 *  \param  x0 Բ��λ��X����
 *  \param  y0 Բ��λ��Y����
 *  \param  r �뾶
 */
void GUI_CircleFill(unsigned short x0, unsigned short y0, unsigned short r);

/*! 
 *  \brief      ������
 *  \param  x0 Բ��λ��X����
 *  \param  y0 Բ��λ��Y����
 *  \param  r �뾶
 *  \param  sa ��ʼ�Ƕ�
 *  \param  ea ��ֹ�Ƕ�
 */
void GUI_Arc(unsigned short x,unsigned short y, unsigned short r,unsigned short sa, unsigned short ea);

/*! 
 *  \brief      �����ľ���
 *  \param  x0 ��ʼλ��X����
 *  \param  y0 ��ʼλ��Y����
 *  \param  x1 ����λ��X����
 *  \param  y1 ����λ��Y����
 */
void GUI_Rectangle(unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 );

/*! 
 *  \brief      ��ʵ�ľ���
 *  \param  x0 ��ʼλ��X����
 *  \param  y0 ��ʼλ��Y����
 *  \param  x1 ����λ��X����
 *  \param  y1 ����λ��Y����
 */
void GUI_RectangleFill(unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 );

/*! 
 *  \brief      ��������Բ
 *  \param  x0 ��ʼλ��X����
 *  \param  y0 ��ʼλ��Y����
 *  \param  x1 ����λ��X����
 *  \param  y1 ����λ��Y����
 */
void GUI_Ellipse (unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 );

/*! 
 *  \brief      ��ʵ����Բ
 *  \param  x0 ��ʼλ��X����
 *  \param  y0 ��ʼλ��Y����
 *  \param  x1 ����λ��X����
 *  \param  y1 ����λ��Y����
 */
void GUI_EllipseFill (unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 );

/*! 
 *  \brief      ����
 *  \param  x0 ��ʼλ��X����
 *  \param  y0 ��ʼλ��Y����
 *  \param  x1 ����λ��X����
 *  \param  y1 ����λ��Y����
 */
void SetBackLight(unsigned char light_level);

/*! 
 *  \brief   ����������
 *  \time  time ����ʱ��(���뵥λ)
 */
void SetBuzzer(unsigned char time);

/*! 
 *  \brief   ����������
 *  \param enable ����ʹ��
 *  \param beep_on ����������
 *  \param work_mode ��������ģʽ��0���¾��ϴ���1�ɿ����ϴ���2�����ϴ�����ֵ��3���º��ɿ����ϴ�����
 *  \param press_calibration �������������20��У׼��������0���ã�1����
 */
void SetTouchPaneOption(unsigned char enbale,unsigned char beep_on,unsigned char work_mode,unsigned char press_calibration);

/*! 
 *  \brief   У׼������
 */
void	CalibrateTouchPane(void);

/*! 
 *  \brief  ����������
 */
void TestTouchPane(void);

/*! 
 *  \brief      ���õ�ǰд��ͼ��
 *  \details  һ������ʵ��˫����Ч��(��ͼʱ������˸)��
 *  \details  unsigned char layer = 0;
 *  \details  WriteLayer(layer);    //����д���
 *  \details  ClearLayer(layer);    //ʹͼ���͸��
 *  \details  //���һϵ�л�ͼָ��
 *  \details  //DisText(100,100,0,4,"hello hmi!!!");
 *  \details  DisplyLayer(layer);  //�л���ʾ��
 *  \details  layer = (layer+1)%2;  //˫�����л�
 *  \see DisplyLayer
 *  \see ClearLayer
 *  \param  layer ͼ����
 */
void WriteLayer(unsigned char layer);

/*! 
 *  \brief      ���õ�ǰ��ʾͼ��
 *  \param  layer ͼ����
 */
void DisplyLayer(unsigned char layer);

/*! 
 *  \brief      ���ͼ�㣬ʹͼ����͸��
 *  \param  layer ͼ����
 */
void ClearLayer(unsigned char layer);

/*! 
 *  \brief  д���ݵ��������û��洢��
 *  \param  startAddress ��ʼ��ַ
 *  \param  length �ֽ���
 *  \param  _data ��д�������
 */
void WriteUserFlash(unsigned long startAddress,unsigned short length,unsigned char *_data);

/*! 
 *  \brief  �Ӵ������û��洢����ȡ����
 *  \param  startAddress ��ʼ��ַ
 *  \param  length �ֽ���
 */
void ReadUserFlash(unsigned long startAddress,unsigned short length);

/*! 
 *  \brief      ����ͼ��
 *  \param  src_layer ԭʼͼ��
 *  \param  dest_layer Ŀ��ͼ��
 */
void CopyLayer(unsigned char src_layer,unsigned char dest_layer);

/*! 
 *  \brief      ���õ�ǰ����
 *  \param  screen_id ����ID
 */
void SetScreen(unsigned short screen_id);

/*! 
 *  \brief      ��ȡ��ǰ����
 */
void GetScreen(void);

/*! 
 *  \brief     ����\���û������
 *  \details ����\����һ��ɶ�ʹ�ã����ڱ�����˸�����ˢ���ٶ�
 *  \details �÷���
 *	\details SetScreenUpdateEnable(0);//��ֹ����
 *	\details һϵ�и��»����ָ��
 *	\details SetScreenUpdateEnable(1);//��������
 *  \param  enable 0���ã�1����
 */
void SetScreenUpdateEnable(unsigned char enable);

/*! 
 *  \brief     ���ÿؼ����뽹��
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  focus �Ƿ�������뽹��
 */
void SetControlFocus(unsigned short screen_id,unsigned short control_id,unsigned char focus);

/*! 
 *  \brief     ��ʾ\���ؿؼ�
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  visible �Ƿ���ʾ
 */
void SetControlVisiable(unsigned short screen_id,unsigned short control_id,unsigned char visible);

/*! 
 *  \brief     ���ô����ؼ�ʹ��
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  enable �ؼ��Ƿ�ʹ��
 */
void SetControlEnable(unsigned short screen_id,unsigned short control_id,unsigned char enable);

/*! 
 *  \brief     ��ȡ�ؼ�ֵ
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void GetControlValue(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     ���ð�ť״̬
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  value ��ť״̬
 */
void SetButtonValue(unsigned short screen_id,unsigned short control_id,unsigned char value);

/*! 
 *  \brief     �����ı�ֵ
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  str �ı�ֵ
 */
void SetTextValue(unsigned short screen_id,unsigned short control_id,unsigned char *str);

#if FIRMWARE_VER>=908

/*! 
 *  \brief     �����ı�Ϊ����ֵ��Ҫ��FIRMWARE_VER>=908
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  value �ı���ֵ
 *  \param  sign 0-�޷��ţ�1-�з���
 *  \param  fill_zero ����λ��������ʱ��ಹ��
 */
void SetTextInt32(unsigned short screen_id,unsigned short control_id,unsigned long value,unsigned char sign,unsigned char fill_zero);

/*! 
 *  \brief     �����ı������ȸ���ֵ��Ҫ��FIRMWARE_VER>=908
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  value �ı���ֵ
 *  \param  precision С��λ��
 *  \param  show_zeros Ϊ1ʱ����ʾĩβ0
 */
void SetTextFloat(unsigned short screen_id,unsigned short control_id,float value,unsigned char precision,unsigned char show_zeros);

#endif

/*! 
 *  \brief      ���ý���ֵ
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  value ��ֵ
 */
void SetProgressValue(unsigned short screen_id,unsigned short control_id,unsigned long value);

/*! 
 *  \brief     �����Ǳ�ֵ
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  value ��ֵ
 */
void SetMeterValue(unsigned short screen_id,unsigned short control_id,unsigned long value);

/*! 
 *  \brief      ���û�����
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  value ��ֵ
 */
void SetSliderValue(unsigned short screen_id,unsigned short control_id,unsigned long value);

/*! 
 *  \brief      ����ѡ��ؼ�
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  item ��ǰѡ��
 */
void SetSelectorValue(unsigned short screen_id,unsigned short control_id,unsigned char item);

/*! 
 *  \brief      ��ʼ���Ŷ���
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void AnimationStart(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief      ֹͣ���Ŷ���
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void AnimationStop(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief      ��ͣ���Ŷ���
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void AnimationPause(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     �����ƶ�֡
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  frame_id ֡ID
 */
void AnimationPlayFrame(unsigned short screen_id,unsigned short control_id,unsigned char frame_id);

/*! 
 *  \brief     ������һ֡
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void AnimationPlayPrev(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     ������һ֡
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void AnimationPlayNext(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     ���߿ؼ�-���ͨ��
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  channel ͨ����
 *  \param  color ��ɫ
 */
void GraphChannelAdd(unsigned short screen_id,unsigned short control_id,unsigned char channel,unsigned short color);

/*! 
 *  \brief     ���߿ؼ�-ɾ��ͨ��
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  channel ͨ����
 */
void GraphChannelDel(unsigned short screen_id,unsigned short control_id,unsigned char channel);

/*! 
 *  \brief     ���߿ؼ�-�������
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  channel ͨ����
 *  \param  pData ��������
 *  \param  nDataLen ���ݸ���
 */
void GraphChannelDataAdd(unsigned short screen_id,unsigned short control_id,unsigned char channel,unsigned char *pData,unsigned short nDataLen);

/*! 
 *  \brief     ���߿ؼ�-�������
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  channel ͨ����
 */
void GraphChannelDataClear(unsigned short screen_id,unsigned short control_id,unsigned char channel);

/*! 
 *  \brief     ���߿ؼ�-������ͼ����
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  x_offset ˮƽƫ��
 *  \param  x_mul ˮƽ����ϵ��
 *  \param  y_offset ��ֱƫ��
 *  \param  y_mul ��ֱ����ϵ��
 */
void GraphSetViewport(unsigned short screen_id,unsigned short control_id,short x_offset,unsigned short x_mul,short y_offset,unsigned short y_mul);

/*! 
 *  \brief     ��ʼ��������
 *  \param  screen_id ����ID
 */
void BatchBegin(unsigned short screen_id);

/*! 
 *  \brief     �������°�ť�ؼ�
 *  \param  control_id �ؼ�ID
 *  \param  value ��ֵ
 */
void BatchSetButtonValue(unsigned short control_id,unsigned char state);

/*! 
 *  \brief     �������½������ؼ�
 *  \param  control_id �ؼ�ID
 *  \param  value ��ֵ
 */
void BatchSetProgressValue(unsigned short control_id,unsigned long value);

/*! 
 *  \brief     �������»������ؼ�
 *  \param  control_id �ؼ�ID
 *  \param  value ��ֵ
 */
void BatchSetSliderValue(unsigned short control_id,unsigned long value);

/*! 
 *  \brief     ���������Ǳ�ؼ�
 *  \param  control_id �ؼ�ID
 *  \param  value ��ֵ
 */
void BatchSetMeterValue(unsigned short control_id,unsigned long value);

/*! 
 *  \brief     ���������ı��ؼ�
 *  \param  control_id �ؼ�ID
 *  \param  strings �ַ���
 */
void BatchSetText(unsigned short control_id,unsigned char *strings);
void BatchSetValueInt32(unsigned short control_id,long value);
/*! 
 *  \brief     �������¶���\ͼ��ؼ�
 *  \param  control_id �ؼ�ID
 *  \param  frame_id ֡ID
 */
void BatchSetFrame(unsigned short control_id,unsigned short frame_id);

#if FIRMWARE_VER>=908

/*! 
 *  \brief     �������ÿؼ��ɼ�
 *  \param  control_id �ؼ�ID
 *  \param  visible ֡ID
 */
void BatchSetVisible(unsigned short control_id,unsigned char visible);

/*! 
 *  \brief     �������ÿؼ�ʹ��
 *  \param  control_id �ؼ�ID
 *  \param  enable ֡ID
 */
void BatchSetEnable(unsigned short control_id,unsigned char enable);

#endif

/*! 
 *  \brief    ������������
 */
void BatchEnd(void);

/*! 
 *  \brief     ���õ���ʱ�ؼ�
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  timeout ����ʱ(��)
 */
void SeTimer(unsigned short screen_id,unsigned short control_id,unsigned long timeout);

/*! 
 *  \brief     ��������ʱ�ؼ�
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void StartTimer(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     ֹͣ����ʱ�ؼ�
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void StopTimer(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     ��ͣ����ʱ�ؼ�
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void PauseTimer(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     ���ÿؼ�����ɫ
 *  \details  ֧�ֿؼ������������ı�
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  color ����ɫ
 */
void SetControlBackColor(unsigned short screen_id,unsigned short control_id,unsigned short color);

/*! 
 *  \brief     ���ÿؼ�ǰ��ɫ
  * \details  ֧�ֿؼ���������
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  color ǰ��ɫ
 */
void SetControlForeColor(unsigned short screen_id,unsigned short control_id,unsigned short color);

/*! 
 *  \brief     ��ʾ\���ص����˵��ؼ�
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 *  \param  show �Ƿ���ʾ��Ϊ0ʱfocus_control_id��Ч
 *  \param  focus_control_id �������ı��ؼ�(�˵��ؼ�������������ı��ؼ�)
 */
void ShowPopupMenu(unsigned short screen_id,unsigned short control_id,unsigned char show,unsigned short focus_control_id);

/*! 
 *  \brief     ��ʾ\����ϵͳ����
 *  \param  show 0���أ�1��ʾ
 *  \param  x ������ʾλ��X����
 *  \param  y ������ʾλ��Y����
 *  \param  type 0С���̣�1ȫ����
 *  \param  option 0�����ַ���1���룬2ʱ������
 *  \param  max_len ����¼���ַ���������
 */
void ShowKeyboard(unsigned char show,unsigned short x,unsigned short y,unsigned char type,unsigned char option,unsigned char max_len);

#if FIRMWARE_VER>=914
/*! 
 *  \brief     ����������
 *  \param  ui_lang �û���������0~9
 *  \param  sys_lang ϵͳ��������-0���ģ�1Ӣ��
 */
void SetLanguage(unsigned char ui_lang,unsigned char sys_lang);
#endif

#if FIRMWARE_VER>=917
/*! 
 *  \brief     ��ʼ����ؼ���ֵ��FLASH
 *  \param  version ���ݰ汾�ţ�������ָ������16λΪ���汾�ţ���16λΪ�ΰ汾��
 *  \param  address �������û��洢���Ĵ�ŵ�ַ��ע���ֹ��ַ�ص�����ͻ
 */
void FlashBeginSaveControl(unsigned long version,unsigned long address);

/*! 
 *  \brief     ����ĳ���ؼ�����ֵ��FLASH
 *  \param  screen_id ����ID
 *  \param  control_id �ؼ�ID
 */
void FlashSaveControl(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     �������
 */
void FlashEndSaveControl(void);

/*! 
 *  \brief     ��FLASH�лָ��ؼ�����
 *  \param  version ���ݰ汾�ţ����汾�ű�����洢ʱһ�£���������ʧ��
 *  \param  address �������û��洢���Ĵ�ŵ�ַ
 */
void FlashRestoreControl(unsigned long version,unsigned long address);
#endif

void ReadRtc(void);

void SetRtc(unsigned short Year,unsigned char Mon,unsigned char Day,unsigned char Week,unsigned char Hour,unsigned char Min,unsigned char Sec);
#endif

void show_string(unsigned short screen_id, unsigned short x, unsigned short y, unsigned char fg, unsigned char *str);

void update_lcd_enable(void);
void update_lcd_disable(void);
void clesr_text(unsigned short screen_id,unsigned short control_id);


