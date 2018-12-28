/*! 
 *  \file hmi_driver.h
 *  \brief 串口屏驱动文件
 *  \version 1.0
 *  \date 2012-2015
 *  \copyright 广州大彩光电科技有限公司
 */

#include <rtthread.h>


#ifndef _HMI_DRIVER_
#define _HMI_DRIVER_

#define FIRMWARE_VER 917   //屏幕固件版本号

#define CRC16_ENABLE 0         /*!< 如果需要CRC16校验功能，修改此宏为1(此时需要在VisualTFT工程中配CRC校验)*/
#define CMD_MAX_SIZE 64        /*!<单条指令大小，根据需要调整，尽量设置大一些*/
#define QUEUE_MAX_SIZE 512   /*!< 指令接收缓冲区大小，根据需要调整，尽量设置大一些*/

#define COLOR_RED		0xF800
#define COLOR_GREEN		0x0400
#define COLOR_BLUE		0X001f
#define COLOR_YELLOW	0XFFE0
#define COLOR_ORANGE	0XFB20
#define COLOR_WHITE 	0XFFFF
#define COLOR_BLACK		0X0000

/*! 
 *  \brief  检查数据是否符合CRC16校验
 *  \param buffer 待校验的数据，末尾存储CRC16
 *  \param n 数据长度，包含CRC16
 *  \return 校验通过返回1，否则返回0
 */
unsigned short CheckCRC16(unsigned char *buffer,unsigned short n);

/*! 
 *  \brief  锁定设备配置，锁定之后需要解锁，才能修改波特率、触摸屏、蜂鸣器工作方式
 */
void LockDeviceConfig(void);

/*! 
 *  \brief  解锁设备配置
 */
void UnlockDeviceConfig(void);

/*! 
 *  \brief     修改串口屏的波特率
 *  \details  波特率选项范围[0~14]，对应实际波特率
                   {1200,2400,4800,9600,19200,38400,57600,115200,1000000,2000000,218750,437500,875000,921800,2500000}
 *  \param  option 波特率选项
 */
void SetCommBps(unsigned char option);

/*! 
 *  \brief  发送握手命令
 */
void SetHandShake(void);

/*! 
 *  \brief  设置前景色
 *  \param  color 前景色
 */
void SetFcolor(unsigned short color);

/*! 
 *  \brief  设置背景色
 *  \param  color 背景色
 */
void SetBcolor(unsigned short color);

/*! 
 *  \brief  清除画面
 */
void GUI_CleanScreen(void);

/*! 
 *  \brief  设置文字间隔
 *  \param  x_w 横向间隔
  *  \param  y_w 纵向间隔
 */
void SetTextSpace(unsigned char x_w, unsigned char y_w);

/*! 
 *  \brief  设置文字显示限制
 *  \param  enable 是否启用限制
 *  \param  width 宽度
 *  \param  height 高度
 */
void SetFont_Region(unsigned char enable,unsigned short width,unsigned short height );

/*! 
 *  \brief  设置过滤色
 *  \param  fillcolor_dwon 颜色下界
 *  \param  fillcolor_up 颜色上界
 */
void SetFilterColor(unsigned short fillcolor_dwon, unsigned short fillcolor_up);

/*! 
 *  \brief  设置过滤色
 *  \param  x 位置X坐标
 *  \param  y 位置Y坐标
 *  \param  back 颜色上界
 *  \param  font 字体
 *  \param  strings 字符串内容
 */
void DisText(unsigned short x, unsigned short y,unsigned char back,unsigned char font,unsigned char *strings );

/*! 
 *  \brief    显示光标
 *  \param  enable 是否显示
 *  \param  x 位置X坐标
 *  \param  y 位置Y坐标
 *  \param  width 宽度
 *  \param  height 高度
 */
void DisCursor(unsigned char enable,unsigned short x, unsigned short y,unsigned char width,unsigned char height );

/*! 
 *  \brief      显示全屏图片
 *  \param  image_id 图片索引
 *  \param  masken 是否启用透明掩码
 */
void DisFull_Image(unsigned short image_id,unsigned char masken);

/*! 
 *  \brief      指定位置显示图片
 *  \param  x 位置X坐标
 *  \param  y 位置Y坐标
 *  \param  image_id 图片索引
 *  \param  masken 是否启用透明掩码
 */
void DisArea_Image(unsigned short x,unsigned short y,unsigned short image_id,unsigned char masken);

/*! 
 *  \brief      显示裁剪图片
 *  \param  x 位置X坐标
 *  \param  y 位置Y坐标
 *  \param  image_id 图片索引
 *  \param  image_x 图片裁剪位置X坐标
 *  \param  image_y 图片裁剪位置Y坐标
 *  \param  image_l 图片裁剪长度
 *  \param  image_w 图片裁剪高度
 *  \param  masken 是否启用透明掩码
 */
void DisCut_Image(unsigned short x,unsigned short y,unsigned short image_id,unsigned short image_x,unsigned short image_y,
                   unsigned short image_l, unsigned short image_w,unsigned char masken);

/*! 
 *  \brief      显示GIF动画
 *  \param  x 位置X坐标
 *  \param  y 位置Y坐标
 *  \param  flashimage_id 图片索引
 *  \param  enable 是否显示
 *  \param  playnum 播放次数
 */
void DisFlashImage(unsigned short x,unsigned short y,unsigned short flashimage_id,unsigned char enable,unsigned char playnum);

/*! 
 *  \brief      画点
 *  \param  x 位置X坐标
 *  \param  y 位置Y坐标
 */
void GUI_Dot(unsigned short x,unsigned short y);

/*! 
 *  \brief      画线
 *  \param  x0 起始位置X坐标
 *  \param  y0 起始位置Y坐标
 *  \param  x1 结束位置X坐标
 *  \param  y1 结束位置Y坐标
 */
void GUI_Line(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1);

/*! 
 *  \brief      画折线
 *  \param  mode 模式
 *  \param  dot 数据点
 *  \param  dot_cnt 点数
 */
void GUI_ConDots(unsigned char mode,unsigned short *dot,unsigned short dot_cnt);

/*! 
 *  \brief      画空心圆
 *  \param  x0 圆心位置X坐标
 *  \param  y0 圆心位置Y坐标
 *  \param  r 半径
 */
void GUI_Circle(unsigned short x0, unsigned short y0, unsigned short r);

/*! 
 *  \brief      画实心圆
 *  \param  x0 圆心位置X坐标
 *  \param  y0 圆心位置Y坐标
 *  \param  r 半径
 */
void GUI_CircleFill(unsigned short x0, unsigned short y0, unsigned short r);

/*! 
 *  \brief      画弧线
 *  \param  x0 圆心位置X坐标
 *  \param  y0 圆心位置Y坐标
 *  \param  r 半径
 *  \param  sa 起始角度
 *  \param  ea 终止角度
 */
void GUI_Arc(unsigned short x,unsigned short y, unsigned short r,unsigned short sa, unsigned short ea);

/*! 
 *  \brief      画空心矩形
 *  \param  x0 起始位置X坐标
 *  \param  y0 起始位置Y坐标
 *  \param  x1 结束位置X坐标
 *  \param  y1 结束位置Y坐标
 */
void GUI_Rectangle(unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 );

/*! 
 *  \brief      画实心矩形
 *  \param  x0 起始位置X坐标
 *  \param  y0 起始位置Y坐标
 *  \param  x1 结束位置X坐标
 *  \param  y1 结束位置Y坐标
 */
void GUI_RectangleFill(unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 );

/*! 
 *  \brief      画空心椭圆
 *  \param  x0 起始位置X坐标
 *  \param  y0 起始位置Y坐标
 *  \param  x1 结束位置X坐标
 *  \param  y1 结束位置Y坐标
 */
void GUI_Ellipse (unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 );

/*! 
 *  \brief      画实心椭圆
 *  \param  x0 起始位置X坐标
 *  \param  y0 起始位置Y坐标
 *  \param  x1 结束位置X坐标
 *  \param  y1 结束位置Y坐标
 */
void GUI_EllipseFill (unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 );

/*! 
 *  \brief      画线
 *  \param  x0 起始位置X坐标
 *  \param  y0 起始位置Y坐标
 *  \param  x1 结束位置X坐标
 *  \param  y1 结束位置Y坐标
 */
void SetBackLight(unsigned char light_level);

/*! 
 *  \brief   蜂鸣器设置
 *  \time  time 持续时间(毫秒单位)
 */
void SetBuzzer(unsigned char time);

/*! 
 *  \brief   触摸屏设置
 *  \param enable 触摸使能
 *  \param beep_on 触摸蜂鸣器
 *  \param work_mode 触摸工作模式：0按下就上传，1松开才上传，2不断上传坐标值，3按下和松开均上传数据
 *  \param press_calibration 连续点击触摸屏20下校准触摸屏：0禁用，1启用
 */
void SetTouchPaneOption(unsigned char enbale,unsigned char beep_on,unsigned char work_mode,unsigned char press_calibration);

/*! 
 *  \brief   校准触摸屏
 */
void	CalibrateTouchPane(void);

/*! 
 *  \brief  触摸屏测试
 */
void TestTouchPane(void);

/*! 
 *  \brief      设置当前写入图层
 *  \details  一般用于实现双缓存效果(绘图时避免闪烁)：
 *  \details  unsigned char layer = 0;
 *  \details  WriteLayer(layer);    //设置写入层
 *  \details  ClearLayer(layer);    //使图层变透明
 *  \details  //添加一系列绘图指令
 *  \details  //DisText(100,100,0,4,"hello hmi!!!");
 *  \details  DisplyLayer(layer);  //切换显示层
 *  \details  layer = (layer+1)%2;  //双缓存切换
 *  \see DisplyLayer
 *  \see ClearLayer
 *  \param  layer 图层编号
 */
void WriteLayer(unsigned char layer);

/*! 
 *  \brief      设置当前显示图层
 *  \param  layer 图层编号
 */
void DisplyLayer(unsigned char layer);

/*! 
 *  \brief      清除图层，使图层变成透明
 *  \param  layer 图层编号
 */
void ClearLayer(unsigned char layer);

/*! 
 *  \brief  写数据到串口屏用户存储区
 *  \param  startAddress 起始地址
 *  \param  length 字节数
 *  \param  _data 待写入的数据
 */
void WriteUserFlash(unsigned long startAddress,unsigned short length,unsigned char *_data);

/*! 
 *  \brief  从串口屏用户存储区读取数据
 *  \param  startAddress 起始地址
 *  \param  length 字节数
 */
void ReadUserFlash(unsigned long startAddress,unsigned short length);

/*! 
 *  \brief      拷贝图层
 *  \param  src_layer 原始图层
 *  \param  dest_layer 目标图层
 */
void CopyLayer(unsigned char src_layer,unsigned char dest_layer);

/*! 
 *  \brief      设置当前画面
 *  \param  screen_id 画面ID
 */
void SetScreen(unsigned short screen_id);

/*! 
 *  \brief      获取当前画面
 */
void GetScreen(void);

/*! 
 *  \brief     禁用\启用画面更新
 *  \details 禁用\启用一般成对使用，用于避免闪烁、提高刷新速度
 *  \details 用法：
 *	\details SetScreenUpdateEnable(0);//禁止更新
 *	\details 一系列更新画面的指令
 *	\details SetScreenUpdateEnable(1);//立即更新
 *  \param  enable 0禁用，1启用
 */
void SetScreenUpdateEnable(unsigned char enable);

/*! 
 *  \brief     设置控件输入焦点
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  focus 是否具有输入焦点
 */
void SetControlFocus(unsigned short screen_id,unsigned short control_id,unsigned char focus);

/*! 
 *  \brief     显示\隐藏控件
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  visible 是否显示
 */
void SetControlVisiable(unsigned short screen_id,unsigned short control_id,unsigned char visible);

/*! 
 *  \brief     设置触摸控件使能
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  enable 控件是否使能
 */
void SetControlEnable(unsigned short screen_id,unsigned short control_id,unsigned char enable);

/*! 
 *  \brief     获取控件值
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void GetControlValue(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     设置按钮状态
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  value 按钮状态
 */
void SetButtonValue(unsigned short screen_id,unsigned short control_id,unsigned char value);

/*! 
 *  \brief     设置文本值
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  str 文本值
 */
void SetTextValue(unsigned short screen_id,unsigned short control_id,unsigned char *str);

#if FIRMWARE_VER>=908

/*! 
 *  \brief     设置文本为整数值，要求FIRMWARE_VER>=908
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  value 文本数值
 *  \param  sign 0-无符号，1-有符号
 *  \param  fill_zero 数字位数，不足时左侧补零
 */
void SetTextInt32(unsigned short screen_id,unsigned short control_id,unsigned long value,unsigned char sign,unsigned char fill_zero);

/*! 
 *  \brief     设置文本单精度浮点值，要求FIRMWARE_VER>=908
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  value 文本数值
 *  \param  precision 小数位数
 *  \param  show_zeros 为1时，显示末尾0
 */
void SetTextFloat(unsigned short screen_id,unsigned short control_id,float value,unsigned char precision,unsigned char show_zeros);

#endif

/*! 
 *  \brief      设置进度值
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  value 数值
 */
void SetProgressValue(unsigned short screen_id,unsigned short control_id,unsigned long value);

/*! 
 *  \brief     设置仪表值
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  value 数值
 */
void SetMeterValue(unsigned short screen_id,unsigned short control_id,unsigned long value);

/*! 
 *  \brief      设置滑动条
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  value 数值
 */
void SetSliderValue(unsigned short screen_id,unsigned short control_id,unsigned long value);

/*! 
 *  \brief      设置选择控件
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  item 当前选项
 */
void SetSelectorValue(unsigned short screen_id,unsigned short control_id,unsigned char item);

/*! 
 *  \brief      开始播放动画
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void AnimationStart(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief      停止播放动画
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void AnimationStop(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief      暂停播放动画
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void AnimationPause(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     播放制定帧
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  frame_id 帧ID
 */
void AnimationPlayFrame(unsigned short screen_id,unsigned short control_id,unsigned char frame_id);

/*! 
 *  \brief     播放上一帧
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void AnimationPlayPrev(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     播放下一帧
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void AnimationPlayNext(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     曲线控件-添加通道
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  channel 通道号
 *  \param  color 颜色
 */
void GraphChannelAdd(unsigned short screen_id,unsigned short control_id,unsigned char channel,unsigned short color);

/*! 
 *  \brief     曲线控件-删除通道
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  channel 通道号
 */
void GraphChannelDel(unsigned short screen_id,unsigned short control_id,unsigned char channel);

/*! 
 *  \brief     曲线控件-添加数据
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  channel 通道号
 *  \param  pData 曲线数据
 *  \param  nDataLen 数据个数
 */
void GraphChannelDataAdd(unsigned short screen_id,unsigned short control_id,unsigned char channel,unsigned char *pData,unsigned short nDataLen);

/*! 
 *  \brief     曲线控件-清除数据
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  channel 通道号
 */
void GraphChannelDataClear(unsigned short screen_id,unsigned short control_id,unsigned char channel);

/*! 
 *  \brief     曲线控件-设置视图窗口
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  x_offset 水平偏移
 *  \param  x_mul 水平缩放系数
 *  \param  y_offset 垂直偏移
 *  \param  y_mul 垂直缩放系数
 */
void GraphSetViewport(unsigned short screen_id,unsigned short control_id,short x_offset,unsigned short x_mul,short y_offset,unsigned short y_mul);

/*! 
 *  \brief     开始批量更新
 *  \param  screen_id 画面ID
 */
void BatchBegin(unsigned short screen_id);

/*! 
 *  \brief     批量更新按钮控件
 *  \param  control_id 控件ID
 *  \param  value 数值
 */
void BatchSetButtonValue(unsigned short control_id,unsigned char state);

/*! 
 *  \brief     批量更新进度条控件
 *  \param  control_id 控件ID
 *  \param  value 数值
 */
void BatchSetProgressValue(unsigned short control_id,unsigned long value);

/*! 
 *  \brief     批量更新滑动条控件
 *  \param  control_id 控件ID
 *  \param  value 数值
 */
void BatchSetSliderValue(unsigned short control_id,unsigned long value);

/*! 
 *  \brief     批量更新仪表控件
 *  \param  control_id 控件ID
 *  \param  value 数值
 */
void BatchSetMeterValue(unsigned short control_id,unsigned long value);

/*! 
 *  \brief     批量更新文本控件
 *  \param  control_id 控件ID
 *  \param  strings 字符串
 */
void BatchSetText(unsigned short control_id,unsigned char *strings);
void BatchSetValueInt32(unsigned short control_id,long value);
/*! 
 *  \brief     批量更新动画\图标控件
 *  \param  control_id 控件ID
 *  \param  frame_id 帧ID
 */
void BatchSetFrame(unsigned short control_id,unsigned short frame_id);

#if FIRMWARE_VER>=908

/*! 
 *  \brief     批量设置控件可见
 *  \param  control_id 控件ID
 *  \param  visible 帧ID
 */
void BatchSetVisible(unsigned short control_id,unsigned char visible);

/*! 
 *  \brief     批量设置控件使能
 *  \param  control_id 控件ID
 *  \param  enable 帧ID
 */
void BatchSetEnable(unsigned short control_id,unsigned char enable);

#endif

/*! 
 *  \brief    结束批量更新
 */
void BatchEnd(void);

/*! 
 *  \brief     设置倒计时控件
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  timeout 倒计时(秒)
 */
void SeTimer(unsigned short screen_id,unsigned short control_id,unsigned long timeout);

/*! 
 *  \brief     开启倒计时控件
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void StartTimer(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     停止倒计时控件
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void StopTimer(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     暂停倒计时控件
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void PauseTimer(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     设置控件背景色
 *  \details  支持控件：进度条、文本
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  color 背景色
 */
void SetControlBackColor(unsigned short screen_id,unsigned short control_id,unsigned short color);

/*! 
 *  \brief     设置控件前景色
  * \details  支持控件：进度条
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  color 前景色
 */
void SetControlForeColor(unsigned short screen_id,unsigned short control_id,unsigned short color);

/*! 
 *  \brief     显示\隐藏弹出菜单控件
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 *  \param  show 是否显示，为0时focus_control_id无效
 *  \param  focus_control_id 关联的文本控件(菜单控件的内容输出到文本控件)
 */
void ShowPopupMenu(unsigned short screen_id,unsigned short control_id,unsigned char show,unsigned short focus_control_id);

/*! 
 *  \brief     显示\隐藏系统键盘
 *  \param  show 0隐藏，1显示
 *  \param  x 键盘显示位置X坐标
 *  \param  y 键盘显示位置Y坐标
 *  \param  type 0小键盘，1全键盘
 *  \param  option 0正常字符，1密码，2时间设置
 *  \param  max_len 键盘录入字符长度限制
 */
void ShowKeyboard(unsigned char show,unsigned short x,unsigned short y,unsigned char type,unsigned char option,unsigned char max_len);

#if FIRMWARE_VER>=914
/*! 
 *  \brief     多语言设置
 *  \param  ui_lang 用户界面语言0~9
 *  \param  sys_lang 系统键盘语言-0中文，1英文
 */
void SetLanguage(unsigned char ui_lang,unsigned char sys_lang);
#endif

#if FIRMWARE_VER>=917
/*! 
 *  \brief     开始保存控件数值到FLASH
 *  \param  version 数据版本号，可任意指定，高16位为主版本号，低16位为次版本号
 *  \param  address 数据在用户存储区的存放地址，注意防止地址重叠、冲突
 */
void FlashBeginSaveControl(unsigned long version,unsigned long address);

/*! 
 *  \brief     保存某个控件的数值到FLASH
 *  \param  screen_id 画面ID
 *  \param  control_id 控件ID
 */
void FlashSaveControl(unsigned short screen_id,unsigned short control_id);

/*! 
 *  \brief     保存结束
 */
void FlashEndSaveControl(void);

/*! 
 *  \brief     从FLASH中恢复控件数据
 *  \param  version 数据版本号，主版本号必须与存储时一致，否则会加载失败
 *  \param  address 数据在用户存储区的存放地址
 */
void FlashRestoreControl(unsigned long version,unsigned long address);
#endif

void ReadRtc(void);

void SetRtc(unsigned short Year,unsigned char Mon,unsigned char Day,unsigned char Week,unsigned char Hour,unsigned char Min,unsigned char Sec);
#endif

void show_string(unsigned short screen_id, unsigned short x, unsigned short y, unsigned char fg, unsigned char size, unsigned char *str);

void update_lcd_enable(void);
void update_lcd_disable(void);
void clesr_text(unsigned short screen_id,unsigned short control_id);


