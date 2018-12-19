

#ifndef __DATACHECK_H__
#define __DATACHECK_H__

unsigned char get_crc8(unsigned char* dat,unsigned short length);
unsigned char get_sum(unsigned char* dat,unsigned short length);
unsigned char get_xor(unsigned char* dat,unsigned short length);

#endif
