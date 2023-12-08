#ifndef _LCD7789_H_
#define _LCD7789_H_

#include "system/app_core.h"
#include "system/includes.h"

#define LCD_RST IO_PORTA_02 // 复位 10MS  RST
#define LCD_RS IO_PORTA_06  // 1 寄存器数据  0 指令寄存器  D/C  A/O
#define LCD_CS IO_PORTA_05  // 片选
#define LCD_DAT IO_PORTA_03 // SPI SDA
#define LCD_CLK IO_PORTA_04 // SPI SCK

#define     RED          0XF800	  //红色
#define     GREEN        0X07E0	  //绿色
#define     BLUE         0x07FF	  //蓝色 0X001F
#define     WHITE        0XFFFF	  //白色
#define     BLACK        0X0000	  //黑色

#define Normal_DisplaySize 24

#define BACKGOUND BLACK //背景颜色
#define FRONT //字体颜色


// typedef Struct 
// {
//     uint8_t type;
//     uint8_t value;
// }Lcd_event_t;   
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
    uint8_t index[25]; //选择的菜单
}MenuData_t;

typedef struct 
{
    uint8_t mac[6]; // 10
    uint8_t useflag;   // 使用标志位
    uint8_t IsModifyFlag; // 被连接过的
    uint8_t Timeout; //超时清除
    int8_t rssi; //强度；
}Ble_Adv_Rp_t;

typedef struct 
{
    float Bat_Capcity;  //电池容量
    float Charge_Current_Max;
    float Charge_Power_Max;
    float Trickle_Current; //涓流电流

    float Low_voltage_Protect; //低压保护电压
    uint32_t Current_Gear; // 0 - 20 电压挡位
    uint32_t Ledar_Pwm; // 0 - 20 电压挡位
    uint32_t Ledar_Dly_Time; // 0 - 20 电压挡位
    uint32_t Led_Set_Pwm; 
   
    uint32_t DischarCurve_Moed; // 0 pwm模式 电流 AI

    float Curv_Data[8][2]; 

    uint32_t Solar_Mode;
    
    uint32_t Lock_Mode; //锁定模式

    uint32_t Extern_Mode; //控制模式

    uint16_t Usercode; //用户码

}Mppt_Set_Parm_t ;

typedef struct 
{
    float Charge_Capcity;          
    float Charge_Current;   
    float Charge_Power; 
    float Dischar_Current;         
    uint8_t  Bat_Resistance;      // unit: mΩ
    uint8_t  Bat_Capcity;         // unit: %
    uint8_t  OutPut_Staus;        // unit:   
    float Bat_Voltage;           // unit: V 
}Mppt_Info_Para_t;

typedef struct 
{
    uint8_t Connect_Mode;//连接模式 0手动 1批量

    Mppt_Set_Parm_t Mppt_SetPara; // MPPT 设置的参数

    Mppt_Set_Parm_t Mppt_ConSetPara_Info; // 返回的参数

    Mppt_Info_Para_t Mppt_Info;  // MPPT 返回的参数
    
    Ble_Adv_Rp_t Ble_Adv_rp[100]; //蓝牙报告
    uint8_t      Ble_Adv_Rp_Count;
    uint8_t      Ble_Connect_Mac[6];//目前蓝牙连接的地址 // 可以省略
    uint16_t    conn_handle;


   // uint16_t    Usercode; //用户码
    uint16_t    SetCount; //已计数

    uint8_t Filter_LockFlag; //过滤锁定的设备

    uint8_t ConnenctOnFlag; //允许连接标志位

    uint8_t Ble_SetConnect_Mac[6]; //设置蓝牙连接的地址

}roter_t;



MenuData_t MenuData; //显示数据
roter_t RoterData; // 遥控器数据 显示用

void Lcd_WriteCmd(uint8_t cmd);
void Lcd_WriteData(uint8_t data);
void Lcd_WriteRgbData(uint16_t data);
void Lcd_Address_Set(unsigned short int x_start,unsigned short int y_start,unsigned short int x_end,unsigned short int y_end);
void PutPixel(uint x_start,uint y_start,uint color);
void Lcd_Show20x20(uint8_t x,uint8_t y,uint8_t *p);
void Lcd_Show16x24(uint8_t x,uint8_t y,uint8_t *p);
void Lcd_Show8x16(uint8_t x,uint8_t y,uint8_t *p);
void Lcd_printf20x20(uint8_t x,uint8_t y,uint8_t *format,...);
void Lcd_printf16x16(uint8_t x,uint8_t y,uint8_t *str);  
void Lcd_Clear20x20(uint8_t x,uint8_t y);
//uint32_t floatToString(float n, u8 a, char *str);
void floatToString(float d, int l,char *str) ;
void Lcd_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[]);

void Lcd_Clear(u16 color);
void ST7789Lcd_Init(void);

//主菜单
void Mppt_Normal_Menu_Select(uint8_t key);
void Mppt_Main_Menu(void);
void Mppt_Main_Menu_Operation(uint8_t key);

//蓝牙批量设置菜单
void Mppt_Ble_BatchSet_Menu_Operation(uint8_t key);
void Mppt_Ble_BatchSet_Menu(void);

    //充电参数设置
    void Mppt_Charge_Set_Menu(void);
    void Mppt_Charge_Set_Menu_Operation(uint8_t key);

    //放电参数设置
    void Mppt_ChargePara_Display(Mppt_Set_Parm_t *SetPara);
    void Mppt_Dischar_Set_Menu(void);
    void Mppt_Dischar_Set_Menu_Operation(uint8_t key);

    //放电曲线设置
    void Mppt_Dischar_Curve_Set_Menu(void);
    void Mppt_Dischar_Curve_Set_Operation(uint8_t key);

    //MPPT 批量设置菜单
    void Mppt_Ble_AutoConnect_set_Menu(void);
    void Mppt_Ble_AutoConnect_set_Menu_Operation(uint8_t key);

//MPPT 蓝牙选择连接
void Mppt_Ble_con_Select_Menu(void);
void Mppt_Ble_con_Select_Menu_Operation(uint8_t key);
        //蓝牙连接界面
    void Mppt_Ble_con_Menu(void);
    void Mppt_Ble_con_Menu_Operation(uint8_t key);

        void Mppt_Ble_Set_Menu(void);
        void Mppt_Ble_Set_Operation(uint8_t key);

            void Mppt_Info_Display(void *priv); // MPPT参数配置
            void Mppt_Info_Menu(void); //充电信息配置
            void Mppt_Info_Menu_Operation(uint8_t key);

            void Mppt_ChargePara_Modify_Menu(void);
            void Mppt_ChargePara_Modify_Menu_Operation(uint8_t key);

            void Mppt_DischarPara_Modify_Menu(void);
            void Mppt_DischarPara_Modify_Menu_Operation(uint8_t key);

            void Mppt_CurvePara_Modify_Menu(void);
            void Mppt_CurvePara_Modify_Menu_Operation(uint8_t key);
       
            void Mppt_Comfir_Modify_Menu(void);
            void Mppt_Comfir_Modify_Menu_Operation(uint8_t key);
        

void Mppt_Ir_Set_Menu(void);
void Mppt_Ir_Set_Menu_Operation(uint8_t key) ; 

    void IR_Normal_Menu(void);
    void IR_Normal_Menu_Operation(uint8_t key) ;

    void IR_Engineer_Menu(void);
    void IR_Engineer_Menu_Operation(uint8_t key);

    void IR_Usercode_Menu(void);
    void IR_Usercode_Menu_Operation(uint8_t key);

void Mppt_Version_Info_menu(void);

void SYS_Set_Menu(void);

void Mppt_Version_Select_Menu(void);

enum
{
    MAIN_MENU,

    BLE_BATCHSET_MENU,
        CHAEGE_SET_MENU,
        DISCHAR_SET_MENU,
        DISCHAR_CURVE_SET_MENU,
        BL_ATCON_SET_MENU,

    BL_CON_MENU,
        BL_CON_SELECT_MENU,
            BL_CON_SET_MENU,
                MPPT_INFO,
                CHAEGE_PARA_MODIFY,
                DISCHAR_PARA_MODIFY,
                CURVE_PARAT_MENU,
                ENTRY_MODIFY,    
                
    IR_SET_MENU,
        IR_NORMAL_MENU,
        IR_ENGINEER_MENU,
        IR_USERCODE_MENU,

    VERSION_CHECK_MENU,
    
    SYS_SET_MENU,

    MPPT_VERSION_SELECT_MENU,

    Menu_Tab_Max,
}Menu_Tab_e;

static Menu_Tab_t const Menu_Tab[]=
{
    {MAIN_MENU,NULL,6,Mppt_Main_Menu_Operation,Mppt_Main_Menu}, //主菜单 次级菜单

    {BLE_BATCHSET_MENU,MAIN_MENU,6,Mppt_Ble_BatchSet_Menu_Operation,Mppt_Ble_BatchSet_Menu}, // 批量设置菜单
        {CHAEGE_SET_MENU,BLE_BATCHSET_MENU,4,Mppt_Charge_Set_Menu_Operation,Mppt_Charge_Set_Menu}, // 次级菜单
        {DISCHAR_SET_MENU,BLE_BATCHSET_MENU,7,Mppt_Dischar_Set_Menu_Operation,Mppt_Dischar_Set_Menu},
        {DISCHAR_CURVE_SET_MENU,BLE_BATCHSET_MENU,16,Mppt_Dischar_Curve_Set_Operation,Mppt_Dischar_Curve_Set_Menu},
        {BL_ATCON_SET_MENU,BLE_BATCHSET_MENU,3,Mppt_Ble_AutoConnect_set_Menu_Operation,Mppt_Ble_AutoConnect_set_Menu},

    
    {BL_CON_MENU,MAIN_MENU,NULL,Mppt_Ble_con_Menu_Operation,Mppt_Ble_con_Menu},
        {BL_CON_SELECT_MENU,MAIN_MENU,8,Mppt_Ble_con_Select_Menu_Operation,Mppt_Ble_con_Select_Menu},
            {BL_CON_SET_MENU,MAIN_MENU,7,Mppt_Ble_Set_Operation,Mppt_Ble_Set_Menu},
                {MPPT_INFO,BL_CON_SET_MENU,2,Mppt_Info_Menu_Operation,Mppt_Info_Menu},
                {CHAEGE_PARA_MODIFY,BL_CON_SET_MENU,4,Mppt_ChargePara_Modify_Menu_Operation,Mppt_ChargePara_Modify_Menu},
                {DISCHAR_PARA_MODIFY,BL_CON_SET_MENU,7,Mppt_DischarPara_Modify_Menu_Operation,Mppt_DischarPara_Modify_Menu},
                {CURVE_PARAT_MENU,BL_CON_SET_MENU,16,Mppt_CurvePara_Modify_Menu_Operation,Mppt_CurvePara_Modify_Menu},
                {ENTRY_MODIFY,BL_CON_SET_MENU,2,Mppt_Comfir_Modify_Menu_Operation,Mppt_Comfir_Modify_Menu},
        

    {IR_SET_MENU,MAIN_MENU,3,Mppt_Ir_Set_Menu_Operation,Mppt_Ir_Set_Menu},
        {IR_NORMAL_MENU,IR_SET_MENU,NULL,IR_Normal_Menu_Operation,IR_Normal_Menu},
        {IR_ENGINEER_MENU,IR_SET_MENU,NULL,IR_Engineer_Menu_Operation,IR_Engineer_Menu},
        {IR_USERCODE_MENU,IR_SET_MENU,2,IR_Usercode_Menu_Operation,IR_Usercode_Menu},


    {VERSION_CHECK_MENU,MAIN_MENU,NULL,Mppt_Normal_Menu_Select,Mppt_Version_Info_menu},

    {SYS_SET_MENU,MAIN_MENU,NULL,Mppt_Normal_Menu_Select,SYS_Set_Menu},

    {MPPT_VERSION_SELECT_MENU,MAIN_MENU,1,Mppt_Normal_Menu_Select,Mppt_Version_Select_Menu},

    
};

//蓝牙参数设置 
uint8_t Ble_Find_Mac_RepAddr(Ble_Adv_Rp_t *adv,uint8_t len,uint8_t *mac); // 找到数组中对应的蓝牙的地址
uint8_t Ble_Check_NonAddr(Ble_Adv_Rp_t *adv,uint8_t len);   // 找到数组中空闲的地址
void Ble_Timeout_Check(void);   // BLE广播超时选择

#endif
