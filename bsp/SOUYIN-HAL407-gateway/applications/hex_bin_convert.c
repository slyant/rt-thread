#include <hex_bin_convert.h>

unsigned char hexchar2byte(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'z')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'Z')
		return c - 'A' + 10;
    else
        return c;
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
        outbuff[i] = hex2byte(&hex[i]);
    }
}

char halfbyte2hexchar(const unsigned char halfbyte)
{
	if(halfbyte >= 0 && halfbyte <=9)
	{
		return '0' + halfbyte;
	}
	else if(halfbyte >= 10 && halfbyte <= 15)
	{
		return 'A' + halfbyte - 10;
	}
	else
	{
		return 'F';
	}
}

void byte2hexchar(const unsigned char byte, char *out_hex)
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