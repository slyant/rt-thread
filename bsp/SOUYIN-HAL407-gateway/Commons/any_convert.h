#ifndef __ANY_CONVERT_H__
#define __ANY_CONVERT_H__

unsigned char hexchar2byte(char c);
unsigned char hex2byte(const char p[2]);
void hex2bytes(char* hex, int len, unsigned char* outbuff);

long str2int32(unsigned char *str);
char halfbyte2hexchar(const unsigned char halfbyte);
void byte2hexchar(const unsigned char byte, char out_hex[2]);
void buffer2hex(unsigned char *buffer, unsigned short int buf_len, char *hex);

#endif
