#include <rtthread.h>
#include <hmi_driver.h>

extern void send_char(rt_uint8_t c);	//from uart_lcd_device.c

#define TX_8(P1) SEND_DATA((P1)&0xFF)  //发送单个字节
#define TX_8N(P,N) SendNU8((unsigned char *)P,N)  //发送N个字节
#define TX_16(P1) TX_8((P1)>>8);TX_8(P1)  //发送16位整数
#define TX_16N(P,N) SendNU16((unsigned short *)P,N)  //发送N个16位整数
#define TX_32(P1) TX_16((P1)>>16);TX_16((P1)&0xFFFF)  //发送32位整数

#if(CRC16_ENABLE)

static unsigned short _crc16 = 0xffff;
static void AddCRC16(unsigned char *buffer,unsigned short n,unsigned short *pcrc)
{
	unsigned short i,j,carry_flag,a;

	for (i=0; i<n; i++)
	{
		*pcrc=*pcrc^buffer[i];
		for (j=0; j<8; j++)
		{
			a=*pcrc;
			carry_flag=a&0x0001;
			*pcrc=*pcrc>>1;
			if (carry_flag==1)
				*pcrc=*pcrc^0xa001;
		}
	}
}

unsigned short CheckCRC16(unsigned char *buffer,unsigned short n)
{
	unsigned short crc0 = 0x0;
	unsigned short crc1 = 0xffff;

	if(n>=2)
	{
		crc0 = ((buffer[n-2]<<8)|buffer[n-1]);
		AddCRC16(buffer,n-2,&crc1);
	}

	return (crc0==crc1);
}

void SEND_DATA(unsigned char c)
{
	AddCRC16(&c,1,&_crc16);
	SendChar(c);
}

void BEGIN_CMD()
{
	TX_8(0XEE);
	_crc16 = 0XFFFF;//开始计算CRC16
}

void END_CMD()
{
	unsigned short crc16 = _crc16;
	TX_16(crc16);//发送CRC16
	TX_32(0XFFFCFFFF);
}

#else//NO CRC16

#define SEND_DATA(P) send_char(P)
#define BEGIN_CMD() TX_8(0XEE)
#define END_CMD() TX_32(0XFFFCFFFF)

#endif

void SendStrings(unsigned char *str)
{
	while(*str)
	{
		TX_8(*str);
		str++;
	}
}

void SendNU8(unsigned char *pData,unsigned short nDataLen)
{
	unsigned short i = 0;
	for (;i<nDataLen;++i)
	{
		TX_8(pData[i]);
	}
}

void SendNU16(unsigned short *pData,unsigned short nDataLen)
{
	unsigned short i = 0;
	for (;i<nDataLen;++i)
	{
		TX_16(pData[i]);
	}
}

void SetHandShake()
{
	BEGIN_CMD();
	TX_8(0x00);
	END_CMD();
}

void SetFcolor(unsigned short color)
{
	BEGIN_CMD();
	TX_8(0x41);
	TX_16(color);
	END_CMD();
}

void SetBcolor(unsigned short color)
{
	BEGIN_CMD();
	TX_8(0x42);
	TX_16(color);
	END_CMD();
}

void ColorPicker(unsigned char mode, unsigned short x,unsigned short y)
{
	BEGIN_CMD();
	TX_8(0xA3);
	TX_8(mode);
	TX_16(x);
	TX_16(y);
	END_CMD();
}

void GUI_CleanScreen()
{
	BEGIN_CMD();
	TX_8(0x01);
	END_CMD();
}

void SetTextSpace(unsigned char x_w, unsigned char y_w)
{
	BEGIN_CMD();
	TX_8(0x43);
	TX_8(x_w);
	TX_8(y_w);
	END_CMD();
}

void SetFont_Region(unsigned char enable,unsigned short width,unsigned short height)
{
	BEGIN_CMD();
	TX_8(0x45);
	TX_8(enable);
	TX_16(width);
	TX_16(height);
	END_CMD();
}

void SetFilterColor(unsigned short fillcolor_dwon, unsigned short fillcolor_up)
{
	BEGIN_CMD();
	TX_8(0x44);
	TX_16(fillcolor_dwon);
	TX_16(fillcolor_up);
	END_CMD();
}

void DisText(unsigned short x, unsigned short y,unsigned char back,unsigned char font,unsigned char *strings )
{
	BEGIN_CMD();
	TX_8(0x20);
	TX_16(x);
	TX_16(y);
	TX_8(back);
	TX_8(font);
	SendStrings(strings);
	END_CMD();
}

void DisCursor(unsigned char enable,unsigned short x, unsigned short y,unsigned char width,unsigned char height )
{
	BEGIN_CMD();
	TX_8(0x21);
	TX_8(enable);
	TX_16(x);
	TX_16(y);
	TX_8(width);
	TX_8(height);
	END_CMD();
}

void DisFull_Image(unsigned short image_id,unsigned char masken)
{
	BEGIN_CMD();
	TX_8(0x31);
	TX_16(image_id);
	TX_8(masken);
	END_CMD();
}

void DisArea_Image(unsigned short x,unsigned short y,unsigned short image_id,unsigned char masken)
{
	BEGIN_CMD();
	TX_8(0x32);
	TX_16(x);
	TX_16(y);
	TX_16(image_id);
	TX_8(masken);
	END_CMD();
}

void DisCut_Image(unsigned short x,unsigned short y,unsigned short image_id,unsigned short image_x,unsigned short image_y,unsigned short image_l, unsigned short image_w,unsigned char masken)
{
	BEGIN_CMD();
	TX_8(0x33);
	TX_16(x);
	TX_16(y);
	TX_16(image_id);
	TX_16(image_x);
	TX_16(image_y);
	TX_16(image_l);
	TX_16(image_w);
	TX_8(masken);
	END_CMD();
}

void DisFlashImage(unsigned short x,unsigned short y,unsigned short flashimage_id,unsigned char enable,unsigned char playnum)
{
	BEGIN_CMD();
	TX_8(0x80);
	TX_16(x);
	TX_16(y);
	TX_16(flashimage_id);
	TX_8(enable);
	TX_8(playnum);
	END_CMD();
}

void GUI_Dot(unsigned short x,unsigned short y)
{
	BEGIN_CMD();
	TX_8(0x50);
	TX_16(x);
	TX_16(y);
	END_CMD();
}

void GUI_Line(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1)
{
	BEGIN_CMD();
	TX_8(0x51);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void GUI_ConDots(unsigned char mode,unsigned short *dot,unsigned short dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x63);
	TX_8(mode);
	TX_16N(dot,dot_cnt*2);
	END_CMD();
}

void GUI_ConSpaceDots(unsigned short x,unsigned short x_space,unsigned short *dot_y,unsigned short dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x59);
	TX_16(x);
	TX_16(x_space);
	TX_16N(dot_y,dot_cnt);
	END_CMD();
}

void GUI_FcolorConOffsetDots(unsigned short x,unsigned short y,unsigned short *dot_offset,unsigned short dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x75);
	TX_16(x);
	TX_16(y);
	TX_16N(dot_offset,dot_cnt);
	END_CMD();
}

void GUI_BcolorConOffsetDots(unsigned short x,unsigned short y,unsigned char *dot_offset,unsigned short dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x76);
	TX_16(x);
	TX_16(y);
	TX_16N(dot_offset,dot_cnt);
	END_CMD();
}

void SetPowerSaving(unsigned char enable, unsigned char bl_off_level, unsigned char bl_on_level, unsigned char  bl_on_time)
{
	BEGIN_CMD();
	TX_8(0x77);
	TX_8(enable);
	TX_8(bl_off_level);
	TX_8(bl_on_level);
	TX_8(bl_on_time);
	END_CMD();
}

void GUI_FcolorConDots(unsigned short *dot,unsigned short dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x68);
	TX_16N(dot,dot_cnt*2);
	END_CMD();
}

void GUI_BcolorConDots(unsigned short *dot,unsigned short dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x69);
	TX_16N(dot,dot_cnt*2);
	END_CMD();
}

void GUI_Circle(unsigned short x, unsigned short y, unsigned short r)
{
	BEGIN_CMD();
	TX_8(0x52);
	TX_16(x);
	TX_16(y);
	TX_16(r);
	END_CMD();
}

void GUI_CircleFill(unsigned short x, unsigned short y, unsigned short r)
{
	BEGIN_CMD();
	TX_8(0x53);
	TX_16(x);
	TX_16(y);
	TX_16(r);
	END_CMD();
}

void GUI_Arc(unsigned short x,unsigned short y, unsigned short r,unsigned short sa, unsigned short ea)
{
	BEGIN_CMD();
	TX_8(0x67);
	TX_16(x);
	TX_16(y);
	TX_16(r);
	TX_16(sa);
	TX_16(ea);
	END_CMD();
}

void GUI_Rectangle(unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 )
{
	BEGIN_CMD();
	TX_8(0x54);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void GUI_RectangleFill(unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 )
{
	BEGIN_CMD();
	TX_8(0x55);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void GUI_Ellipse(unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 )
{
	BEGIN_CMD();
	TX_8(0x56);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void GUI_EllipseFill(unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 )
{
	BEGIN_CMD();
	TX_8(0x57);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void SetBackLight(unsigned char light_level)
{
	BEGIN_CMD();
	TX_8(0x60);
	TX_8(light_level);
	END_CMD();
}

void SetBuzzer(unsigned char time)
{
	BEGIN_CMD();
	TX_8(0x61);
	TX_8(time);
	END_CMD();
}

void GUI_AreaInycolor(unsigned short x0, unsigned short y0, unsigned short x1,unsigned short y1 )
{
	BEGIN_CMD();
	TX_8(0x65);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void SetTouchPaneOption(unsigned char enbale,unsigned char beep_on,unsigned char work_mode,unsigned char press_calibration)
{
	unsigned char options = 0;

	if(enbale)
		options |= 0x01;
	if(beep_on)
		options |= 0x02;
	if(work_mode)
		options |= (work_mode<<2);
	if(press_calibration)
		options |= (press_calibration<<5);

	BEGIN_CMD();
	TX_8(0x70);
	TX_8(options);
	END_CMD();
}

void CalibrateTouchPane()
{
	BEGIN_CMD();
	TX_8(0x72);
	END_CMD();
}

void TestTouchPane()
{
	BEGIN_CMD();
	TX_8(0x73);
	END_CMD();
}

void LockDeviceConfig(void)
{
	BEGIN_CMD();
	TX_8(0x09);
	TX_8(0xDE);
	TX_8(0xED);
	TX_8(0x13);
	TX_8(0x31);
	END_CMD();
}

void UnlockDeviceConfig(void)
{
	BEGIN_CMD();
	TX_8(0x08);
	TX_8(0xA5);
	TX_8(0x5A);
	TX_8(0x5F);
	TX_8(0xF5);
	END_CMD();
}

void SetCommBps(unsigned char option)
{
	BEGIN_CMD();
	TX_8(0xA0);
	TX_8(option);
	END_CMD();
}

void WriteLayer(unsigned char layer)
{
	BEGIN_CMD();
	TX_8(0xA1);
	TX_8(layer);
	END_CMD();
}

void DisplyLayer(unsigned char layer)
{
	BEGIN_CMD();
	TX_8(0xA2);
	TX_8(layer);
	END_CMD();
}

void CopyLayer(unsigned char src_layer,unsigned char dest_layer)
{
	BEGIN_CMD();
	TX_8(0xA4);
	TX_8(src_layer);
	TX_8(dest_layer);
	END_CMD();
}

void ClearLayer(unsigned char layer)
{
	BEGIN_CMD();
	TX_8(0x05);
	TX_8(layer);
	END_CMD();
}

void GUI_DispRTC(unsigned char enable,unsigned char mode,unsigned char font,unsigned short color,unsigned short x,unsigned short y)
{
	BEGIN_CMD();
	TX_8(0x85);
	TX_8(enable);
	TX_8(mode);
	TX_8(font);
	TX_16(color);
	TX_16(x);
	TX_16(y);
	END_CMD();
}

void WriteUserFlash(unsigned long startAddress,unsigned short length,unsigned char *_data)
{
	BEGIN_CMD();
	TX_8(0x87);
	TX_32(startAddress);
	TX_8N(_data,length);
	END_CMD();
}

void ReadUserFlash(unsigned long startAddress,unsigned short length)
{
	BEGIN_CMD();
	TX_8(0x88);
	TX_32(startAddress);
	TX_16(length);
	END_CMD();
}

void GetScreen()
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x01);
	END_CMD();
}

void SetScreen(unsigned short screen_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x00);
	TX_16(screen_id);
	END_CMD();
}

void SetScreenUpdateEnable(unsigned char enable)
{
	BEGIN_CMD();
	TX_8(0xB3);
	TX_8(enable);
	END_CMD();
}

void SetControlFocus(unsigned short screen_id,unsigned short control_id,unsigned char focus)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x02);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(focus);
	END_CMD();
}

void SetControlVisiable(unsigned short screen_id,unsigned short control_id,unsigned char visible)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x03);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(visible);
	END_CMD();
}

void SetControlEnable(unsigned short screen_id,unsigned short control_id,unsigned char enable)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x04);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(enable);
	END_CMD();
}

void SetButtonValue(unsigned short screen_id,unsigned short control_id,unsigned char state)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(state);
	END_CMD();
}

void SetTextValue(unsigned short screen_id,unsigned short control_id,unsigned char *str)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	SendStrings(str);
	END_CMD();
}

#if FIRMWARE_VER>=908

void SetTextInt32(unsigned short screen_id,unsigned short control_id,unsigned long value,unsigned char sign,unsigned char fill_zero)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x07);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(sign?0X01:0X00);
	TX_8((fill_zero&0x0f)|0x80);
	TX_32(value);
	END_CMD();
}

void SetTextFloat(unsigned short screen_id,unsigned short control_id,float value,unsigned char precision,unsigned char show_zeros)
{
	unsigned char i = 0;

	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x07);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(0x02);
	TX_8((precision&0x0f)|(show_zeros?0x80:0x00));
	
	for (i=0;i<4;++i)
	{
		TX_8(((unsigned char *)&value)[i]);
	}

	END_CMD();
}

#endif

void SetProgressValue(unsigned short screen_id,unsigned short control_id,unsigned long value)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_32(value);
	END_CMD();
}

void SetMeterValue(unsigned short screen_id,unsigned short control_id,unsigned long value)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_32(value);
	END_CMD();
}

void SetSliderValue(unsigned short screen_id,unsigned short control_id,unsigned long value)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_32(value);
	END_CMD();
}

void SetSelectorValue(unsigned short screen_id,unsigned short control_id,unsigned char item)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(item);
	END_CMD();
}

void GetControlValue(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x11);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationStart(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x20);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationStop(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x21);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationPause(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x22);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationPlayFrame(unsigned short screen_id,unsigned short control_id,unsigned char frame_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x23);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(frame_id);
	END_CMD();
}

void AnimationPlayPrev(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x24);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationPlayNext(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x25);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void GraphChannelAdd(unsigned short screen_id,unsigned short control_id,unsigned char channel,unsigned short color)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x30);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(channel);
	TX_16(color);
	END_CMD();
}

void GraphChannelDel(unsigned short screen_id,unsigned short control_id,unsigned char channel)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x31);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(channel);
	END_CMD();
}

void GraphChannelDataAdd(unsigned short screen_id,unsigned short control_id,unsigned char channel,unsigned char *pData,unsigned short nDataLen)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x32);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(channel);
	TX_16(nDataLen);
	TX_8N(pData,nDataLen);
	END_CMD();
}

void GraphChannelDataClear(unsigned short screen_id,unsigned short control_id,unsigned char channel)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x33);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(channel);
	END_CMD();
}

void GraphSetViewport(unsigned short screen_id,unsigned short control_id,short x_offset,unsigned short x_mul,short y_offset,unsigned short y_mul)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x34);
	TX_16(screen_id);
	TX_16(control_id);
	TX_16(x_offset);
	TX_16(x_mul);
	TX_16(y_offset);
	TX_16(y_mul);
	END_CMD();
}

void BatchBegin(unsigned short screen_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x12);
	TX_16(screen_id);
}

void BatchSetButtonValue(unsigned short control_id,unsigned char state)
{
	TX_16(control_id);
	TX_16(1);
	TX_8(state);
}

void BatchSetProgressValue(unsigned short control_id,unsigned long value)
{
	TX_16(control_id);
	TX_16(4);
	TX_32(value);
}

void BatchSetSliderValue(unsigned short control_id,unsigned long value)
{
	TX_16(control_id);
	TX_16(4);
	TX_32(value);
}

void BatchSetMeterValue(unsigned short control_id,unsigned long value)
{
	TX_16(control_id);
	TX_16(4);
	TX_32(value);
}

unsigned long GetStringLen(unsigned char *str)
{
	unsigned char *p = str;
	while(*str)
	{
		str++;
	}

	return (str-p);
} 

void BatchSetText(unsigned short control_id,unsigned char *strings)
{
	TX_16(control_id);
	TX_16(GetStringLen(strings));
	SendStrings(strings);
}
void BatchSetValueInt32(unsigned short control_id,long value)
{
    char buffer[12] = {0};
	rt_kprintf(buffer,"%ld",value);
	TX_16(control_id);
	TX_16(GetStringLen((unsigned char*)buffer));
	SendStrings((unsigned char*)buffer);
}
void BatchSetFrame(unsigned short control_id,unsigned short frame_id)
{
	TX_16(control_id);
	TX_16(2);
	TX_16(frame_id);
}

#if FIRMWARE_VER>=908

void BatchSetVisible(unsigned short control_id,unsigned char visible)
{
	TX_16(control_id);
	TX_8(1);
	TX_8(visible);
}

void BatchSetEnable(unsigned short control_id,unsigned char enable)
{
	TX_16(control_id);
	TX_8(2);
	TX_8(enable);
}

#endif

void BatchEnd()
{
	END_CMD();
}

void SeTimer(unsigned short screen_id,unsigned short control_id,unsigned long timeout)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x40);
	TX_16(screen_id);
	TX_16(control_id);
	TX_32(timeout);
	END_CMD();
}

void StartTimer(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x41);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void StopTimer(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x42);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void PauseTimer(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x44);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void SetControlBackColor(unsigned short screen_id,unsigned short control_id,unsigned short color)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x18);
	TX_16(screen_id);
	TX_16(control_id);
	TX_16(color);
	END_CMD();
}

void SetControlForeColor(unsigned short screen_id,unsigned short control_id,unsigned short color)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x19);
	TX_16(screen_id);
	TX_16(control_id);
	TX_16(color);
	END_CMD();
}

void ShowPopupMenu(unsigned short screen_id,unsigned short control_id,unsigned char show,unsigned short focus_control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x13);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(show);
	TX_16(focus_control_id);
	END_CMD();
}

void ShowKeyboard(unsigned char show,unsigned short x,unsigned short y,unsigned char type,unsigned char option,unsigned char max_len)
{
	BEGIN_CMD();
	TX_8(0x86);
	TX_8(show);
	TX_16(x);
	TX_16(y);
	TX_8(type);
	TX_8(option);
	TX_8(max_len);
	END_CMD();
}

#if FIRMWARE_VER>=914
void SetLanguage(unsigned char ui_lang,unsigned char sys_lang)
{
	unsigned char lang = ui_lang;
	if(sys_lang)	lang |= 0x80;

	BEGIN_CMD();
	TX_8(0xC1);
	TX_8(lang);
	TX_8(0xC1+lang);//校验，防止意外修改语言
	END_CMD();
}
#endif


#if FIRMWARE_VER>=917

void FlashBeginSaveControl(unsigned long version,unsigned long address)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0xAA);
	TX_32(version);
	TX_32(address);
}

void FlashSaveControl(unsigned short screen_id,unsigned short control_id)
{
	TX_16(screen_id);
	TX_16(control_id);
}

void FlashEndSaveControl()
{
	END_CMD();
}

void FlashRestoreControl(unsigned long version,unsigned long address)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0xAB);
	TX_32(version);
	TX_32(address);
	END_CMD();
}
void ReadRtc(void)
{
    BEGIN_CMD();
	TX_8(0x82);
	END_CMD();
}
void SetRtc(unsigned short Year,unsigned char Mon,unsigned char Day,unsigned char Week,unsigned char Hour,unsigned char Min,unsigned char Sec)
{
    unsigned char y,m,d,hh,mm,ss;
    Year = Year%100;
    y = Year/10;y <<= 4;y |= Year%10;
    m = Mon/10;m <<= 4;m |= Mon%10;
    d = Day/10;d <<= 4;d |= Day%10;
    hh = Hour/10;hh <<= 4;hh |= Hour%10;
    mm = Min/10;mm <<=4;mm |= Min%10;
    ss = Sec/10;ss <<= 4;ss |= Sec%10;
    BEGIN_CMD();
	TX_8(0x81);
    TX_8(ss);
    TX_8(mm);
    TX_8(hh);
    TX_8(d);
    TX_8(Week);
    TX_8(m);
    TX_8(y);
	END_CMD();    
}

void show_string(unsigned short screen_id, unsigned short x, unsigned short y, unsigned char fg, unsigned char size, unsigned char *str)
{
	BEGIN_CMD();
	TX_8(0x20);
	TX_16(x);
	TX_16(y);
	TX_8(fg);
	TX_8(size);
	SendStrings(str);
	END_CMD();
}

void update_lcd_enable(void)
{
	BEGIN_CMD();
	TX_16(0xB301);
	END_CMD();
}

void update_lcd_disable(void)
{
	BEGIN_CMD();
	TX_16(0xB300);
	END_CMD();
}

void clesr_text(unsigned short screen_id,unsigned short control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}


#endif
