#include <stdio.h>
#include <any_convert.h>

unsigned long bytes2uint32(unsigned char bytes[4])
{
	unsigned long result = 0;
	result = bytes[3]; result <<= 8;
	result |= bytes[2]; result <<=8;
	result |= bytes[1]; result <<=8;
	result |= bytes[0];
	return result;
}

//字符串转整数
long str2int32(unsigned char *str)
{
	long v = 0;
	sscanf((char *)str,"%ld",&v);
	return v;
}

unsigned char hexchar2byte(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
    else
        return 0;
}

unsigned char hex2byte(const char p[2])
{
	unsigned char tmp = 0;
	tmp = hexchar2byte(p[0]);
	tmp <<= 4;
	tmp |= hexchar2byte(p[1]);
	return tmp;
}

void hex2bytes(char* hex, int len, unsigned char* outbuff)
{
    for (int i = 0; i<len; i+=2)
    {
        *outbuff++ = hex2byte(&hex[i]);
    }
}

char halfbyte2hexchar(const unsigned char halfbyte)
{
	if(halfbyte <=9)
	{
		return '0' + halfbyte;
	}
	else if(halfbyte >= 10 && halfbyte <= 15)
	{
		return 'A' + halfbyte - 10;
	}
	else
	{
		return '0';
	}
}

void byte2hexchar(const unsigned char byte, char out_hex[2])
{
	unsigned char h, l;
	h = byte >> 4;
	l = byte & 0x0f;
	out_hex[0] = halfbyte2hexchar(h);
	out_hex[1] = halfbyte2hexchar(l);
}

void buffer2hex(unsigned char *buffer, unsigned short int buf_len, char *out_hex)
{
	int hex_index = 0;
	for(int i=0; i<buf_len; i++)
	{
		byte2hexchar(buffer[i], out_hex + hex_index);
		hex_index += 2;
	}
}
