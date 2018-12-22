#ifndef __HEX_BIN_CONVERT_H__
#define __HEX_BIN_CONVERT_H__

unsigned char hexchar2byte(char c);
unsigned char hex2byte(const char p[2]);
void hex2bytes(char* hex, int len, unsigned char* outbuff);

char halfbyte2hexchar(const unsigned char halfbyte);
void byte2hexchar(const unsigned char byte, char *out_hex);
void buffer2hex(unsigned char *buffer, unsigned short int buf_len, char *hex);

#endif
