#ifndef _LCD7789_H_
#define _LCD7789_H_

#include "system/app_core.h"
#include "system/includes.h"

#define LCD_RST IO_PORTB_09 // 复位 10MS  RST
#define LCD_RS IO_PORTB_08  // 1 寄存器数据  0 指令寄存器  D/C  A/O
#define LCD_CS IO_PORTB_07  // 片选
#define LCD_DAT IO_PORTA_01 // SPI SDA
#define LCD_CLK IO_PORTA_02 // SPI SCK

#define     RED          0XF800	  //红色
#define     GREEN        0X07E0	  //绿色
#define     BLUE         0x07FF	  //蓝色 0X001F
#define     WHITE        0XFFFF	  //白色
#define     BLACK        0X0000	  //黑色

#define BK BLACK //背景颜色
#define ZT //字体颜色

typedef struct 
{
    uint8_t current_id; //当前ID
    uint8_t up_id;      //返回上级的ID
    uint8_t max_menu_num; //最大索引
    void (*current_operation)(uint8_t); //当前ID状态应该执行的操作
    void (*display_operation)(void);        //当前ID状态应该执行的操作
}Menu_Tab_t;

typedef struct 
{
    uint8_t current_id; //当前ID
    uint8_t index[10]; //选择的菜单
}MenuData_t;

typedef struct 
{
    uint8_t mac[6]; // 10
    int8_t rssi; //强度；
    uint8_t useflag;
    uint8_t Timeout; //超时清除
}Bl_Adv_Rp_t;

typedef struct 
{
    uint8_t Ir_Onflag;  //IR控制
    uint8_t Ble_Onflag; //蓝牙通信

    uint8_t Bat_Volatage;
    uint8_t Charge_Volatage;
    uint8_t Charge_Power_Max;
    uint8_t Charge_Current_Max;

    Bl_Adv_Rp_t bl_adv_rp[14]; //蓝牙报告
    uint8_t bl_cnt;

    uint8_t bl_connect_addr[6];//目前蓝牙连接的地址

}roter_t;

MenuData_t MenuData; //显示数据
roter_t roter_data;

void Lcd_WriteCmd(uint8_t cmd);
void Lcd_WriteData(uint8_t data);
void Lcd_WriteRgbData(uint16_t data);
void Lcd_Address_Set(unsigned short int x_start,unsigned short int y_start,unsigned short int x_end,unsigned short int y_end);
void PutPixel(uint x_start,uint y_start,uint color);
void Lcd_Show24x24(uint8_t x,uint8_t y,uint8_t *p);
void Lcd_Show16x24(uint8_t x,uint8_t y,uint8_t *p);
void Lcd_Show8x16(uint8_t x,uint8_t y,uint8_t *p);
void Lcd_printf24x24(uint8_t x,uint8_t y,uint8_t *str);
void Lcd_printf16x16(uint8_t x,uint8_t y,uint8_t *str);  //must be string
void Lcd_Clear24x24(uint8_t x,uint8_t y);


void Lcd_Clear(u16 color);
void ST7789Lcd_Init(void);

void menu_select(uint8_t key);
void main_menu(void);
void main_menu_operation(uint8_t key);
void charge_set_menu(void);
void charge_menu_operation(uint8_t key);
void dischar_set_menu(void);
void IRorBL_set_menu(void);
void IRorBL_set_operation(uint8_t key);
void bl_con_set_menu(void);
void bl_con_set_operation(uint8_t key);
void bl_ATcon_set_menu(void);
void Version_Check_menu(void);

enum
{
    MAIN_MENU,
    CHAEGE_SET_MENU,
    DISCHAR_SET_MENU,
    IRORBLE_SET_MENU,
    BL_CON_SET_MENU,
    BL_ATCON_SET_MENU,
    VERSION_CHECK_MENU,

}Menu_Tab_e;

static Menu_Tab_t const Menu_Tab[10]=
{
    {MAIN_MENU,NULL,6,main_menu_operation,main_menu}, //主菜单 次级菜单

    {CHAEGE_SET_MENU,MAIN_MENU,4,menu_select,charge_set_menu}, // 次级菜单
    {DISCHAR_SET_MENU,MAIN_MENU,6,menu_select,dischar_set_menu},
    {IRORBLE_SET_MENU,MAIN_MENU,2,IRorBL_set_operation,IRorBL_set_menu},
    {BL_CON_SET_MENU,MAIN_MENU,13,bl_con_set_operation,bl_con_set_menu},
    {BL_ATCON_SET_MENU,MAIN_MENU,13,menu_select,bl_ATcon_set_menu},
    {VERSION_CHECK_MENU,MAIN_MENU,5,menu_select,Version_Check_menu},
};

uint8_t BL_Find_Mac_RepAddr(Bl_Adv_Rp_t *adv,uint8_t len,uint8_t *mac);
uint8_t BL_Check_NonAddr(Bl_Adv_Rp_t *adv,uint8_t len);

#endif
