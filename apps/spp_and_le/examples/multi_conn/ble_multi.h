// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)

#ifndef _BLE_MUTIL_H
#define _BLE_MUTIL_H

#include <stdint.h>
#include "app_config.h"

void multi_server_init(void);
void multi_server_exit(void);
void multi_client_init(void);
void multi_client_exit(void);
int multi_client_clear_pair(void);
int multi_server_clear_pair(void);


#define P01 0x01
#define P02 0x81
#define P03 0x41
#define P04 0xC1
#define P05 0x21
#define P06 0xA1
#define P07 0x61
#define P08 0xE1
#define P09 0x11
#define P10 0x91
#define P11 0x51
#define P12 0xD1
#define P13 0x31
#define P14 0xB1
#define P15 0x71
#define P16 0xF1	
#define P17 0x09
#define P18 0x89
#define P19 0x49
#define P20 0xC9

uint8_t static const IR_Current_Code[20]=
{
    P01,P02,P03,P04,P05,P06,P07,P08,P09,P10,
    P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,
};

void Ir_tx_star(u8 data,u8 size); // 设置IO 电流大小
void Ir_tx_star_adr(u16 data,u8 size);
void Ir_tx_star_Def(u8 *data); //
void Ir_tx_star_x(uint8_t *data,uint8_t len);

void Timer_Auto_Off_ReCount(void);

void Mppt_Set_Para_Send(void *priv);
void Get_Mppt_Report(void);
void Get_Mppt_Report1(void);
void MPPT_Get_Info_Timer_Star(void);
#endif
