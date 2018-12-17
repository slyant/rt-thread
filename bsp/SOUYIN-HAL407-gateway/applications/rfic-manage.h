


#ifndef _RFIC_MANAGE_H
#define _RFIC_MANAGE_H


typedef struct 
{
	uint8_t typ;
	uint8_t jskeya[6];
	uint8_t jskeyb[6];
	char  jspwd[16];
	char  jsiden[10];
}card_msg;


uint8_t decode_json(char *json);
uint8_t read_json_from_card(void);

void key_card_init(void);
void key_card_make(void);
void key_card_clear(void);
void key_card_backup(void);
void key_card_recovery(void);

void card_init_fun(uint8_t typ);
void card_make_fun(uint8_t typ);
void card_clear_fun(uint8_t typ);
uint8_t card_service_fun(uint8_t typ);

int check_cof_card_cun(void);

#endif

