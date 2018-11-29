#ifndef __IC_CARD_PROTOCOL_H__
#define __IC_CARD_PROTOCOL_H__

unsigned char card_check(void);
unsigned char card_init(void);
unsigned char card_reset(void);
unsigned char write_inf_card(unsigned char* dat,unsigned short length,unsigned char* out_card_id);
unsigned char rfid_scan_handle(unsigned char *out_card_id, unsigned char *outInf, unsigned short* outLength);

#endif
