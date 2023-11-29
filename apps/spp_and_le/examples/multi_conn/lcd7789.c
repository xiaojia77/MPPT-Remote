#include "lcd7789.h"
#include "lcd_data.h"
#include "asm/spi.h"

#include "key_driver.h"

#include "stdarg.h"
#include "stdlib.h"

#define log_info(x, ...) printf("[INFO]" x " ", ##__VA_ARGS__)

MenuData_t MenuData = {0}; // 5个菜单

roter_t RoterData = {0};

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
    y_start += 80;
    y_end += 80;
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

void Lcd_Show20x20(uint8_t x, uint8_t y, uint8_t *p)
{
    uint8_t i, j, k;
    uint8_t temp; // 缓存
    Lcd_Address_Set(x, y, x + 19, y + 19);
    for (i = 1; i <= 60; i++)
    {
        temp = *p++;
        if( (i%3) == 0 )
        {
            for (k = 0; k < 4; k++)
            {
                if (temp & 0x80)
                    Lcd_WriteRgbData(BLUE);
                else
                    Lcd_WriteRgbData(BLACK);
                temp <<= 1;
            }
        }
        else
        {
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
}

void Lcd_Show16x24(uint8_t x, uint8_t y, uint8_t *p)
{
    uint8_t i, j, k;
    uint8_t temp; // 缓存
  
    Lcd_Address_Set(x, y, x + 9, y + 19);
    for (i = 1; i <= 40; i++)
    {
        temp = *p++;
        if( (i%2) == 0 )
        {
            for (k = 0; k < 2; k++)
            {
                if (temp & 0x80)
                    Lcd_WriteRgbData(BLUE);
                else
                    Lcd_WriteRgbData(BLACK);
                temp <<= 1;
            }
        }
        else
        {
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

void Lcd_printf20x20(uint8_t x, uint8_t y, uint8_t *format, ...) // must be string
{
    uint8_t SearchData[4];
    char *str_data = malloc(128);
    va_list ap;
    va_start(ap, format);
    vsnprintf(str_data, 128, format, ap);
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
            if (!w)break;
            Lcd_Show20x20(x, y, DotTbl24[ (w - DotTbl24String)/3 ]);
            x += 20;
            str += 3;
        }
        else
        {
            SearchData[0] = str[0];
            SearchData[1] = 0;
            w = strstr(DotTbl24AsciiString, SearchData);
            if (!w)break;
            Lcd_Show16x24(x, y, DotTbl24Ascii[w - DotTbl24AsciiString]);
            x += 10;
            str++;
        }
    }
    free(str_data);
}
 
void Lcd_printf16x16(uint8_t x, uint8_t y, uint8_t *str) // must be string
{
    uint8_t SearchData[4] = {0, 0, 0};
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
            Lcd_Show16x24(x, y, DotTbl24[w - DotTbl24String]);
            x += 24;
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

void Lcd_Clear20x20(uint8_t x, uint8_t y)
{
    uint8_t i, j, k;
    uint8_t temp; // 缓存
    Lcd_Address_Set(x, y, x + 19, y + 19);
    for (i = 0; i < 60; i++)
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
    Lcd_WriteData(0xC0);

    Lcd_WriteCmd(0x3A);  // Interface pixel format
    Lcd_WriteData(0x55); // 65K
    Lcd_WriteCmd(0xe7);  // SPI2 enable    启用2数据通道模式
    Lcd_WriteData(0x00);

   // Lcd_WriteCmd(0x21); // Display inversion on
    Lcd_Clear(BLACK);

    Lcd_WriteCmd(0x29); // Display o
}

uint32_t floatToString(float n, u8 a, char *str)
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
    str[length] = 0;

    return length;
}

void Lcd_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
{
	u16 i,j;
	u32 k=0;
	Lcd_Address_Set(x,y,x+length-1,y+width-1);
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
			Lcd_WriteData(gImage_image[k*2+1]);
            Lcd_WriteData(gImage_image[k*2]);
			k++;
		}
	}			
}


static const uint8_t Num_Tab[] = 
{
    KEY_VALUE_TYPE_0,KEY_VALUE_TYPE_1,KEY_VALUE_TYPE_2,
    KEY_VALUE_TYPE_3,KEY_VALUE_TYPE_4,KEY_VALUE_TYPE_5,
    KEY_VALUE_TYPE_6,KEY_VALUE_TYPE_7,KEY_VALUE_TYPE_8,
    KEY_VALUE_TYPE_9,
}; // 给按键做映射表

void Mppt_Menu_Select_Display(void)
{
    uint8_t i;
    for (i = 1; i <= 7; i++)
        Lcd_Clear20x20(5, 30 * i);

    // 限制下标长度
    if (!MenuData.index[MenuData.current_id])
        MenuData.index[MenuData.current_id] = 1;
    if (MenuData.index[MenuData.current_id] > 7)
        MenuData.index[MenuData.current_id] = 7;
    Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
}
void Mppt_Normal_Menu_Select(uint8_t key)
{
    switch (key)
    {
        case KEY_VALUE_TYPE_UP: // 上
            if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
            if (MenuData.index[MenuData.current_id] > 1)
                MenuData.index[MenuData.current_id]--;
            else
                MenuData.index[MenuData.current_id] = Menu_Tab[MenuData.current_id].max_menu_num;
            Mppt_Menu_Select_Display();
            break;
        case KEY_VALUE_TYPE_DOWN: // 下
            if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
            if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                MenuData.index[MenuData.current_id]++;
            else
                MenuData.index[MenuData.current_id] = 1;
            Mppt_Menu_Select_Display();
            break;
        case KEY_VALUE_TYPE_LEFT: // ←
            MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
            if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                Menu_Tab[MenuData.current_id].display_operation();
            break;
    }
}
void Mppt_Main_Menu(void) // 主菜单
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 20 * 5, 0, "ＭＰＰＴ太阳能控制器");
    Lcd_printf20x20(30, 30, "蓝牙批量设置");
    Lcd_printf20x20(30, 60, "蓝牙指定连接");
    Lcd_printf20x20(30, 90, "红外参数设置");
    Lcd_printf20x20(30, 120, "系统信息查询");
    Lcd_printf20x20(30, 150, "ＭＰＰＴ版本设置"); 
    Lcd_printf20x20(30, 180, "当前版本:%s","SQ20P75SA-B"); 
    Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
}
void Mppt_Main_Menu_Operation(uint8_t key)
{
    Mppt_Normal_Menu_Select(key);
    switch (key)
    {
        case KEY_VALUE_TYPE_ENTRE:
        case KEY_VALUE_TYPE_RIGHT:      
            switch (MenuData.index[MenuData.current_id])
            {
                case 1:
                    MenuData.current_id = BLE_BATCHSET_MENU; // 切换到下级菜单
                    break;
                case 2:
                     MenuData.current_id = BL_CON_SELECT_MENU; // 切换到下级菜单
                    break;
                case 3:
                     MenuData.current_id = IR_SET_MENU; // 切换到下级菜单
                    break;
                case 4:
                    MenuData.current_id = VERSION_CHECK_MENU; // 切换到下级菜单
                    break;
                case 5:
                    MenuData.current_id = MPPT_VERSION_SELECT_MENU; // 切换到下级菜单
                    break;
            }           
            if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                 Menu_Tab[MenuData.current_id].display_operation();                                
            break;
    }
}

void Mppt_Ble_BatchSet_Display()
{
    const char *Curve_Mode_Str[]={"PWM ","电流","AI  "};
    Lcd_printf20x20(120 - 20 * 3, 0, "蓝牙批量设置");
    Lcd_printf20x20(30, 30, "充电参数设置");
    Lcd_printf20x20(30, 60, "放电参数设置");
    Lcd_printf20x20(30, 90, "放电曲线模式：%s",Curve_Mode_Str[RoterData.Mppt_SetPara.DischarCurve_Moed]);
    Lcd_printf20x20(30, 120,"放电曲线设置");
    Lcd_printf20x20(30, 150,"批量设置");
    Lcd_printf20x20(5, 30 * MenuData.index[BLE_BATCHSET_MENU], "＞");
}
void Mppt_Ble_BatchSet_Menu(void)
{
    Lcd_Clear(BLACK);
    Mppt_Ble_BatchSet_Display();
}
void Mppt_Ble_BatchSet_Menu_Operation(uint8_t key)
{
    Mppt_Normal_Menu_Select(key);
    switch (key)
    {
        case KEY_VALUE_TYPE_ENTRE:
        case KEY_VALUE_TYPE_RIGHT: // 
                switch (MenuData.index[MenuData.current_id])
                {
                    case 1:
                        MenuData.current_id = CHAEGE_SET_MENU; // 切换到下级菜单
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                                Menu_Tab[MenuData.current_id].display_operation();
                        return ;
                         break;
                    case 2:
                        MenuData.current_id = DISCHAR_SET_MENU; // 切换到下级菜单
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                             Menu_Tab[MenuData.current_id].display_operation();
                        return ;
                        break;
                    case 3:
                        if(++RoterData.Mppt_SetPara.DischarCurve_Moed>2)RoterData.Mppt_SetPara.DischarCurve_Moed = 0;
                        Mppt_Ble_BatchSet_Display();
                        break;
                    case 4:
                        MenuData.current_id = DISCHAR_CURVE_SET_MENU; // 切换到下级菜单
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                             Menu_Tab[MenuData.current_id].display_operation();
                         return ;
                    break;
                    case 5:
                        MenuData.current_id = BL_ATCON_SET_MENU; // 切换到下级菜单
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                             Menu_Tab[MenuData.current_id].display_operation();
                         return ;
                    break;
                }
               // Mppt_DischarPara_Display();
            break;
        case KEY_VALUE_TYPE_DECRE:    
               // Mppt_DischarPara_Display();
            break;
    }   
}

    void Mppt_ChargePara_Display(Mppt_Set_Parm_t *SetPara)
    {
        char str[10] = ""; 
        Lcd_printf20x20(120 - 24 * 3, 0, "充电参数设置");
        floatToString(SetPara->Bat_Capcity,1,str);
        Lcd_printf20x20(30, 30,"电池容量%sAh ",str);
        Lcd_printf20x20(30,60,"充电电流%dA ",SetPara->Charge_Current_Max);
        Lcd_printf20x20(30,90,"最大功率%dw ",SetPara->Charge_Power_Max);
        floatToString(SetPara->Trickle_Current,1,str);
        Lcd_printf20x20(30,120,"涓流电流%sA ",str);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
    }
    void Mppt_Charge_Set_Menu(void)
    {
        Lcd_Clear(BLACK);
        Mppt_ChargePara_Display(&RoterData.Mppt_SetPara);
    }
    void Mppt_Charge_Set_Menu_Operation(uint8_t key)
    {
        static uint8_t fsm;
        Mppt_Normal_Menu_Select(key);
        switch (key)
        {
            case KEY_VALUE_TYPE_INCRE: // →
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            RoterData.Mppt_SetPara.Bat_Capcity += 0.5;
                            if (RoterData.Mppt_SetPara.Bat_Capcity>20)RoterData.Mppt_SetPara.Bat_Capcity = 5;
                            break;
                        case 2:
                            if(++RoterData.Mppt_SetPara.Charge_Current_Max > 20) RoterData.Mppt_SetPara.Charge_Current_Max = 2;
                            break;
                        case 3:
                            if(++RoterData.Mppt_SetPara.Charge_Power_Max > 75) RoterData.Mppt_SetPara.Charge_Power_Max = 10;
                            break;
                        case 4:
                            RoterData.Mppt_SetPara.Trickle_Current += 0.1;
                            if(RoterData.Mppt_SetPara.Trickle_Current>2.5)RoterData.Mppt_SetPara.Trickle_Current = 0.5;
                            break;
                    }
                    Mppt_ChargePara_Display(&RoterData.Mppt_SetPara);
                   break;
            case KEY_VALUE_TYPE_DECRE:
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            RoterData.Mppt_SetPara.Bat_Capcity -= 0.5;
                            if (RoterData.Mppt_SetPara.Bat_Capcity<7.5)RoterData.Mppt_SetPara.Bat_Capcity = 20;
                            break;
                        case 2:
                            if(--RoterData.Mppt_SetPara.Charge_Current_Max < 2) RoterData.Mppt_SetPara.Charge_Current_Max = 20;
                            break;
                        case 3:
                            if(--RoterData.Mppt_SetPara.Charge_Power_Max < 10) RoterData.Mppt_SetPara.Charge_Power_Max = 75;
                            break;
                        case 4:
                            RoterData.Mppt_SetPara.Trickle_Current -= 0.1;
                            if(RoterData.Mppt_SetPara.Trickle_Current<0.5)RoterData.Mppt_SetPara.Trickle_Current = 2.5;
                            break;
                    }
                    Mppt_ChargePara_Display(&RoterData.Mppt_SetPara);
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
  
    void Mppt_DischarPara_Display(Mppt_Set_Parm_t *SetPara)
    {   
        char str[7] = "";
        floatToString(SetPara->Low_voltage_Protect,2,str);
        Lcd_printf20x20(120 - 24 * 3, 0, "放电参数设置");
        Lcd_printf20x20(30, 30, "低压保护:%sv",str);
        Lcd_printf20x20(30, 60, "电流挡位:%d ",SetPara->Current_Gear);
        Lcd_printf20x20(30, 90, "雷达感应:%d%% ",SetPara->Ledar_Pwm);
        Lcd_printf20x20(30, 120, "雷达时间:%ds ",SetPara->Ledar_Dly_Time);
        Lcd_printf20x20(30, 150, "亮度设置:%d%% ",SetPara->Led_Set_Pwm);
        Lcd_printf20x20(30, 180, "光控模式:%d%% ",SetPara->Led_Set_Pwm);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
    }
    void Mppt_Dischar_Set_Menu(void)
    {
        Lcd_Clear(BLACK);
        Mppt_DischarPara_Display(&RoterData.Mppt_SetPara);
    }
    void Mppt_Dischar_Set_Menu_Operation(uint8_t key)
    {
        Mppt_Normal_Menu_Select(key);
        switch (key)
        {
            case KEY_VALUE_TYPE_INCRE: // 
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            RoterData.Mppt_SetPara.Low_voltage_Protect += 0.0501;
                            if(RoterData.Mppt_SetPara.Low_voltage_Protect>2.85)RoterData.Mppt_SetPara.Low_voltage_Protect = 2.5;
                            break;
                        case 2:
                            if(++RoterData.Mppt_SetPara.Current_Gear > 20) RoterData.Mppt_SetPara.Current_Gear = 1;
                            break;
                        case 3:
                            if(++RoterData.Mppt_SetPara.Ledar_Pwm > 30) RoterData.Mppt_SetPara.Ledar_Pwm = 10;
                            break;
                        case 4:
                            if(++RoterData.Mppt_SetPara.Ledar_Dly_Time>15)RoterData.Mppt_SetPara.Trickle_Current = 5;
                            break;      
                        case 5:
                            if(++RoterData.Mppt_SetPara.Led_Set_Pwm>100)RoterData.Mppt_SetPara.Trickle_Current = 10;
                            break;
                        case 6:
                            MenuData.current_id = DISCHAR_CURVE_SET_MENU; // 切换到下级菜单
                            if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                                Menu_Tab[MenuData.current_id].display_operation();
                            return ;
                            break;
                    }
                    Mppt_DischarPara_Display(&RoterData.Mppt_SetPara);
                break;
            case KEY_VALUE_TYPE_DECRE:
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            RoterData.Mppt_SetPara.Low_voltage_Protect -= 0.0501;
                            if(RoterData.Mppt_SetPara.Low_voltage_Protect<2.5)RoterData.Mppt_SetPara.Low_voltage_Protect = 2.5;
                            break;
                        case 2:
                            if(--RoterData.Mppt_SetPara.Current_Gear <= 0) RoterData.Mppt_SetPara.Current_Gear = 20;
                            break;
                        case 3:
                            if(--RoterData.Mppt_SetPara.Ledar_Pwm < 10) RoterData.Mppt_SetPara.Ledar_Pwm = 30;
                            break;
                        case 4:
                            if(--RoterData.Mppt_SetPara.Ledar_Dly_Time<5)RoterData.Mppt_SetPara.Ledar_Dly_Time = 15;
                            break;      
                        case 5:
                            if(--RoterData.Mppt_SetPara.Led_Set_Pwm<=0)RoterData.Mppt_SetPara.Led_Set_Pwm = 100;
                            break;
                    }
                    Mppt_DischarPara_Display(&RoterData.Mppt_SetPara);
                break;
        }   

    }

    void Mppt_Curve_Menu_Select_Display(void)
    {
        uint8_t i;
        for (i = 1; i <= 8; i++)
        {
              Lcd_Clear20x20(0, 20 * i + 10);
              Lcd_Clear20x20(24 + 10*10, 20 * i + 10);
        }
        //限制下标长度
        if (!MenuData.index[MenuData.current_id])MenuData.index[MenuData.current_id] = 1;        
        if (MenuData.index[MenuData.current_id] >= 16)MenuData.index[MenuData.current_id] = 16;
        
        if( MenuData.index[MenuData.current_id] / 9 )
            Lcd_printf20x20(24 + 10*10, 20 * (MenuData.index[MenuData.current_id]-8) + 10, "＞");
        else
            Lcd_printf20x20(0, 20 * MenuData.index[MenuData.current_id] + 10, "＞");
    }
    void Mppt_DischarCurve_Display(Mppt_Set_Parm_t *Para)
    {
        uint8_t i;
        uint8_t str[10];
        float (*CurvData)[2] = Para->Curv_Data;
        const char *Dischar_Curve_Str[]={"PWM","Cur","AI"};
        char *Curvestr = Dischar_Curve_Str[RoterData.Mppt_SetPara.DischarCurve_Moed];
        if(RoterData.Mppt_SetPara.DischarCurve_Moed < 2)
        {
            Lcd_printf20x20(120 - 20 * 3, 0, "放电曲线设置");
            for(i=0;i<8;i++)
            {
                floatToString(CurvData[i][0],1,str);
                Lcd_printf20x20(24,20*(i+1)+ 10,"曲%d %sH ",i+1,str,Curvestr,str);
            }
            CurvData[3][1] = 50;
            for(i=0;i<8;i++)
            {
            //    floatToString(CurvData[i][1],0,str);
                Lcd_printf20x20(24 + 12*10, 20*(i+1)+ 10,"%s:%d%% ",Curvestr,(uint32_t)CurvData[i][1]);
            }
            // Lcd_printf20x20(24, 20 + 10,  "曲1 1.1H   %s:90%%",str);
        }
        else
        {
            Lcd_printf20x20(120 - 20 * 3, 0, "放电曲线设置");
            Lcd_printf20x20(120 - 10 * 3, 30,"Ai模式");
        }
       
        //Lcd_printf20x20(0, 30 * MenuData.index[MenuData.current_id], "＞");
        if( MenuData.index[MenuData.current_id] / 9 )
            Lcd_printf20x20(24 + 10* 10, 20 * (MenuData.index[MenuData.current_id]-8) + 10, "＞");
        else
            Lcd_printf20x20(0, 20 * MenuData.index[MenuData.current_id] + 10, "＞");
    }
    void Mppt_Dischar_Curve_Set_Menu(void)
    {   
        Lcd_Clear(BLACK);
        Mppt_DischarCurve_Display(&RoterData.Mppt_SetPara); 
    }
    void Mppt_Dischar_Curve_Set_Operation(uint8_t key)
    {   
        float (*CurvData)[2] = RoterData.Mppt_SetPara.Curv_Data;
        uint8_t Menu_Id = MenuData.current_id;
        switch (key)
        {
            case KEY_VALUE_TYPE_UP: // 上
                if(RoterData.Mppt_SetPara.DischarCurve_Moed > 1)return;
                if(!Menu_Tab[Menu_Id].max_menu_num)return;
                if (MenuData.index[Menu_Id] > 1)MenuData.index[Menu_Id]--;    
                else MenuData.index[Menu_Id] = Menu_Tab[Menu_Id].max_menu_num;                
                Mppt_Curve_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_DOWN: // 下
                if(RoterData.Mppt_SetPara.DischarCurve_Moed > 1)return;
                if(!Menu_Tab[Menu_Id].max_menu_num)return;
                if (MenuData.index[Menu_Id] < Menu_Tab[Menu_Id].max_menu_num)MenuData.index[Menu_Id]++;
                else MenuData.index[Menu_Id] = 1;
                Mppt_Curve_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_LEFT: // ←    
                if( ( MenuData.index[Menu_Id] / 9 ) && RoterData.Mppt_SetPara.DischarCurve_Moed <= 1)
                {
                    MenuData.index[Menu_Id] -= 8;
                    Mppt_Curve_Menu_Select_Display();
                }
                else
                {
                    MenuData.current_id = Menu_Tab[Menu_Id].up_id;
                    if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                        Menu_Tab[MenuData.current_id].display_operation();
                }
                break;
            case KEY_VALUE_TYPE_RIGHT:
                if(RoterData.Mppt_SetPara.DischarCurve_Moed > 1)return;
                if( ( MenuData.index[Menu_Id] / 9 ) == 0)
                {
                    MenuData.index[Menu_Id] += 8; 
                    Mppt_Curve_Menu_Select_Display();
                }
                break;
            case KEY_VALUE_TYPE_INCRE:
                 if( ( MenuData.index[Menu_Id] / 9 ) == 0)
                {
                    CurvData[MenuData.index[Menu_Id]-1][0] += 0.5;
                    if(CurvData[MenuData.index[Menu_Id]-1][0]>24)CurvData[MenuData.index[Menu_Id]-1][0] = 0.5;
                }
                else
                {
                    CurvData[(MenuData.index[Menu_Id]-1)%8][1] += 1;
                    if(CurvData[(MenuData.index[Menu_Id]-1)%8][1]>100)CurvData[(MenuData.index[Menu_Id]-1)%8][1] = 1;
                }
                Mppt_DischarCurve_Display(&RoterData.Mppt_SetPara);
                break;
            case KEY_VALUE_TYPE_DECRE:
                if( ( MenuData.index[Menu_Id] / 9 ) == 0)
                {
                    CurvData[MenuData.index[Menu_Id]-1][0] -= 0.5;
                    if(CurvData[MenuData.index[Menu_Id]-1][0]<0)CurvData[MenuData.index[Menu_Id]-1][0] = 24;
                }
                else
                {
                    CurvData[(MenuData.index[Menu_Id]-1)%8][1] -= 1;
                    if(CurvData[(MenuData.index[Menu_Id]-1)%8][1]<1)CurvData[(MenuData.index[Menu_Id]-1)%8][1] = 100;
                }
                Mppt_DischarCurve_Display(&RoterData.Mppt_SetPara);
                break;
        }
    }
    
    void Mppt_Ble_AutoConnect_set_Menu(void)
    {
        Lcd_Clear(BLACK);
        Lcd_printf20x20(120 - 24 * 3, 0, "蓝牙自动连接");
        Lcd_printf20x20(30, 30, "自动设置：关");
        Lcd_printf20x20(30, 60, "清空设置记录");
        Lcd_printf20x20(30, 90, "在线数：%d",RoterData.Ble_Adv_Rp_Count);
        Lcd_printf20x20(30, 120, "已设置：");
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
    }
    void Mppt_Ble_AutoConnect_set_Menu_Operation(uint8_t key)
    {
        Mppt_Normal_Menu_Select(key);
    }


static u16 bl_con_menu_timer;

void Mppt_BLe_Menu_Select_Display(void)
{
    uint8_t i;
    for (i = 1; i <= 13; i++)
        Lcd_printf16x16(5, 8 + 16 * i, " ");
    // 限制下标长度
    if (!MenuData.index[MenuData.current_id])
        MenuData.index[MenuData.current_id] = 1;
    if (MenuData.index[MenuData.current_id] > 13)
        MenuData.index[MenuData.current_id] = 13;
    Lcd_printf16x16(5, 8 + 16 * MenuData.index[MenuData.current_id], ">");
}
void Mppt_Ble_Mac_Info_Display(void)
{
    uint8_t i;
    char str[30];
    for (i = 0; i < 13; i++)
    {
        sprintf(str, "%02x %02x %02x %02x %02x %02x rssi:%d  ",
                RoterData.Ble_Adv_rp[i].mac[0],
                RoterData.Ble_Adv_rp[i].mac[1],
                RoterData.Ble_Adv_rp[i].mac[2],
                RoterData.Ble_Adv_rp[i].mac[3],
                RoterData.Ble_Adv_rp[i].mac[4],
                RoterData.Ble_Adv_rp[i].mac[5],
                RoterData.Ble_Adv_rp[i].rssi);
        //Lcd_printf16x16(16 + 184, 16 + 16 * (i + 1),"    ");    // 清空后面的显示 防止之前的残留
        Lcd_printf16x16(16, 8 + 16 * (i + 1), str);
    }
}

void Mppt_Ble_con_Select_Menu(void)
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 24 * 3, 0, "蓝牙指定连接");
    Lcd_printf16x16(5, 8 + 16 * MenuData.index[MenuData.current_id], ">");
    Mppt_Ble_Mac_Info_Display();
    bl_con_menu_timer = sys_timer_add(NULL, Mppt_Ble_Mac_Info_Display, 1000);
}
void Mppt_Ble_con_Select_Menu_Operation(uint8_t key)
{
    switch (key)
    {
        case KEY_VALUE_TYPE_UP: // 上
            if (MenuData.index[MenuData.current_id] > 1)
                MenuData.index[MenuData.current_id]--;
            else
                MenuData.index[MenuData.current_id] = Menu_Tab[MenuData.current_id].max_menu_num;
            Mppt_BLe_Menu_Select_Display();
            break;
        case KEY_VALUE_TYPE_DOWN: // 下
            if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                MenuData.index[MenuData.current_id]++;
            else
                MenuData.index[MenuData.current_id] = 1;
            Mppt_BLe_Menu_Select_Display();
            break;
        case KEY_VALUE_TYPE_LEFT: // ←
            sys_timer_del(bl_con_menu_timer);
            MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
            Menu_Tab[MenuData.current_id].display_operation();
            break;
        case KEY_VALUE_TYPE_RIGHT: // →
            sys_timer_del(bl_con_menu_timer);
            MenuData.current_id = BL_CON_MENU; //切换到下级菜单
            if( Menu_Tab[MenuData.current_id].current_operation != NULL)Menu_Tab[MenuData.current_id].display_operation();
            break;
    }
}

    void Mppt_Ble_con_Menu(void)
    {
        Lcd_Clear(BLACK);
        Lcd_printf20x20(120 - 24 * 3, 120 - 20, "蓝牙连接......");
    }
    void Mppt_Ble_con_Menu_Operation(uint8_t key)
    {
        switch (key)
        {
            case KEY_VALUE_TYPE_RIGHT: 
                MenuData.current_id = BL_CON_SET_MENU;
                Menu_Tab[MenuData.current_id].display_operation();
                break;
            
        }
    }

        void Mppt_Ble_Set_Menu(void)
            {
                Lcd_Clear(BLACK);
                Lcd_printf20x20(120 - 24 * 3, 0, "蓝牙指定连接");
                Lcd_printf20x20(30,30,"ＭＰＰＴ信息查询");
                Lcd_printf20x20(30,60,"充电参数修改");
                Lcd_printf20x20(30,90,"放电参数修改");
                Lcd_printf20x20(30,120,"放电曲线修改");
                Lcd_printf20x20(30,150,"确认修改");
                Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
            }     
        void Mppt_Ble_Set_Operation(uint8_t key)
        {
            Mppt_Normal_Menu_Select(key);
             switch (key)
            {
                case KEY_VALUE_TYPE_ENTRE:
                case KEY_VALUE_TYPE_RIGHT: // 
                        switch (MenuData.index[MenuData.current_id])
                        {
                            case 1:
                                MenuData.current_id = MPPT_INFO; // 切换到下级菜单
                                break;
                            case 2:
                                MenuData.current_id = CHAEGE_PARA_MODIFY; // 切换到下级菜单
                                break;
                            case 3:
                                MenuData.current_id = DISCHAR_PARA_MODIFY; // 切换到下级菜单
                                break;
                            case 4:
                                MenuData.current_id = CURVE_PARAT_MENU;
                                break;
                            case 5:
                                MenuData.current_id = ENTRY_MODIFY; // 切换到下级菜单
                              break;
                        }
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                                    Menu_Tab[MenuData.current_id].display_operation();
                    break;
            }   
        }
        
            void Mppt_Info_Display(Mppt_Info_Para_t *Info)
            {
                char *str[10];
                const char* OutPut_Status_Str[]={"正常，短路"};
                Lcd_printf20x20(120 - 24 * 3, 0, "ＭＰＰＴ信息");
                floatToString(Info->Bat_Capcity,1,str);
                Lcd_printf20x20(30, 25 * 1, "已充电量:%sAh",str);
                floatToString(Info->Charge_Current,1,str);
                Lcd_printf20x20(30, 25 * 2, "充电电流:%sA",str);
                floatToString(Info->Dischar_Current,1,str);
                Lcd_printf20x20(30, 25 * 3, "放电电流:%sdA",str);
                floatToString(Info->Charge_Power,1,str);
                Lcd_printf20x20(30, 25 * 4, "充电功率:%sW",str);
                Lcd_printf20x20(30, 25 * 5, "电池内阻:%dＭΩ",Info->Bat_Resistance);
                Lcd_printf20x20(30, 25 * 6, "电池电量:%d%%",Info->Bat_Capcity);
                floatToString(Info->Bat_Voltage,1,str);
                Lcd_printf20x20(30, 25 * 7, "电池电压:%sV",str);
                Lcd_printf20x20(30, 25 * 8, "输出状态:%s",OutPut_Status_Str[Info->OutPut_Staus]);
            }
            void Mppt_Info_Menu(void) //充电信息配置
            {
                Lcd_Clear(BLACK);
                Mppt_Info_Display(&RoterData.Mppt_Info);
            }

            void Mppt_ChargePara_Modify_Menu(void)
            {
                Lcd_Clear(BLACK);        
                Lcd_printf20x20(120 - 24 * 3, 0, "充电参数修改");
                Mppt_ChargePara_Display(&RoterData.Mppt_ConSetPara_Info);
            }
            void Mppt_ChargePara_Modify_Menu_Operation(uint8_t key)
            {
                Mppt_Normal_Menu_Select(key);
            }

            void Mppt_DischarPara_Modify_Menu(void)
            {
                Lcd_Clear(BLACK);
                Lcd_printf20x20(120 - 24 * 3, 0, "放电参数修改");
                Mppt_DischarPara_Display(&RoterData.Mppt_ConSetPara_Info);
            }
            void Mppt_DischarPara_Modify_Menu_Operation(uint8_t key)
            {
                Mppt_Normal_Menu_Select(key);
            }

            void Mppt_CurvePara_Modify_Menu(void)
            {
                Lcd_Clear(BLACK);
                Lcd_printf20x20(120 - 24 * 3, 0, "放电曲线修改");
                Mppt_DischarCurve_Display(&RoterData.Mppt_SetPara);
            }
            void Mppt_CurvePara_Modify_Menu_Operation(uint8_t key)
            {
                Mppt_Normal_Menu_Select(key);
            }

            void Mppt_Comfir_Modify_Menu(void)
            {
                Lcd_Clear(BLACK);
                Lcd_printf20x20(120 - 24 * 3, 120 - 20, "修改中......");     
            }
            void Mppt_Comfir_Modify_Menu_Operation(uint8_t key)
            {
                Mppt_Normal_Menu_Select(key);
            }

void Mppt_Ir_Set_Menu(void)
{
    Lcd_Clear(BLACK);   
    Lcd_printf20x20(120 - 24 * 3, 0, "红外参数设置");
    Lcd_printf20x20(30, 30, "常规遥控器");
    Lcd_printf20x20(30, 60, "工程遥控器");
    Lcd_printf20x20(30, 90, "用户码设置");
    Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
}
void Mppt_Ir_Set_Menu_Operation(uint8_t key)  
{
    Mppt_Normal_Menu_Select(key);
    switch (key)
    {
        case KEY_VALUE_TYPE_ENTRE:
        case KEY_VALUE_TYPE_RIGHT:      
            switch (MenuData.index[MenuData.current_id])
            {
                case 1:
                    MenuData.current_id = IR_NORMAL_MENU; // 切换到下级菜单
                    break;
                case 2:
                     MenuData.current_id = IR_ENGINEER_MENU; // 切换到下级菜单
                    break;
                case 3:
                     MenuData.current_id = IR_USERCODE_MENU; // 切换到下级菜单
                    break;
            }           
            if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                 Menu_Tab[MenuData.current_id].display_operation();                                
            break;
    }
}
   
    void IR_Normal_Menu(void)
    {
        Lcd_Clear(BLACK);   
        Lcd_printf20x20( 120 - 10 * 5 , 0, "常规遥控器");
        Lcd_printf20x20(30, 30, "1:0N   2:0FF");
        Lcd_printf20x20(30, 60, "2:FULL 3:HALF");
        Lcd_printf20x20(30, 90,"+:PWM+ -:PWM-");
        Lcd_printf20x20(30, 120,"4:Rader 5:Rader 4H ");
        Lcd_printf20x20(30, 150,"6:3H 7:5H 8:8h 9:Auto");
    }
    void IR_Normal_Menu_Operation(uint8_t key)
    {
        Mppt_Normal_Menu_Select(key);
    }

    void IR_Engineer_Menu(void)
    {
        Lcd_Clear(BLACK);   
        Lcd_printf20x20(120 - 10 * 5 , 0, "工程遥控器");
        Lcd_printf20x20(30, 60, "1:0N 2:0FF");
        Lcd_printf20x20(30, 90, "2:IR On 3:IR Off");
        Lcd_printf20x20(30, 120,"Rader 4: On 5: Off");
        Lcd_printf20x20(30, 150,"6:Current Gare:%d",20);
    }
    void IR_Engineer_Menu_Operation(uint8_t key)
    {
        Mppt_Normal_Menu_Select(key);
    }

    void IR_Usercode_Menu(void)
    {
        Lcd_Clear(BLACK);   
        Lcd_printf20x20( 120 - 12 * 5 , 0, "用户码设置");
    }
    void IR_Usercode_Menu_Operation(uint8_t key)
    {
        Mppt_Normal_Menu_Select(key);
    }


void Mppt_Version_Info_menu(void)
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 24 * 3, 0, "系统信息查询");
    Lcd_printf20x20(30, 60, "用户码:%06d",000000);
    Lcd_printf20x20(30, 90, "遥控器版本:1.0V");
    Lcd_printf20x20(30, 120, "出厂日期:2023:11:11");
}
void Mppt_Version_Select_Menu(void)
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 20 * 4, 0, "ＭＰＰＴ版本设置");
    Lcd_printf20x20(30, 30, "SQ20P75SA-B");
    Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
}


// 测试蓝牙重复位置
uint8_t Ble_Find_Mac_RepAddr(Ble_Adv_Rp_t *adv, uint8_t len, uint8_t *mac)
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
uint8_t Ble_Check_NonAddr(Ble_Adv_Rp_t *adv, uint8_t len)
{
    uint32_t i, j;
    for (i = 0; i < len; i++)
    {
        if (adv[i].useflag == 0)
            return i;
    }
    return 255;
}

void Ble_Timeout_Check(void)
{
    uint8_t i,j;
    for (i = 0; i < 13; i++) //
    {
        if(RoterData.Ble_Adv_rp[i].useflag)
        {
            if (++RoterData.Ble_Adv_rp[i].Timeout > 5)
            {
                RoterData.Ble_Adv_rp[i].Timeout = 0;
                RoterData.Ble_Adv_rp[i].useflag = 0; // 清除使用标志
                RoterData.Ble_Adv_rp[i].rssi = 0;
                for(j=0;j<6;j++)RoterData.Ble_Adv_rp[i].mac[j] = 0;
                RoterData.Ble_Adv_Rp_Count -- ;
            }
        }
        
    }
}
