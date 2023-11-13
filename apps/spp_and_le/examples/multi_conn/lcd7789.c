
#include "lcd7789.h"
#include "lcd_data.h"
#include "asm/spi.h"

#include "stdarg.h"
#include "stdlib.h"

#define log_info(x, ...) printf("[INFO]" x " ", ##__VA_ARGS__)

MenuData_t MenuData = {0}; // 5个菜单

roter_t roter_data = {0};

void Lcd_WriteCmd(uint8_t cmd)
{
    gpio_write(LCD_RS, 0);
    gpio_write(LCD_CS, 0);
    spi_send_byte(SPI1, cmd);
    gpio_write(LCD_CS, 1);
}
void Lcd_WriteData(uint8_t data)
{
    gpio_write(LCD_RS, 1);
    gpio_write(LCD_CS, 0);
    spi_send_byte(SPI1, data);
    gpio_write(LCD_CS, 1);
}
void Lcd_WriteRgbData(uint16_t data)
{
    uint8_t spi_tx_buf[2];
    gpio_write(LCD_RS, 1);
    gpio_write(LCD_CS, 0);
    spi_tx_buf[0] = data >> 8;
    spi_tx_buf[1] = data;
    spi_dma_send(SPI1, spi_tx_buf, 2);
    gpio_write(LCD_CS, 1);
    // Lcd_WriteData(data>>8);
    // Lcd_WriteData(data);
}
void Lcd_Address_Set(unsigned short int x_start, unsigned short int y_start, unsigned short int x_end, unsigned short int y_end)
{
    uint16_t x = x_start + 0, y = x_end + 0;
    Lcd_WriteCmd(0x2a);    // Column address set
    Lcd_WriteData(x >> 8); // start column
    Lcd_WriteData(x);
    Lcd_WriteData(y >> 8); // end column
    Lcd_WriteData(y);
    x = y_start + 0;
    y = y_end + 0;
    Lcd_WriteCmd(0x2b);    // Row address set
    Lcd_WriteData(x >> 8); // start row
    Lcd_WriteData(x);
    Lcd_WriteData(y >> 8); // end row
    Lcd_WriteData(y);
    Lcd_WriteCmd(0x2C); // Memory write
}
void PutPixel(uint x_start, uint y_start, uint color)
{
    Lcd_Address_Set(x_start, y_start, x_start + 1, y_start + 1);
    Lcd_WriteRgbData(color);
}
void Lcd_Show24x24(uint8_t x, uint8_t y, uint8_t *p)
{
    uint8_t i, j, k;
    uint8_t temp; // 缓存
    Lcd_Address_Set(x, y, x + 23, y + 23);
    for (i = 0; i < 72; i++)
    {
        temp = *p++;
        for (k = 0; k < 8; k++)
        {
            if (temp & 0x80)
                Lcd_WriteRgbData(BLUE);
            else
                Lcd_WriteRgbData(BLACK);
            temp <<= 1;
        }
    }
}
void Lcd_Show16x24(uint8_t x, uint8_t y, uint8_t *p)
{
    uint8_t i, j, k;
    uint8_t temp; // 缓存
    uint8_t str[40];

    Lcd_Address_Set(x, y, x + 15, y + 23);
    for (i = 0; i < 48; i++)
    {
        temp = *p++;
        for (k = 0; k < 8; k++)
        {
            if (temp & 0x80)
                Lcd_WriteRgbData(BLUE);
            else
                Lcd_WriteRgbData(BLACK);
            temp <<= 1;
        }
    }
}
void Lcd_Show8x16(uint8_t x, uint8_t y, uint8_t *p)
{
    uint8_t i, j, k;
    uint8_t temp; // 缓存
    Lcd_Address_Set(x, y, x + 7, y + 15);
    for (i = 0; i < 16; i++)
    {
        temp = *p++;
        for (k = 0; k < 8; k++)
        {
            if (temp & 0x80)
                Lcd_WriteRgbData(BLUE);
            else
                Lcd_WriteRgbData(BLACK);
            temp <<= 1;
        }
    }
}
void Lcd_printf24x24(uint8_t x, uint8_t y, uint8_t *format, ...) // must be string
{
    uint8_t SearchData[4];
    char str_data[40];
    va_list ap;
    va_start(ap, format);
    vsnprintf(str_data, 40, format, ap);
    char *str = str_data;
    char *w;
    while (*str)
    {
        if (*str & 0x80) // 汉字
        {
            SearchData[0] = str[0];
            SearchData[1] = str[1];
            SearchData[2] = str[2];
            SearchData[3] = 0;
            w = strstr(DotTbl24String, SearchData);
            if (!w)
                return;
            Lcd_Show24x24(x, y, DotTbl24[w - DotTbl24String]);
            x += 24;
            str += 3;
        }
        else
        {
            SearchData[0] = str[0];
            SearchData[1] = 0;
            w = strstr(DotTbl24AsciiString, SearchData);
            Lcd_Show16x24(x, y, DotTbl24Ascii[w - DotTbl24AsciiString]);
            x += 16;
            str++;
        }
    }
}
 
void Lcd_printf16x16(uint8_t x, uint8_t y, uint8_t *str) // must be string
{
    uint8_t SearchData[4] = {0, 0, 0};
    char *w;
    while (*str)
    {
        if (*str & 0x80) // 汉字
        {
            str += 3;
        }
        else
        {
            SearchData[0] = str[0];
            SearchData[1] = 0;
            w = strstr(DotTbl16AsciiString, SearchData);
            Lcd_Show8x16(x, y, DotTbl16Ascii[w - DotTbl16AsciiString]);
            x += 8;
            str++;
        }
    }
}

void Lcd_Clear24x24(uint8_t x, uint8_t y)
{
    uint8_t i, j, k;
    uint8_t temp; // 缓存
    Lcd_Address_Set(x, y, x + 23, y + 23);
    for (i = 0; i < 72; i++)
    {
        for (k = 0; k < 8; k++)
        {
            Lcd_WriteRgbData(BLACK);
        }
    }
}
void Lcd_Clear(u16 color)
{
    u16 i, j;
    Lcd_Address_Set(0, 0, 240, 240);
    for (i = 0; i < 240; i++)
    {
        for (j = 0; j < 240; j++)
        {
            Lcd_WriteRgbData(color);
        }
    }
}
void ST7789Lcd_Init(void)
{
    gpio_set_direction(LCD_RST, 0);
    gpio_set_pull_up(LCD_RST, 0);
    gpio_set_pull_down(LCD_RST, 0);
    gpio_write(LCD_RST, 0);
    gpio_set_direction(LCD_RS, 0);
    gpio_set_pull_up(LCD_RS, 0);
    gpio_set_pull_down(LCD_RS, 0);
    gpio_write(LCD_RS, 0);
    gpio_set_direction(LCD_CS, 0);
    gpio_set_pull_up(LCD_CS, 0);
    gpio_set_pull_down(LCD_CS, 0);
    gpio_write(LCD_CS, 1);
    //-----------------------------------ST7789S reset sequence------------------------------------//
    gpio_write(LCD_RST, 0);
    os_time_dly(15);
    gpio_write(LCD_RST, 1);
    os_time_dly(15);
    Lcd_WriteCmd(0x11); // 无此指令，不能正常初始化芯片，无显示
    os_time_dly(12);
    //--------------------------------ST7789S Frame rate setting----------------------------------//
    Lcd_WriteCmd(0x2a);  // Column address set
    Lcd_WriteData(0x00); // start column
    Lcd_WriteData(0x00);
    Lcd_WriteData(0x00); // end column
    Lcd_WriteData(0xef);
    Lcd_WriteCmd(0x2b);  // Row address set
    Lcd_WriteData(0x00); // start row
    Lcd_WriteData(0x28);
    Lcd_WriteData(0x01); // end row
    Lcd_WriteData(0x17);
    Lcd_WriteCmd(0xb2); // Porch control
    Lcd_WriteData(0x0c);
    Lcd_WriteData(0x0c);
    Lcd_WriteData(0x00);
    Lcd_WriteData(0x33);
    Lcd_WriteData(0x33);
    Lcd_WriteCmd(0x20);  // Display Inversion Off
    Lcd_WriteCmd(0xb7);  // Gate control
    Lcd_WriteData(0x56); // 35
    //---------------------------------ST7789S Power setting--------------------------------------//
    Lcd_WriteCmd(0xbb);  // VCOMS Setting
    Lcd_WriteData(0x18); // 1f
    Lcd_WriteCmd(0xc0);  // LCM Control
    Lcd_WriteData(0x2c);
    Lcd_WriteCmd(0xc2); // VDV and VRH Command Enable
    Lcd_WriteData(0x01);
    Lcd_WriteCmd(0xc3);  // VRH Set
    Lcd_WriteData(0x1f); // 12
    Lcd_WriteCmd(0xc4);  // VDV Setting
    Lcd_WriteData(0x20);
    Lcd_WriteCmd(0xc6); // FR Control 2
    Lcd_WriteData(0x0f);
    Lcd_WriteCmd(0xd0);  // Power Control 1
    Lcd_WriteData(0xa6); // a4
    Lcd_WriteData(0xa1);
    //--------------------------------ST7789S gamma setting---------------------------------------//
    Lcd_WriteCmd(0xe0);
    Lcd_WriteData(0xd0);
    Lcd_WriteData(0x0d);
    Lcd_WriteData(0x14);
    Lcd_WriteData(0x0b);
    Lcd_WriteData(0x0b);
    Lcd_WriteData(0x07);
    Lcd_WriteData(0x3a);
    Lcd_WriteData(0x44);
    Lcd_WriteData(0x50);
    Lcd_WriteData(0x08);
    Lcd_WriteData(0x13);
    Lcd_WriteData(0x13);
    Lcd_WriteData(0x2d);
    Lcd_WriteData(0x32);
    Lcd_WriteCmd(0xe1); // Negative Voltage Gamma Contro
    Lcd_WriteData(0xd0);
    Lcd_WriteData(0x0d);
    Lcd_WriteData(0x14);
    Lcd_WriteData(0x0b);
    Lcd_WriteData(0x0b);
    Lcd_WriteData(0x07);
    Lcd_WriteData(0x3a);
    Lcd_WriteData(0x44);
    Lcd_WriteData(0x50);
    Lcd_WriteData(0x08);
    Lcd_WriteData(0x13);
    Lcd_WriteData(0x13);
    Lcd_WriteData(0x2d);
    Lcd_WriteData(0x32);
    Lcd_WriteCmd(0x36); // Memory data access control
    Lcd_WriteData(0x00);
    Lcd_WriteCmd(0x3A);  // Interface pixel format
    Lcd_WriteData(0x55); // 65K
    Lcd_WriteCmd(0xe7);  // SPI2 enable    启用2数据通道模式
    Lcd_WriteData(0x00);

    Lcd_WriteCmd(0x21); // Display inversion on
    Lcd_Clear(BLACK);

    Lcd_WriteCmd(0x29); // Display o
}

void menu_select_display()
{
    uint8_t i;
    for (i = 1; i <= 7; i++)
        Lcd_Clear24x24(5, 30 * i);

    // 限制下标长度
    if (!MenuData.index[MenuData.current_id])
        MenuData.index[MenuData.current_id] = 1;
    if (MenuData.index[MenuData.current_id] > 7)
        MenuData.index[MenuData.current_id] = 7;
    Lcd_printf24x24(5, 30 * MenuData.index[MenuData.current_id], "＞");
}

void BL_menu_select_display()
{
    uint8_t i;
    for (i = 1; i <= 13; i++)
        Lcd_printf16x16(5, 16 + 16 * i, " ");

    // 限制下标长度
    if (!MenuData.index[MenuData.current_id])
        MenuData.index[MenuData.current_id] = 1;
    if (MenuData.index[MenuData.current_id] > 13)
        MenuData.index[MenuData.current_id] = 13;
    Lcd_printf16x16(5, 16 + 16 * MenuData.index[MenuData.current_id], ">");
}

void menu_select(uint8_t key)
{
    switch (key)
    {
    case 0: // 上
        if (MenuData.index[MenuData.current_id] > 1)
            MenuData.index[MenuData.current_id]--;
        else
            MenuData.index[MenuData.current_id] = Menu_Tab[MenuData.current_id].max_menu_num;
        menu_select_display();
        break;
    case 1: // 下
        if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
            MenuData.index[MenuData.current_id]++;
        else
            MenuData.index[MenuData.current_id] = 1;
        menu_select_display();
        break;
    case 2: // ←
        MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
            Menu_Tab[MenuData.current_id].display_operation();
        break;
    }
}

void main_menu() // 主菜单
{
    Lcd_Clear(BLACK);
    Lcd_printf24x24(120 - 24 * 5, 0, "ＭＰＰＴ太阳能控制器");
    Lcd_printf24x24(30, 30, "充电参数设置");
    Lcd_printf24x24(30, 60, "放电参数设置");
    Lcd_printf24x24(30, 90, "蓝牙红外设置");
    Lcd_printf24x24(30, 120, "蓝牙广播信息");
    Lcd_printf24x24(30, 150, "蓝牙自动设置");
    Lcd_printf24x24(30, 180, "版本查询");
    Lcd_printf24x24(5, 30 * MenuData.index[MenuData.current_id], "＞");
}

void main_menu_operation(uint8_t key)
{
    menu_select(key);
    switch (key)
    {
    case 3:                                                        // →
        MenuData.current_id = MenuData.index[MenuData.current_id]; // 切换到下级菜单
        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
            Menu_Tab[MenuData.current_id].display_operation();
        break;
    }
}

uint floatToString(float n, u8 a, char *str)
{
    uint length = 0;
    uint i, j;

    if (n == 0)
    {
        str[0] = '0';
        if (a > 0)
            str[1] = '.';
        memset(str + 2, '0', a);
        length = 2 + a;

        if (a == 0)
            length--;
    }
    else
    {
        int num;
        for (i = 0; i < a; i++)
            n *= 10;
        num = (uint)n;

        if (num < 0)
            num *= -1;

        i = 0;
        while (num > 0)
        {
            str[i] = num % 10 + 48;
            num /= 10;
            i++;
            length++;
        }
        if (n < 0)
        {
            str[i] = '-';
            length++;
        }
        if(n<10)
        {
            str[i] = '0';
            length++;
        }
     
        i = 0;
        j = length - 1;
        while (i < j)
        {
            char c = str[i];
            str[i] = str[j];
            str[j] = c;
            i++;
            j--;
        }

        if (a > 0)
        {
            for (i = 0, j = length; i < a; i++, j--)
                str[j] = str[j - 1];
            str[length - a] = '.';
            length++;
        }
    }

    return length;
}

void charge_set_display()
{
    char str[7] = "";
    floatToString(roter_data.Trickle_Current,1,str);
    Lcd_printf24x24(120 - 24 * 3, 0, "充电参数设置");
    if (roter_data.Bat_Volatage)
        Lcd_printf24x24(30, 30, "电池规格3.7v");
    else
        Lcd_printf24x24(30, 30, "电池规格3.2v");
    Lcd_printf24x24(30,60,"充电电流%dA ",roter_data.Charge_Current_Max);
    Lcd_printf24x24(30,90,"最大功率%dw ",roter_data.Charge_Power_Max);
    Lcd_printf24x24(30,120,"涓流电流%sA ",str);
    Lcd_printf24x24(5, 30 * MenuData.index[MenuData.current_id], "＞");
}

void charge_set_menu()
{
    Lcd_Clear(BLACK);
    charge_set_display();
}

void charge_menu_operation(uint8_t key)
{
    static uint8_t fsm;
    menu_select(key);
    switch (key)
    {
        case 3: // →
                switch (MenuData.index[MenuData.current_id])
                {
                    case 1:
                        if (roter_data.Bat_Volatage)
                        {
                            roter_data.Bat_Volatage = 0;
                        }
                        else
                        {
                            roter_data.Bat_Volatage = 1;
                        }
                        break;
                    case 2:
                        if(++roter_data.Charge_Current_Max > 20) roter_data.Charge_Current_Max = 2;
                        break;
                    case 3:
                        if(++roter_data.Charge_Power_Max > 75) roter_data.Charge_Power_Max = 10;
                        break;
                    case 4:
                        roter_data.Trickle_Current += 0.1;
                        if(roter_data.Trickle_Current>2.5)roter_data.Trickle_Current = 0.5;
                        break;
                }
                charge_set_display();
        break;
        case 4:
                switch (MenuData.index[MenuData.current_id])
                {
                    case 1:
                        if (roter_data.Bat_Volatage)
                        {
                            roter_data.Bat_Volatage = 0;
                        }
                        else
                        {
                            roter_data.Bat_Volatage = 1;
                        }
                        break;
                    case 2:
                        if(--roter_data.Charge_Current_Max < 2) roter_data.Charge_Current_Max = 20;
                        break;
                    case 3:
                        if(++roter_data.Charge_Power_Max > 10) roter_data.Charge_Power_Max = 75;
                        break;
                    case 4:
                        roter_data.Trickle_Current -= 0.1;
                        if(roter_data.Trickle_Current<0.5)roter_data.Trickle_Current = 2.5;
                        break;
                }
                charge_set_display();
            break;
    }

    switch (fsm)
    {
    case 1:
        switch (key) // 数字输入
        {
        case 0:
            break;

        default:
            break;
        }
        break;
    default:
        break;
    }
}

void dischar_display()
{   
    char str[7] = "";
    floatToString(roter_data.Low_voltage_Protect,2,str);
    Lcd_printf24x24(120 - 24 * 3, 0, "放电参数设置");
    Lcd_printf24x24(30, 30, "低压保护:%sv",str);
    Lcd_printf24x24(30, 60, "电流挡位:%d ",roter_data.current_gear);
    Lcd_printf24x24(30, 90, "雷达感应:%d%% ",roter_data.Ledar_Pwm);
    Lcd_printf24x24(30, 120, "雷达时间:%ds ",roter_data.Ledar_Dly_Time);
    Lcd_printf24x24(30, 150, "亮度设置:%d%% ",roter_data.Led_Set_Pwm);
    Lcd_printf24x24(30, 180, "放电曲线设置");
    Lcd_printf24x24(5, 30 * MenuData.index[MenuData.current_id], "＞");
}

void dischar_set_menu()
{
    Lcd_Clear(BLACK);
    dischar_display();
}

void dischar_set_operation(uint8_t key)
{
     menu_select(key);
    switch (key)
    {
        case 3: // →
                switch (MenuData.index[MenuData.current_id])
                {
                    case 1:
                        roter_data.Low_voltage_Protect += 0.0501;
                        if(roter_data.Low_voltage_Protect>2.85)roter_data.Low_voltage_Protect = 2.5;
                        break;
                    case 2:
                        if(++roter_data.current_gear > 20) roter_data.current_gear = 1;
                        break;
                    case 3:
                        if(++roter_data.Ledar_Pwm > 30) roter_data.Ledar_Pwm = 10;
                        break;
                    case 4:
                        if(++roter_data.Ledar_Dly_Time>15)roter_data.Trickle_Current = 5;
                        break;      
                     case 5:
                        if(++roter_data.Led_Set_Pwm>100)roter_data.Trickle_Current = 10;
                        break;
                     case 6:
                        MenuData.current_id = DISCHAR_CURVE_SET_MENU; // 切换到下级菜单
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                            Menu_Tab[MenuData.current_id].display_operation();
                        return ;
                        break;
                }
                dischar_display();
        break;
        case 4:
                switch (MenuData.index[MenuData.current_id])
                {
                      case 1:
                        roter_data.Low_voltage_Protect -= 0.0501;
                        if(roter_data.Low_voltage_Protect<2.5)roter_data.Low_voltage_Protect = 2.5;
                        break;
                    case 2:
                        if(--roter_data.current_gear <= 0) roter_data.current_gear = 20;
                        break;
                    case 3:
                        if(--roter_data.Ledar_Pwm < 10) roter_data.Ledar_Pwm = 30;
                        break;
                    case 4:
                        if(--roter_data.Ledar_Dly_Time<5)roter_data.Ledar_Dly_Time = 15;
                        break;      
                     case 5:
                        if(--roter_data.Led_Set_Pwm<=0)roter_data.Led_Set_Pwm = 100;
                        break;
                }
                dischar_display();
            break;
    }

}

void dischar_curve_set_menu()
{   
    Lcd_Clear(BLACK);
    Lcd_printf24x24(120 - 24 * 3, 0, "放电曲线设置");
    Lcd_printf24x24(5, 30 * MenuData.index[MenuData.current_id], "＞");
}

void IRorBL_set_menu()
{
    Lcd_Clear(BLACK);
    Lcd_printf24x24(120 - 24 * 3, 0, "蓝牙红外设置");
    if (roter_data.Ble_Onflag)
        Lcd_printf24x24(30, 30, "蓝牙通信：开");
    else
        Lcd_printf24x24(30, 30, "蓝牙通信：关");
    if (roter_data.Ir_Onflag)
        Lcd_printf24x24(30, 60, "红外通信：开");
    else
        Lcd_printf24x24(30, 60, "红外通信：关");
    Lcd_printf24x24(5, 30 * MenuData.index[MenuData.current_id], "＞");
}

void IRorBL_set_operation(uint8_t key)
{
    menu_select(key);
    switch (key)
    {
    case 3: // →
        switch (MenuData.index[MenuData.current_id])
        {
        case 1:
            if (roter_data.Ble_Onflag)
                roter_data.Ble_Onflag = 0;
            else
                roter_data.Ble_Onflag = 1;
            if (roter_data.Ble_Onflag)
                Lcd_printf24x24(30, 30, "蓝牙通信：开");
            else
                Lcd_printf24x24(30, 30, "蓝牙通信：关");
            break;
        case 2:
            if (roter_data.Ir_Onflag)
                roter_data.Ir_Onflag = 0;
            else
                roter_data.Ir_Onflag = 1;
            if (roter_data.Ir_Onflag)
                Lcd_printf24x24(30, 60, "红外通信：开");
            else
                Lcd_printf24x24(30, 60, "红外通信：关");
            break;
        }
        break;
    }
}

static u16 bl_con_menu_timer;
void bl_con_info_display()
{
    uint8_t i;
    char str[30];
    for (i = 0; i < 13; i++)
    {
        sprintf(str, "%02x %02x %02x %02x %02x %02x rssi:%d",
                roter_data.bl_adv_rp[i].mac[0],
                roter_data.bl_adv_rp[i].mac[1],
                roter_data.bl_adv_rp[i].mac[2],
                roter_data.bl_adv_rp[i].mac[3],
                roter_data.bl_adv_rp[i].mac[4],
                roter_data.bl_adv_rp[i].mac[5],
                roter_data.bl_adv_rp[i].rssi);
        Lcd_printf16x16(16, 16 + 16 * (i + 1), str);
    }
}
void bl_con_set_menu()
{
    Lcd_Clear(BLACK);
    Lcd_printf24x24(120 - 24 * 3, 0, "蓝牙广播信息");
    Lcd_printf16x16(5, 16 + 16 * MenuData.index[MenuData.current_id], ">");
    bl_con_info_display();
    bl_con_menu_timer = sys_timer_add(NULL, bl_con_info_display, 1000);
}
void bl_con_set_operation(uint8_t key)
{
    switch (key)
    {
    case 0: // 上
        if (MenuData.index[MenuData.current_id] > 1)
            MenuData.index[MenuData.current_id]--;
        else
            MenuData.index[MenuData.current_id] = Menu_Tab[MenuData.current_id].max_menu_num;
        BL_menu_select_display();
        break;
    case 1: // 下
        if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
            MenuData.index[MenuData.current_id]++;
        else
            MenuData.index[MenuData.current_id] = 1;
        BL_menu_select_display();
        break;
    case 2: // ←
        sys_timer_del(bl_con_menu_timer);
        MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
        Menu_Tab[MenuData.current_id].display_operation();
        break;
    case 3: // →
        /*  MenuData.current_id = MenuData.index[MenuData.current_id]; //切换到下级菜单
          if( Menu_Tab[MenuData.current_id].current_operation != NULL)Menu_Tab[MenuData.current_id].display_operation();*/
        break;
    }
}

void bl_ATcon_set_menu()
{
    Lcd_Clear(BLACK);
    Lcd_printf24x24(120 - 24 * 3, 0, "蓝牙自动连接");
    Lcd_printf24x24(30, 30, "自动设置：关");
    Lcd_printf24x24(30, 60, "清空设置记录");
    Lcd_printf24x24(30, 90, "在线数：%d",roter_data.bl_cnt);
    Lcd_printf24x24(30, 120, "已设置：");
    Lcd_printf24x24(5, 30 * MenuData.index[MenuData.current_id], "＞");
}

void Version_Check_menu()
{
    Lcd_Clear(BLACK);
    Lcd_printf24x24(120 - 24 * 2, 0, "版本查询");
    Lcd_printf24x24(120 - 16 * 5, 30, "2023:11:10");
}

// 测试蓝牙重复位置
uint8_t BL_Find_Mac_RepAddr(Bl_Adv_Rp_t *adv, uint8_t len, uint8_t *mac)
{
    uint32_t i, j;
    uint8_t isok = 1;
    for (i = 0; i < len; i++)
    {
        isok = 1;
        for (j = 0; j < 6; j++)
        {
            if (adv[i].mac[j] != mac[j])
            {
                isok = 0;
                break;
            }
        }
        if (isok)
            return i;
    }
    return 255;
}

// 查找空地址
uint8_t BL_Check_NonAddr(Bl_Adv_Rp_t *adv, uint8_t len)
{
    uint32_t i, j;
    for (i = 0; i < len; i++)
    {
        if (adv[i].useflag == 0)
            return i;
    }
    return 255;
}

uint8_t BL_Timeout_Check(Bl_Adv_Rp_t *adv, uint8_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++) //
    {
        if (++adv[i].Timeout > 5)
        {
            adv[i].useflag = 0; // 清除使用标志
        }
    }
}
