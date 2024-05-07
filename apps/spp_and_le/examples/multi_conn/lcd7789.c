#include "lcd7789.h"
#include "lcd_data.h"
#include "asm/spi.h"

#include "key_driver.h"
#include "ble_multi.h"


#include "stdarg.h"
#include "stdlib.h"
#include "math.h"

#define log_info(x, ...) printf("[INFO]" x " ", ##__VA_ARGS__)

// 显示字符串用
static uint8_t InputMode = 0;
static uint8_t input_index = 0; 
static char instr[10];
static u16 InputFlash_Timer = 0;

static const uint8_t Num_Map_Tab[] = 
{
    KEY_VALUE_TYPE_0,KEY_VALUE_TYPE_1,KEY_VALUE_TYPE_2,
    KEY_VALUE_TYPE_3,KEY_VALUE_TYPE_4,KEY_VALUE_TYPE_5,
    KEY_VALUE_TYPE_6,KEY_VALUE_TYPE_7,KEY_VALUE_TYPE_8,
    KEY_VALUE_TYPE_9,
}; // 给按键做映射表

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

   
    //Lcd_WriteCmd(0x21); // Display inversion on
    Lcd_Clear(WHITE);
    Lcd_WriteCmd(0x29); // Display o
   
}

void floatToString(float d, int l,char *str) 
{

    int n = (int)d; //去掉小数点

    int b = 0;

    int i,j;
    float m = d;//

    u8 len;

    char *buf;

    buf = malloc(128);
    itoa(n,str,10);
    i=strlen(str);

    
    str[i] = '.'; //小数点
    str[i+1] = 0;
    len = i+1;
    while(d>0.99999f)
    {
        d /= 10.0f;
        b++;
    }

    for(i=0;i<l;i++)
    {
        m *= 10;//扩大
        if(!(int)m%10)
        {
            str[len] = '0';
            str[len+1] = 0;
            if(len<l   )len++;
        }
    }

    n = (int)m; //放弃其他小数点

     //log_info("floatToString%d B IS %d ", n , b);

    itoa(n,buf,10);//转换成字符串，更好操作

    //log_info("floatToString%s %d %d ", buf ,strlen(str) , strlen(buf));

    for(i=b,j=strlen(str);i<strlen(buf);i++,j++)
    {
        str[j] = buf[i];
    }

    //log_info("floatToString%s %d %d ", str ,i , j);

    str[j] = 0;

    //log_info("floatToString%s %d %d ", str ,strlen(str) , strlen(buf)); 
         
    free(buf);
} 

float string_to_float(char *string)
{
	unsigned int i=0,j=0;
	unsigned char flag=0;  			//判断正负号的标志
	unsigned char flag_dot=1; 	//判断小数点的标志
	float num=0,data =0;     					//临时存储计算结果的变量
	
	for(i=0;string[i];i++) //循环直到字符串结尾
	{
		if(string[i]>='0'&&string[i]<='9'&&flag_dot==1)  //如果当前字符为数字且在小数点之前
		{
			if(j==0) num = num*pow(10,j)+(double)(string[i]-'0');     //运算并存储中间计算结果
			else     num = num*pow(10,1)+(double)(string[i]-'0');
			j++;
		}
		else if(string[i]>='0'&&string[i]<='9'&&flag_dot==0) //如果当前字符为数字且在小数点之后
		{
			num = num+(double)(string[i]-'0')*pow(0.1,j+1);     //运算并存储中间计算结果
			j++;
		}
		else if(string[i]=='.')                               //读到了小数点则将对应标志位数值改变
		{
			flag_dot=0;
			j=0;
		}
		else if(string[i]=='-')                              //读到减号同样改变对应标志位的值
		{
			flag = 1;
		}
		else if(string[i]==',')                             //读完一个数据，重置标志位，记录最终计算结果
		{
			data = num*pow(-1,flag);
			flag = 0;
			flag_dot=1;
			j=0;
			num = 0;
		}
	}
	data = num*pow(-1,flag);                            //补上最后一个数
    return data;
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
			Lcd_WriteData(pic[k*2+1]);
            Lcd_WriteData(pic[k*2]);
			k++;
		}
	}			
}

int Get_Num_Map(uint8_t key)
{
    uint8_t i;
    for(i=0;i<sizeof(Num_Map_Tab);i++)
    {
        if(key == Num_Map_Tab[i])return i;
    }
    return -1;
}


void Mppt_Log_Menu(void)
{
    Lcd_ShowPicture(0,0,240,240,gImage_image);
}
void Mppt_Log_Menu_Operation(uint8_t key)
{
    
    switch (key)
    {
        case KEY_VALUE_TYPE_ENTRE:
        case KEY_VALUE_TYPE_RIGHT: // 
            MenuData.current_id = MAIN_MENU; // 切换到下级菜单
            if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                 Menu_Tab[MenuData.current_id].display_operation();        
            break;
    }   
    //Mppt_Normal_Menu_Select(key);
}


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
            {
                MenuData.index[MenuData.current_id] = Menu_Tab[MenuData.current_id].max_menu_num;
            }
                
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
    Lcd_printf20x20(30, 150, "系统参数查询");
    Lcd_printf20x20(30, 180, "ＭＰＰＴ版本设置"); 
    Lcd_printf20x20(30, 210, "当前版本:%s","SQ20P75SA-B"); 
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
                    MenuData.current_id = CHAEGE_SET_MENU; // 切换到下级菜单
                    break;
                case 2:
                     MenuData.current_id = BL_CON_MENU; // 切换到下级菜单
                    break;
                case 3:
                     MenuData.current_id = IR_SET_MENU; // 切换到下级菜单
                    break;
                case 4:
                    MenuData.current_id = VERSION_CHECK_MENU; // 切换到下级菜单
                    break;
                case 5:
                    MenuData.current_id = SYS_SET_MENU; // 切换到下级菜单
                    break;
                case 6:
                    MenuData.current_id = MPPT_VERSION_SELECT_MENU; // 切换到下级菜单
                    break;
            }           
            if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                 Menu_Tab[MenuData.current_id].display_operation();                                
            break;
    }
}

const char *Curve_Mode_Str[]={"PWM ","电流","AI  "};
const char *Connect_Mode_Str[]={"IO","串口"}; 
const char *AotoSetStr[]={"关","开"};
const char *Lock_Mode_Str[]={"关","开"}; 
const char *SolarModeStr[]={"开","关"};
const char *Dischar_Curve_Str[]={"PWM","Cur","AI"};

void Mppt_Ble_BatchSet_Display()
{
    Lcd_printf20x20(120 - 20 * 3, 0, "蓝牙批量设置");
    Lcd_printf20x20(30, 30, "充电参数设置");
    /*Lcd_printf20x20(30, 60, "放电参数设置");
    Lcd_printf20x20(30, 90, "放电曲线模式：%s",Curve_Mode_Str[RoterData.Mppt_SetPara.DischarCurve_Moed]);
    Lcd_printf20x20(30, 120,"放电曲线设置");
    Lcd_printf20x20(30, 150,"锁定模式：%s",Lock_Mode_Str[RoterData.Mppt_SetPara.Lock_Mode]);
    Lcd_printf20x20(30, 180,"批量设置");*/
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
                        RoterData.Mppt_SetPara.Lock_Mode = !RoterData.Mppt_SetPara.Lock_Mode;
                        Mppt_Ble_BatchSet_Display();
                        break;
                    case 6:
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


    void Mppt_Normal_InPutFlash(void)
    {
        static u8 flash = 1;
        flash = !flash;
        if(flash)Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
        else Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "  ");
    }
    void Mppt_ChargePara_Display(Mppt_Set_Parm_t *SetPara)
    {
        char str[10] = ""; 
        Lcd_printf20x20(120 - 24 * 3, 0, "充电参数设置");
        floatToString(SetPara->Bat_Capcity,1,str);
        Lcd_printf20x20(30, 30,"电池容量:%sAh  ",str);
        floatToString(SetPara->Charge_Current_Max,1,str);
        Lcd_printf20x20(30,60,"充电电流:%sA    ",str);
        floatToString(SetPara->Charge_Power_Max,1,str);
        Lcd_printf20x20(30,90,"最大功率:%sw    ",str);
        floatToString(SetPara->Trickle_Current,1,str);
        Lcd_printf20x20(30,120,"涓流电流:%sA   ",str);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");

    }
    void Mppt_Charge_Set_Menu(void)
    {
        Lcd_Clear(BLACK);
        MenuData.index[MenuData.current_id] = 1;
        Mppt_ChargePara_Display(&RoterData.Mppt_SetPara);
    }
    void Mppt_ChargeSet_Operation(Mppt_Set_Parm_t *SetPara,uint8_t key)
    {
        int num_key = Get_Num_Map(key);
        float indata;
        if(num_key != -1) // 有数字输入 第一个点不能是字符串输入
        {
            if(!InputMode)
            {
                InputMode = 1; //  输入模式
                InputFlash_Timer = sys_timer_add(NULL,Mppt_Normal_InPutFlash,300);
                input_index = 0;
                memset(instr,0,sizeof(instr));
            }  
        }
        if(InputMode)
        {
            if(num_key != -1)
            {
                if(input_index<5)input_index++;
                instr[input_index - 1] = num_key + '0';
                instr[input_index] = 0;
            }
            switch (key)
            {

                case KEY_VALUE_TYPE_DOT: // 输入小数点
                    if(input_index<5)input_index++;
                    instr[input_index - 1] = '.';
                    instr[input_index] = 0;
                    break;
                
                case KEY_VALUE_TYPE_BACKSPACE: // 退格
                    if(input_index)
                    {   
                        if(input_index == 1)instr[input_index - 1] = '0';
                        else instr[input_index - 1] = 0; 
                        input_index --;       
                    }
                    break;
                case KEY_VALUE_TYPE_ENTRE:
                    indata = string_to_float(instr);
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            if(indata<5)   indata = 5; 
                            if(indata>100) indata = 100;
                            SetPara->Bat_Capcity=indata;
                            break;
                        case 2:
                            if(indata<5) indata = 5;
                            if(indata>25) indata = 25;
                            SetPara->Charge_Current_Max=indata;
                            break;
                        case 3:
                            if(indata<10) indata = 10;
                            if(indata>100) indata = 100;
                            SetPara->Charge_Power_Max=indata;
                            break;
                        case 4:
                            if(indata<0.5) indata = 0.5;
                            if(indata>5) indata = 5;
                            SetPara->Trickle_Current=indata;
                            break;    
                    }    
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_ChargePara_Display(SetPara);
                    return;
                    break;
                case KEY_VALUE_TYPE_LEFT:
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_ChargePara_Display(SetPara);
                    return;
                    break;
            }

            float input_num = string_to_float(instr);
            if(MenuData.index[MenuData.current_id] == 1)
            {
                if( input_num > 100)
                {
                    strcpy(instr,"100"); 
                    input_index = sizeof("100") - 1;
                }
                Lcd_printf20x20(30, 30,"电池容量:%sAh    ",instr);
               
            }
            else if(MenuData.index[MenuData.current_id] == 2)
            {
                if( input_num > 25)
                {
                    strcpy(instr,"25"); 
                    input_index = sizeof("25") - 1;
                }
                Lcd_printf20x20(30, 60,"充电电流:%sA    ",instr);
            }
            if(MenuData.index[MenuData.current_id] == 3)
            {
                if( input_num > 100)
                {
                    strcpy(instr,"100"); 
                    input_index = sizeof("100") - 1;
                }
                Lcd_printf20x20(30, 90,"最大功率:%sw    ",instr);
            }
            if(MenuData.index[MenuData.current_id] == 4)
            {
                if( input_num > 5)
                {
                    strcpy(instr,"5"); 
                    input_index = sizeof("5") - 1;
                }
                Lcd_printf20x20(30, 120,"涓流电流:%sA   ",instr);
            }
             
            log_info("KEY %d NUMKRY %d input_index %d",key,num_key,input_index);
            log_info("instr %s value: %d",instr,(uint32_t)(input_num*100));

        }
        else
        {
            //Mppt_Normal_Menu_Select(key);
            switch (key)
            {
                case KEY_VALUE_TYPE_INCRE: // →
                        switch (MenuData.index[MenuData.current_id])
                        {
                            case 1:
                                SetPara->Bat_Capcity += 0.5;
                                if (SetPara->Bat_Capcity>20)SetPara->Bat_Capcity = 5;
                                break;
                            case 2:
                                if(++SetPara->Charge_Current_Max > 20) SetPara->Charge_Current_Max = 2;
                                break;
                            case 3:
                                if(++SetPara->Charge_Power_Max > 75) SetPara->Charge_Power_Max = 10;
                                break;
                            case 4:
                                SetPara->Trickle_Current += 0.1;
                                if(SetPara->Trickle_Current>2.5)SetPara->Trickle_Current = 0.5;
                                break;
                        }
                        Mppt_ChargePara_Display(SetPara);
                    break;
                case KEY_VALUE_TYPE_DECRE:
                        switch (MenuData.index[MenuData.current_id])
                        {
                            case 1:
                                SetPara->Bat_Capcity -= 0.5;
                                if (SetPara->Bat_Capcity<7.5)SetPara->Bat_Capcity = 20;
                                break;
                            case 2:
                                if(--SetPara->Charge_Current_Max < 2) SetPara->Charge_Current_Max = 20;
                                break;
                            case 3:
                                if(--SetPara->Charge_Power_Max < 10) SetPara->Charge_Power_Max = 75;
                                break;
                            case 4:
                                SetPara->Trickle_Current -= 0.1;
                                if(SetPara->Trickle_Current<0.5)SetPara->Trickle_Current = 2.5;
                                break;
                        }
                        Mppt_ChargePara_Display(SetPara);
                    break;
            }
        }

    }
    void Mppt_Charge_Set_Menu_Operation(uint8_t key)
    {
        Mppt_ChargeSet_Operation(&RoterData.Mppt_SetPara,key);
        switch (key)
        {
            case KEY_VALUE_TYPE_UP: // 上
                if (MenuData.index[MenuData.current_id] > 1)
                    MenuData.index[MenuData.current_id]--;
                else
                {
                    MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
                    Menu_Tab[MenuData.current_id].display_operation();
                }
                    
                Mppt_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_DOWN: // 下
                if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
                if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                    MenuData.index[MenuData.current_id]++;
                else{
                     MenuData.current_id = DISCHAR_SET_MENU;
                     Menu_Tab[MenuData.current_id].display_operation();
                }
                Mppt_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_LEFT: // ←
                MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
                Menu_Tab[MenuData.current_id].display_operation();
                break;
        }
        // if(key == KEY_VALUE_TYPE_RIGHT)
        // {
        //     MenuData.current_id = DISCHAR_SET_MENU;
        //     Menu_Tab[MenuData.current_id].display_operation();
        // }
    }
  
    void Mppt_DischarPara_Display(Mppt_Set_Parm_t *SetPara)
    {   
        char str[7] = "";      
        Lcd_printf20x20(120 - 24 * 3, 0, "放电参数设置"); 
        floatToString(SetPara->Low_voltage_Protect,2,str);
        Lcd_printf20x20(30, 30, "低压保护:%sv    ",str);
        Lcd_printf20x20(30, 60, "电流挡位:%d     ",SetPara->Current_Gear);
        Lcd_printf20x20(30, 90, "雷达感应:%d%%   ",SetPara->Ledar_Pwm);
        Lcd_printf20x20(30, 120, "雷达时间:%dS   ",SetPara->Ledar_Dly_Time);
        Lcd_printf20x20(30, 150, "亮度设置:%d%%  ",SetPara->Led_Set_Pwm);
        Lcd_printf20x20(30, 180, "光控模式:%s   ",SolarModeStr[SetPara->Solar_Mode]);
        Lcd_printf20x20(30, 210, "通信模式:%s   ",Connect_Mode_Str[SetPara->Extern_Mode]);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
    }
    void Mppt_Dischar_Set_Menu(void)
    {
        Lcd_Clear(BLACK);
        MenuData.index[MenuData.current_id] = 1;
        Mppt_DischarPara_Display(&RoterData.Mppt_SetPara);
    }
    void Mppt_DischarSet_Operation(Mppt_Set_Parm_t *SetPara,uint8_t key)
    {
        int num_key = Get_Num_Map(key);
        float indata; 
        if(num_key != -1) // 有数字输入 第一个点不能是字符串输入
        {
            if(!InputMode && MenuData.index[MenuData.current_id]<=5)
            {
                InputMode = 1; //  输入模式
                InputFlash_Timer = sys_timer_add(NULL,Mppt_Normal_InPutFlash,300);
                input_index = 0;
                memset(instr,0,sizeof(instr));
            }  
        }
        if(InputMode)
        {
            if(num_key != -1)
            {
                if(input_index<4)input_index++;
                instr[input_index - 1] = num_key + '0';
            }
            
            switch (key)
            {

                case KEY_VALUE_TYPE_DOT: // 输入小数点
                    if(input_index<6)input_index++;
                    instr[input_index - 1] = '.';
                    break;
                
                case KEY_VALUE_TYPE_BACKSPACE: // 退格
                    if(input_index)
                    {   
                        if(input_index == 1)instr[input_index - 1] = '0';
                        else instr[input_index - 1] = 0; 
                        input_index --;       
                    }
                    break;
                case KEY_VALUE_TYPE_ENTRE:
                    indata = string_to_float(instr);
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:          
                            if(indata<2.5)indata = 2.5;
                            if(indata>2.851)indata = 2.85;
                            SetPara->Low_voltage_Protect=indata;
                            break;
                        case 2:
                            if(indata<1)indata = 1;
                            if(indata>20)indata = 20;
                            SetPara->Current_Gear=indata;
                            break;
                        case 3:
                            if(indata<0)indata = 0;
                            if(indata>30)indata = 30;
                            SetPara->Ledar_Pwm=indata;
                            break;
                        case 4:
                            if(indata<0)indata = 0;
                            if(indata>60)indata = 60;
                            SetPara->Ledar_Dly_Time=indata;
                            break; 
                        case 5:
                            if(indata<10)indata = 10;
                            if(indata>100)indata = 100;
                            SetPara->Led_Set_Pwm=indata;
                            break;      
                    }    
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_DischarPara_Display(SetPara);
                    return;
                    break;       
                 case KEY_VALUE_TYPE_LEFT:
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_DischarPara_Display(SetPara);
                    return;
                    break;
            }
            
            float input_num = string_to_float(instr);
            if(MenuData.index[MenuData.current_id] == 1)
            {
                if( input_num > 2.85)
                {
                    strcpy(instr,"2.85"); 
                    input_index = sizeof("2.85") - 1;
                }
                Lcd_printf20x20(30, 30,"低压保护:%sV   ",instr);
            }
            else if(MenuData.index[MenuData.current_id] == 2)
            {
                if( input_num > 20)
                {
                    strcpy(instr,"20"); 
                    input_index = sizeof("20") - 1;
                }
                Lcd_printf20x20(30, 60,"电流挡位:%s     ",instr);
            }
            else if(MenuData.index[MenuData.current_id] == 3)
            {
                if( input_num > 30)
                {
                    strcpy(instr,"30"); 
                    input_index = sizeof("30") - 1;
                }
                Lcd_printf20x20(30, 90,"雷达感应:%s%%    ",instr);
            }
            else if(MenuData.index[MenuData.current_id] == 4)
            {
                if( input_num > 60)
                {
                    strcpy(instr,"60"); 
                    input_index = sizeof("60") - 1;
                }
                 Lcd_printf20x20(30, 120,"雷达时间:%sS   ",instr);
            }
            else if(MenuData.index[MenuData.current_id] == 5)
            {
                if( input_num > 100)
                {
                    strcpy(instr,"100"); 
                    input_index = sizeof("100") - 1;
                }
                Lcd_printf20x20(30, 150,"亮度设置:%s%%  ",instr);
            }
            
             
            log_info("KEY %d NUMKRY %d input_index %d",key,num_key,input_index);
            log_info("instr %s  value: %d",instr,(uint32_t)(string_to_float(instr)*100));

        }
        else
        {
            //Mppt_Normal_Menu_Select(key);
            switch (key)
            {
                
                case KEY_VALUE_TYPE_RIGHT:
                    if(MenuData.index[MenuData.current_id] == 6)
                    {

                        if(SetPara->Solar_Mode) SetPara->Solar_Mode= 0;
                        else SetPara->Solar_Mode = 1;
                        Mppt_DischarPara_Display(SetPara);
                    }       
                    else if(MenuData.index[MenuData.current_id] == 7)
                    {
                        if(SetPara->Extern_Mode) SetPara->Extern_Mode = 0;
                        else SetPara->Extern_Mode = 1;
                       
                        Mppt_DischarPara_Display(SetPara);
                    }                   
                    break;
                case KEY_VALUE_TYPE_INCRE: // 
                        switch (MenuData.index[MenuData.current_id])
                        {
                            case 1:
                                SetPara->Low_voltage_Protect += 0.0501;
                                if(SetPara->Low_voltage_Protect>2.85)SetPara->Low_voltage_Protect = 2.5;
                                break;
                            case 2:
                                if(++SetPara->Current_Gear > 20) SetPara->Current_Gear = 1;
                                break;
                            case 3:
                                if(++SetPara->Ledar_Pwm > 30) SetPara->Ledar_Pwm = 10;
                                break;                          
                            case 4:
                                if(++SetPara->Ledar_Dly_Time>15)SetPara->Trickle_Current = 5;
                                break;      
                            case 5:
                                if(++SetPara->Led_Set_Pwm>100)SetPara->Trickle_Current = 10;
                                break;
                        }
                        Mppt_DischarPara_Display(SetPara);
                    break;
                case KEY_VALUE_TYPE_DECRE:
                        switch (MenuData.index[MenuData.current_id])
                        {
                            case 1:
                                SetPara->Low_voltage_Protect -= 0.0501;
                                if(SetPara->Low_voltage_Protect<2.5)SetPara->Low_voltage_Protect = 2.5;
                                break;
                            case 2:
                                if(--SetPara->Current_Gear <= 0) SetPara->Current_Gear = 20;
                                break;
                            case 3:
                                if(--SetPara->Ledar_Pwm < 10) SetPara->Ledar_Pwm = 30;
                                break;
                            case 4:
                                if(--SetPara->Ledar_Dly_Time<5)SetPara->Ledar_Dly_Time = 15;
                                break;      
                            case 5:
                                if(--SetPara->Led_Set_Pwm<=0)SetPara->Led_Set_Pwm = 100;
                                break;
                        }
                        Mppt_DischarPara_Display(SetPara);
                    break;
            }   
        }
    }
    void Mppt_Dischar_Set_Menu_Operation(uint8_t key)
    {
        Mppt_DischarSet_Operation(&RoterData.Mppt_SetPara,key);
         switch (key)
        {
            case KEY_VALUE_TYPE_UP: // 上
                if (MenuData.index[MenuData.current_id] > 1)
                    MenuData.index[MenuData.current_id]--;
                else
                {
                    MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
                    Menu_Tab[MenuData.current_id].display_operation();
                }
                    
                Mppt_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_DOWN: // 下
                if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
                if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                    MenuData.index[MenuData.current_id]++;
                else{
                        MenuData.current_id = CURVE_SET_MENU;
                        Menu_Tab[MenuData.current_id].display_operation();
                }
                Mppt_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_LEFT: // ←
                MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
                Menu_Tab[MenuData.current_id].display_operation();
                break;
        }
        // if(key == KEY_VALUE_TYPE_RIGHT)
        // {
        //     MenuData.current_id = CURVE_SET_MENU;
        //     Menu_Tab[MenuData.current_id].display_operation();
        // }
    }

    void Mppt_Curce_Set_Menu(void)
    {
        Lcd_Clear(BLACK);
        MenuData.index[MenuData.current_id] = 1;
        Lcd_printf20x20(120 - 20 * 3, 0, "曲线参数设置");
        Lcd_printf20x20(30, 30, "放电曲线模式：%s",Curve_Mode_Str[RoterData.Mppt_SetPara.DischarCurve_Moed]);
        Lcd_printf20x20(30, 60,"放电曲线设置");
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
    }
    void Mppt_Curce_Set_Menu_Operation(uint8_t key)
    {
       // Mppt_Normal_Menu_Select(key);
        switch (key)
        {
            case KEY_VALUE_TYPE_UP: // 上
                if (MenuData.index[MenuData.current_id] > 1)
                    MenuData.index[MenuData.current_id]--;
                else
                {
                    MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
                    Menu_Tab[MenuData.current_id].display_operation();
                }
                    
                Mppt_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_DOWN: // 下
                if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
                if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                    MenuData.index[MenuData.current_id]++;
                else{
                        MenuData.current_id = BL_ATCON_SET_MENU;
                        Menu_Tab[MenuData.current_id].display_operation();
                }
                Mppt_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_LEFT: // ←
                MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
                Menu_Tab[MenuData.current_id].display_operation();
                break;
        }
        switch (key)
        {
            case KEY_VALUE_TYPE_ENTRE:
              switch (MenuData.index[MenuData.current_id])
                {
                    case 2:
                        MenuData.current_id = DISCHAR_CURVE_SET_MENU; // 切换到下级菜单
                        Menu_Tab[MenuData.current_id].display_operation();
                        return ;
                        break;
                }  
            case KEY_VALUE_TYPE_INCRE:
                switch (MenuData.index[MenuData.current_id])
                {
                    case 1:
                        if(++RoterData.Mppt_SetPara.DischarCurve_Moed>2)RoterData.Mppt_SetPara.DischarCurve_Moed = 0;
                        Mppt_Curce_Set_Menu();
                        return ;
                        break;      
                }  
            // case KEY_VALUE_TYPE_RIGHT: // 
            //     MenuData.current_id = BL_ATCON_SET_MENU; // 切换到下级菜单       
            //     Menu_Tab[MenuData.current_id].display_operation();
            //     break;
        }   
    }

    void Mppt_Curve_InPutFlash(void)
    {
        uint8_t MenuId = MenuData.current_id;
        static u8 flash = 1;
        flash = !flash;
        if(flash)
        {
            if( MenuData.index[MenuId] / 9 )
                 Lcd_printf20x20(24 + 10*10, 20 * (MenuData.index[MenuId]-8) + 10, "＞");
            else
                Lcd_printf20x20(0, 20 * MenuData.index[MenuId] + 10, "＞");
        }
        else
        {
            if( MenuData.index[MenuData.current_id] / 9 )
                 Lcd_printf20x20(24 + 10*10, 20 * (MenuData.index[MenuId]-8) + 10, "  ");
            else
                Lcd_printf20x20(0, 20 * MenuData.index[MenuId] + 10, "  ");
        }
    }
    void Mppt_Curve_Menu_Select_Display(void)
    {
        uint8_t i;
        uint8_t MenuId = MenuData.current_id;
        for (i = 1; i <= 8; i++)
        {
              Lcd_Clear20x20(0, 20 * i + 10);
              Lcd_Clear20x20(24 + 10*10, 20 * i + 10);
        }
        //限制下标长度
        if (!MenuData.index[MenuId])MenuData.index[MenuId] = 1;        
        if (MenuData.index[MenuId] >= 16)MenuData.index[MenuId] = 16;
        
        if( MenuData.index[MenuId] / 9 )
            Lcd_printf20x20(24 + 10*10, 20 * (MenuData.index[MenuId]-8) + 10, "＞");
        else
            Lcd_printf20x20(0, 20 * MenuData.index[MenuId] + 10, "＞");
    }
    void Mppt_DischarCurve_Display(Mppt_Set_Parm_t *Para)
    {
        uint8_t i;
        uint8_t str[10];
        float (*CurvData)[2] = Para->Curv_Data;
        const char *Dischar_Curve_Str[]={"PWM","Cur","AI"};
        char *Curvestr = Dischar_Curve_Str[Para->DischarCurve_Moed];
        if(Para->DischarCurve_Moed < 2)
        {     
            Lcd_printf20x20(120 - 20 * 3, 0, "放电曲线设置");
            for(i=0;i<8;i++)
            {    
                floatToString(CurvData[i][0],1,str);
                Lcd_printf20x20(24,20*(i+1)+ 10,"曲%d %sH ",i+1,str);
            }
            for(i=0;i<8;i++)
            {   
                Lcd_printf20x20(24 + 12*10, 20*(i+1)+ 10,"%s:%d%%  ",Curvestr,(uint32_t)CurvData[i][1]);
            }
        }
        else
        {  
            Lcd_printf20x20(120 - 20 * 3, 0, "放电曲线设置");
            Lcd_printf20x20(120 - 10 * 3, 30,"Ai模式");
        }
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
    void Mppt_CurveSet_Operation(Mppt_Set_Parm_t *Para,uint8_t key)
    {
        float (*CurvData)[2] = Para->Curv_Data;
        char *Curvestr = Dischar_Curve_Str[Para->DischarCurve_Moed];
        uint8_t Menu_Id = MenuData.current_id; // 当前ID号
        uint8_t *Menu_Index = &MenuData.index[Menu_Id];
        uint8_t data_index;
        int num_key = Get_Num_Map(key);
        float indata;

        if(num_key != -1) // 有数字输入 第一个点不能是字符串输入
        {
            if(!InputMode && *Menu_Index<=16)
            {
                InputMode = 1; //  输入模式
                InputFlash_Timer = sys_timer_add(NULL,Mppt_Curve_InPutFlash,300);
                input_index = 0;
                memset(instr,0,sizeof(instr));
            }  
        }
        if(InputMode)
        {
            if(num_key != -1)
            {
                if(input_index<4)input_index++;
                instr[input_index - 1] = num_key + '0';
            }
            
            switch (key)
            {

                case KEY_VALUE_TYPE_DOT: // 输入小数点
                    if(input_index<4)input_index++;
                    instr[input_index - 1] = '.';
                    break;
                
                case KEY_VALUE_TYPE_BACKSPACE: // 退格
                    if(input_index)
                    {   
                        if(input_index == 1)instr[input_index - 1] = '0';
                        else instr[input_index - 1] = 0; 
                        input_index --;       
                    }
                    break;
                case KEY_VALUE_TYPE_ENTRE:
                    indata = string_to_float(instr);
                    data_index = (*Menu_Index-1)%8;  
                    if((*Menu_Index)/9)
                    {
                         //if(data_index)if(indata > CurvData[data_index-1][1]) indata = CurvData[data_index-1][1];
                         //else if(data_index<7)if(indata < CurvData[data_index+1][1]) indata = CurvData[data_index+1][1];
                        CurvData[data_index][1] = indata;
                        // if(abs(indata) < 1e-6)
                        // {
                        //     CurvData[data_index][0] = 0;
                        // }
                    }
                    else
                    {
                        if(data_index == 0) // 前面的必须小于后面的
                        {
                            if(indata >= CurvData[1][0]) indata = CurvData[1][0];
                        }
                        else if(data_index<7)
                        {
                            
                            if(indata > CurvData[data_index+1][0] &&  (abs(CurvData[data_index+1][0])>1e-6) ) 
                                indata = CurvData[data_index+1][0];
                            if(abs(indata) < 1e-6) // 如果输入0
                            {
                                if(abs(CurvData[data_index+1][0]) > 1e-6  ) indata = CurvData[data_index-1][0];
                            }
                        }
                        else
                        {
                            if(abs(indata) > 1e-6)  // indata > 0
                            {
                                if(indata < CurvData[data_index-1][0]) indata = CurvData[data_index-1][0];
                            }
                        }
                        CurvData[data_index][0] = indata;
                        if(abs(indata) < 1e-6)
                        {
                            CurvData[data_index][1] = 0;
                        }
                    }      
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_DischarCurve_Display(Para);
                    return;
                    break;
                 case KEY_VALUE_TYPE_LEFT:
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_DischarCurve_Display(Para);
                    return;
                    break;
            }
             
            float input_num = string_to_float(instr);
            log_info("KEY %d NUMKRY %d input_index %d",key,num_key,input_index);
            log_info("instr %s  value: %d",instr,(uint32_t)(input_num*100)); //放大小数点 看是否有错误

            if( (*Menu_Index/9) == 0)
            {
                if( input_num > 24)
                {
                    strcpy(instr,"24"); 
                    input_index = sizeof("24") - 1;
                }    
                Lcd_printf20x20(24,20*(*Menu_Index) + 10,"曲%d %sH   ",*Menu_Index,instr);
            }
            else
            {
                if( input_num > 100)
                {
                    strcpy(instr,"100"); 
                    input_index = sizeof("100") - 1;
                }    
                Lcd_printf20x20(24 + 12*10, 20*(*Menu_Index - 8) + 10,"%s:%s%%   ",Curvestr,instr);
            }
               
        }
        else
        {
            switch (key)
            {
                case KEY_VALUE_TYPE_UP: // 上
                    if(Para->DischarCurve_Moed > 1)return;
                    if(!Menu_Tab[Menu_Id].max_menu_num)return;
                    if (*Menu_Index > 1)*Menu_Index-=1;    
                    else *Menu_Index = Menu_Tab[Menu_Id].max_menu_num;                
                    Mppt_Curve_Menu_Select_Display();
                    break;
                case KEY_VALUE_TYPE_DOWN: // 下
                    if(Para->DischarCurve_Moed > 1)return;
                    if(!Menu_Tab[Menu_Id].max_menu_num)return;
                    if (*Menu_Index < Menu_Tab[Menu_Id].max_menu_num)*Menu_Index+=1;
                    else *Menu_Index = 1;
                    Mppt_Curve_Menu_Select_Display();
                    break;
                case KEY_VALUE_TYPE_LEFT: // ←    
                    if( ( *Menu_Index / 9 ) && Para->DischarCurve_Moed <= 1)
                    {
                        *Menu_Index -= 8;
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
                    if(Para->DischarCurve_Moed > 1)return;
                    if( ( *Menu_Index / 9 ) == 0)
                    {
                        *Menu_Index += 8; 
                        Mppt_Curve_Menu_Select_Display();
                    }
                    break;
                case KEY_VALUE_TYPE_INCRE:
                    if( ( *Menu_Index / 9 ) == 0)
                    {
                        data_index =  *Menu_Index -1;
                        // 1 - 8  前面要小于后面 || 后面的<0.1  
                        if((CurvData[data_index][0] < 0.000001f) && data_index )CurvData[data_index][0] = CurvData[data_index-1][0]; 
                        if(  data_index == 7 || (CurvData[data_index][0]+0.5) < CurvData[data_index+1][0] || (CurvData[data_index+1][0] < 0.000001f) )
                        {   
                            CurvData[data_index][0] += 0.5f; 
                            if(CurvData[data_index][0]>24)CurvData[data_index][0] = 24;
                        } 
                    }
                    else
                    {
                        data_index = (*Menu_Index-1)%8;   
                        // if((CurvData[data_index][1] < 0.000001f) && data_index )CurvData[data_index][1] = CurvData[data_index-1][1] ; 
                        // if( data_index == 0 || (CurvData[data_index][1] + 5) < CurvData[data_index - 1][1]  )
                        // {
                            CurvData[data_index][1] += 5;
                            if(CurvData[data_index][1]>100)CurvData[data_index][1] = 100;
                        //}
                    }
                    Mppt_DischarCurve_Display(Para);
                    break;
                case KEY_VALUE_TYPE_DECRE:
                    if( ( *Menu_Index / 9 ) == 0)
                    {
                        data_index =  *Menu_Index -1;
                        if((CurvData[data_index][0] < 0.000001f) && data_index )CurvData[data_index][0] = CurvData[data_index-1][0]; 
                        if(  (data_index == 0) || (CurvData[data_index][0]-0.5f) > (CurvData[data_index-1][0])  )
                        {
                            CurvData[data_index][0] -= 0.5f;  
                            if(CurvData[data_index][0]<0.5)CurvData[data_index][0] = 0.5f;
                        }
                    }
                    else
                    {
                        data_index = (*Menu_Index-1)%8; 
                        // if((CurvData[data_index][1] < 0.000001f) && data_index )CurvData[data_index][1] = CurvData[data_index-1][1] ; 
                        // if( (data_index == 7) || (CurvData[data_index][1]- 5) > (CurvData[data_index + 1][1] ) )
                        // {
                            CurvData[data_index][1] -= 5;
                            if(CurvData[data_index][1]<0)CurvData[data_index][1] = 0;
                        //}
                    }
                    Mppt_DischarCurve_Display(Para);
                    break;
            }
        }
    }
    void Mppt_Dischar_Curve_Set_Operation(uint8_t key)
    {   
        Mppt_CurveSet_Operation(&RoterData.Mppt_SetPara,key);
    }

    static u16 Ble_AutoConnect_timer = NULL;

    void Mppt_Ble_AutoConnect_Display(void)  
    {
        uint8_t location;
        location = Ble_Find_Mac_RepAddr(RoterData.Ble_Adv_rp,sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]),RoterData.Ble_Connect_Mac);
                
        if(!RoterData.Ble_Adv_rp[location].IsModifyFlag)
        {
            Mppt_Set_Para_Send(&RoterData.Mppt_SetPara);
        }
        Lcd_printf20x20(120 - 24 * 3, 0, "蓝牙自动设置");
        Lcd_printf20x20(30, 30, "自动设置：%s",AotoSetStr[RoterData.ConnenctOnFlag]);
        Lcd_printf20x20(30, 60, "清空设置记录");
        Lcd_printf20x20(30, 90, "过滤锁定：%s",Lock_Mode_Str[RoterData.Filter_LockFlag]);
        Lcd_printf20x20(30, 120,"在线数：%d",RoterData.Ble_Adv_Rp_Count);
        Lcd_printf20x20(30, 150,"已设置：%d",RoterData.SetCount);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
    }
    void Mppt_Ble_AutoConnect_set_Menu(void)
    {
        Lcd_Clear(BLACK);
        RoterData.ConnenctOnFlag = 0;
        RoterData.Filter_LockFlag = 1;
        Mppt_Ble_AutoConnect_Display();
    }
    void Mppt_Ble_AutoConnect_set_Menu_Operation(uint8_t key)
    {
        uint32_t i;  
        switch (key)
        {
            case KEY_VALUE_TYPE_ENTRE:
            case KEY_VALUE_TYPE_RIGHT: // 
                switch (MenuData.index[MenuData.current_id])
                {
                    case 1:
                        RoterData.ConnenctOnFlag = !RoterData.ConnenctOnFlag;
                        if(Ble_AutoConnect_timer == NULL)
                            Ble_AutoConnect_timer = sys_timer_add(NULL,Mppt_Ble_AutoConnect_Display,500);
                        Mppt_Ble_AutoConnect_Display();
                        break;
                    case 2:
                        for(i=0;i<sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]);i++)
                            RoterData.Ble_Adv_rp[i].IsModifyFlag = 0;
                        RoterData.ConnenctOnFlag = 0;
                        RoterData.SetCount = 0;
                        Mppt_Ble_AutoConnect_Display();       
                        return ;
                        break;
                    case 3:
                        RoterData.Filter_LockFlag = !RoterData.Filter_LockFlag;
                        Mppt_Ble_AutoConnect_Display();   
                        break;
                }
            break;
            case KEY_VALUE_TYPE_LEFT:
                sys_timer_del(Ble_AutoConnect_timer);
                RoterData.ConnenctOnFlag = 0;
                RoterData.Filter_LockFlag = 1;
                Ble_AutoConnect_timer = NULL;
                break;
        }   
        Mppt_Normal_Menu_Select(key);
    }
    
static u16 bl_con_menu_timer;
static int Rssi_Cmpsort(const void*p1,const void*p2)
{
    return(*(Ble_Adv_Rp_t*)p1).rssi<(*(Ble_Adv_Rp_t*)p2).rssi?1:-1;
}

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
    //Get_Mppt_Report();
    //Get_Mppt_Report1();
    
    for (i = 0; i < 13; i++)
    {
        sprintf(str, "%02X %02X %02X %02X %02X %02X rssi:%d  ",
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
    MPPT_Get_Info_Timer_Star();
    Lcd_Clear(BLACK); 
    Lcd_printf20x20(120 - 24 * 3, 0, "蓝牙指定连接");
    MenuData.index[MenuData.current_id] = 1;
    Lcd_printf16x16(5, 8 + 16 * MenuData.index[MenuData.current_id], ">");
    Mppt_Ble_Mac_Info_Display();
    bl_con_menu_timer = sys_timer_add(NULL, Mppt_Ble_Mac_Info_Display, 200);
}
void Mppt_Ble_con_Select_Menu_Operation(uint8_t key)
{
    u16 conn_handle = ble_comm_dev_get_handle(0, 1);
    uint8_t *index = &MenuData.index[MenuData.current_id];
    switch (key) 
    {
        case KEY_VALUE_TYPE_UP: // 上
            if (index[0] > 1)index[0]--;      
            else index[0] = Menu_Tab[MenuData.current_id].max_menu_num;

            ble_gatt_client_disconnect_all();
            memcpy(RoterData.Ble_SetConnect_Mac,RoterData.Ble_Adv_rp[index[0]-1].mac,6);
            MPPT_Get_Info_Timer_Star();
            Mppt_BLe_Menu_Select_Display();

            //log_info("mac set");
            put_buf(RoterData.Ble_SetConnect_Mac,6);
            put_buf(RoterData.Ble_Adv_rp[index[0]-1].mac,6);    

            
            break;
        case KEY_VALUE_TYPE_DOWN: // 下

         
            if (index[0] < Menu_Tab[MenuData.current_id].max_menu_num)
                index[0]++;
            else
                index[0] = 1;   
                
            ble_gatt_client_disconnect_all();
            memcpy(RoterData.Ble_SetConnect_Mac,RoterData.Ble_Adv_rp[index[0]-1].mac,6);
            MPPT_Get_Info_Timer_Star();
            Mppt_BLe_Menu_Select_Display();

            break;
        case KEY_VALUE_TYPE_LEFT: // ←
            sys_timer_del(bl_con_menu_timer); 
            
            ble_gatt_client_disconnect_all();
            memset(RoterData.Ble_SetConnect_Mac,0,6);

            MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
            Menu_Tab[MenuData.current_id].display_operation();
            break;
        case KEY_VALUE_TYPE_RIGHT: // →
            sys_timer_del(bl_con_menu_timer);
            //ble_gatt_client_disconnect_all();
            MenuData.current_id = BL_CON_SET_MENU; //切换到下级菜单
            if( Menu_Tab[MenuData.current_id].current_operation != NULL)Menu_Tab[MenuData.current_id].display_operation();
            break;
    }
}


static u16 Cutdown_cnt_Timer = 0;
static u8 Cutdown_cnt = 5;  //倒计时
void Mppt_Ble_connCount_Cutdown()
{
    int len = sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]);
    int size = sizeof(RoterData.Ble_Adv_rp[0]);
    Cutdown_cnt--;
    Lcd_printf20x20(120 - 10 * 4, 120, "倒计时:%d",Cutdown_cnt);
    qsort(&RoterData.Ble_Adv_rp,len,size,Rssi_Cmpsort);
    if(!Cutdown_cnt)
    {
        if(Cutdown_cnt_Timer)
        {
            sys_timer_del(Cutdown_cnt_Timer);
            Cutdown_cnt_Timer = 0;
        } 
        MenuData.current_id = BL_CON_SELECT_MENU;
        Menu_Tab[MenuData.current_id].display_operation();
        memcpy(RoterData.Ble_SetConnect_Mac,RoterData.Ble_Adv_rp[0].mac,6);
        //MPPT_Get_Info_Timer_Star();
    }
}
void Mppt_Ble_con_Menu(void) //界面初始化
{
    Lcd_Clear(BLACK);
    Cutdown_cnt = 5;
    Lcd_printf20x20(120 - 20 * 4, 120 - 20, "蓝牙搜索中......");
    Lcd_printf20x20(120 - 10 * 4, 120, "倒计时:%d",Cutdown_cnt);
    Cutdown_cnt_Timer = sys_timer_add(NULL,Mppt_Ble_connCount_Cutdown,1000);
}
void Mppt_Ble_con_Menu_Operation(uint8_t key)
{
    switch (key)
    {
        case KEY_VALUE_TYPE_RIGHT: 
            MenuData.current_id = BL_CON_SELECT_MENU;
            Menu_Tab[MenuData.current_id].display_operation();
            memcpy(RoterData.Ble_SetConnect_Mac,RoterData.Ble_Adv_rp[0].mac,6);
            if(Cutdown_cnt_Timer)
            {
                sys_timer_del(Cutdown_cnt_Timer);
                Cutdown_cnt_Timer = 0;
            }
            break;
        case KEY_VALUE_TYPE_LEFT: 
            MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
            if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                Menu_Tab[MenuData.current_id].display_operation();
            if(Cutdown_cnt_Timer)
            {
                sys_timer_del(Cutdown_cnt_Timer);
                Cutdown_cnt_Timer = 0;
            }
            break;
    }
}


    static u16 Get_Info_Timer = 0;
    static u16 Modify_Info_Timer = 0;
    static u16 Display_Info_Timer = 0;
    void Mppt_Ble_Set_Menu(void)
    {
        Lcd_Clear(BLACK);
        Lcd_printf20x20(120 - 24 * 3, 0, "蓝牙指定连接");
        Lcd_printf20x20(30,30, "ＭＰＰＴ信息查询");
        Lcd_printf20x20(30,60, "充电参数修改");
        /*Lcd_printf20x20(30,90, "放电参数修改");
        Lcd_printf20x20(30,120,"曲线模式修改：%s",Curve_Mode_Str[RoterData.Mppt_ConSetPara_Info.DischarCurve_Moed]);
        Lcd_printf20x20(30,150,"放电曲线修改");
        Lcd_printf20x20(30,180,"锁定模式：%s",Lock_Mode_Str[RoterData.Mppt_ConSetPara_Info.Lock_Mode]);*/

        uint32_t const bl_ckey[4]= {12,34,56,78}; 
        uint8_t data[8]  =
        {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        };
        for(int i=0;i<6;i++)data[i] = RoterData.Ble_Connect_Mac[i];
        btea(data,2,bl_ckey);
        RoterData.paircode = little_endian_read_32(data,0);
         RoterData.paircode%=999999;

        Lcd_printf20x20(30,90,"code:%d",RoterData.paircode);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
        //MPPT_Get_Info_Timer_Star();
    }     
    void Mppt_Ble_Set_Operation(uint8_t key)
    {
        switch (key)
        {
            case KEY_VALUE_TYPE_ENTRE:
            case KEY_VALUE_TYPE_RIGHT: // 
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            Get_Info_Timer = sys_timer_add(NULL,Get_Mppt_Report,500);
                            MenuData.current_id = MPPT_INFO; // 切换到下级菜单
                            break;
                        case 2:
                            MenuData.current_id = CHAEGE_PARA_MODIFY; // 切换到下级菜单
                            break;
                        case 3:
                            MenuData.current_id = DISCHAR_PARA_MODIFY; // 切换到下级菜单
                            break;
                        case 4:
                            if(++RoterData.Mppt_ConSetPara_Info.DischarCurve_Moed>2)RoterData.Mppt_ConSetPara_Info.DischarCurve_Moed = 0;
                            Lcd_printf20x20(30,120,"曲线模式修改：%s",Curve_Mode_Str[RoterData.Mppt_ConSetPara_Info.DischarCurve_Moed]);
                            return ;
                            break;
                        case 5:
                            MenuData.current_id = CURVE_PARAT_MENU;
                            break;
                        case 6:
                            RoterData.Mppt_ConSetPara_Info.Lock_Mode = !RoterData.Mppt_ConSetPara_Info.Lock_Mode;
                            Lcd_printf20x20(30,180,"锁定模式：%s",Lock_Mode_Str[RoterData.Mppt_ConSetPara_Info.Lock_Mode]);
                            return ;
                            break;
                        case 7:
                            Modify_Info_Timer = sys_timer_add(&RoterData.Mppt_ConSetPara_Info,Mppt_Set_Para_Send,1000);
                            MenuData.current_id = ENTRY_MODIFY; // 切换到下级菜单
                            break;
                    }
                    if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                                Menu_Tab[MenuData.current_id].display_operation();
                break;
            case KEY_VALUE_TYPE_LEFT: //断开所有连接
                memset(RoterData.Ble_SetConnect_Mac,0,6);
                ble_gatt_client_disconnect_all();
                break;
        }  
        Mppt_Normal_Menu_Select(key);
    }
    
        void Mppt_Info_Display(void *priv)
        {
            Mppt_Info_Para_t *Info= priv;
            char *str[10];
            const char* OutPut_Status_Str[]={"正常，短路"};
            Timer_Auto_Off_ReCount();
            Lcd_printf20x20(120 - 24 * 3, 0, "ＭＰＰＴ信息");
          //  Info->Charge_Capcity = 0.004;
           floatToString(Info->Charge_Capcity,3,str);
            Lcd_printf20x20(30, 20 * 1, "已充电量:%sAh   ",str);
            floatToString(Info->Charge_Current,3,str);
            Lcd_printf20x20(30, 20 * 2, "充电电流:%sA    ",str);
            floatToString(Info->Dischar_Current,3,str);
           // floatToString(0.118f,3,str);
            Lcd_printf20x20(30, 20 * 3, "放电电流:%sA    ",str);
            floatToString(Info->Charge_Power,3,str);
            Lcd_printf20x20(30, 20 * 4, "充电功率:%sW    ",str);
            Lcd_printf20x20(30, 20 * 5, "电池内阻:%dm    ",Info->Bat_Resistance);
            Lcd_printf20x20(30, 20 * 6, "电池电量:%d%%   ",Info->Bat_Capcity);
            floatToString(Info->Bat_Voltage,3,str);
            Lcd_printf20x20(30, 20 * 7, "电池电压:%sV    ",str);
            //Lcd_printf20x20(30, 25 * 8, "输出状态:%s     ",OutPut_Status_Str[Info->OutPut_Staus]);
           
            floatToString(RoterData.Mppt_ConSetPara_Info.Bat_Capcity,1,str);
            Lcd_printf20x20(30,20 * 8,"电池容量:%sAh  ",str);

            
            floatToString(Info->Temp,3,str);
            Lcd_printf20x20(30,20 * 9,"温度:%s℃  ",str);
            
        }
        void Mppt_Info_Menu(void) //充电信息配置
        {
            Lcd_Clear(BLACK);
            Mppt_Info_Display(&RoterData.Mppt_Info);
            Display_Info_Timer = sys_timer_add(&RoterData.Mppt_Info,Mppt_Info_Display,500);
        }
        void Mppt_Info_Menu_Operation(uint8_t key)
        {
            if(key == KEY_VALUE_TYPE_LEFT)
            {
                if(Display_Info_Timer)
                {
                    sys_timer_del(Display_Info_Timer);
                    Display_Info_Timer = 0;
                }
                if(Get_Info_Timer)
                {
                    sys_timer_del(Get_Info_Timer);
                    Get_Info_Timer = 0;
                }
            }
            Mppt_Normal_Menu_Select(key);
            
        }

        void Mppt_ChargePara_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);        
            Lcd_printf20x20(120 - 24 * 3, 0, "充电参数修改");
            Mppt_ChargePara_Display(&RoterData.Mppt_ConSetPara_Info);
        }
        void Mppt_ChargePara_Modify_Menu_Operation(uint8_t key)
        {
            Mppt_ChargeSet_Operation(&RoterData.Mppt_ConSetPara_Info,key);
            Mppt_Normal_Menu_Select(key);
            if(key == KEY_VALUE_TYPE_LEFT)
            {
                MenuData.current_id = BL_CON_SET_MENU;
                Menu_Tab[MenuData.current_id].display_operation();
            }
            if(key == KEY_VALUE_TYPE_RIGHT)
            {
                MenuData.current_id = DISCHAR_PARA_MODIFY;
                Menu_Tab[MenuData.current_id].display_operation();
            }
        }

        void Mppt_DischarPara_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);
            Lcd_printf20x20(120 - 24 * 3, 0, "放电参数修改");
            Mppt_DischarPara_Display(&RoterData.Mppt_ConSetPara_Info);
        }
        void Mppt_DischarPara_Modify_Menu_Operation(uint8_t key)
        {
            Mppt_DischarSet_Operation(&RoterData.Mppt_ConSetPara_Info,key);
             Mppt_Normal_Menu_Select(key);
              if(key == KEY_VALUE_TYPE_RIGHT)
            {
                MenuData.current_id = CURVE_MODIFY;
                Menu_Tab[MenuData.current_id].display_operation();
            }
        }

        void Mppt_Curce_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);
            MenuData.index[MenuData.current_id] = 1;
            Lcd_printf20x20(120 - 20 * 3, 0, "曲线参数设置");
            Lcd_printf20x20(30, 30, "放电曲线模式：%s",Curve_Mode_Str[RoterData.Mppt_ConSetPara_Info.DischarCurve_Moed]);
            Lcd_printf20x20(30, 60,"放电曲线设置");
            Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
        }
        void Mppt_Curce_Modify_Menu_Operation(uint8_t key)
        {
        // Mppt_Normal_Menu_Select(key);
            switch (key)
            {
                case KEY_VALUE_TYPE_UP: // 上
                    if (MenuData.index[MenuData.current_id] > 1)
                        MenuData.index[MenuData.current_id]--;
                    else
                    {
                        MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
                        Menu_Tab[MenuData.current_id].display_operation();
                    }
                        
                    Mppt_Menu_Select_Display();
                    break;
                case KEY_VALUE_TYPE_DOWN: // 下
                    if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
                    if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                        MenuData.index[MenuData.current_id]++;
                    else{
                            MenuData.current_id = MODIFY_MENU;
                            Menu_Tab[MenuData.current_id].display_operation();
                    }
                    Mppt_Menu_Select_Display();
                    break;
                case KEY_VALUE_TYPE_LEFT: // ←
                    MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
                    Menu_Tab[MenuData.current_id].display_operation();
                    break;
            }
            switch (key)
            {
                case KEY_VALUE_TYPE_ENTRE:
                switch (MenuData.index[MenuData.current_id])
                    {
                        case 2:
                            MenuData.current_id = CURVE_PARAT_MENU; // 切换到下级菜单
                            Menu_Tab[MenuData.current_id].display_operation();
                            return ;
                            break;
                    }  
                case KEY_VALUE_TYPE_INCRE:
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            if(++RoterData.Mppt_SetPara.DischarCurve_Moed>2)RoterData.Mppt_SetPara.DischarCurve_Moed = 0;
                            Mppt_Curce_Set_Menu();
                            return ;
                            break;      
                    }  
                case KEY_VALUE_TYPE_RIGHT: // 
                    MenuData.current_id = MODIFY_MENU; // 切换到下级菜单       
                    Menu_Tab[MenuData.current_id].display_operation();
                    break;
            }   
        }

        void Mppt_CurvePara_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);
            Lcd_printf20x20(120 - 20 * 3, 0, "放电曲线修改");
            Mppt_DischarCurve_Display(&RoterData.Mppt_ConSetPara_Info);
        }
        void Mppt_CurvePara_Modify_Menu_Operation(uint8_t key)
        {
            Mppt_CurveSet_Operation(&RoterData.Mppt_ConSetPara_Info,key);
        }

        void Mppt_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);
            Lcd_printf20x20(120 - 20 * 2, 0, "参数修改");
            Lcd_printf20x20(30,30,"确认修改");
            Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
        }
        void Mppt_Modify_Menu_Operation(uint8_t key)
        {
            Mppt_Normal_Menu_Select(key);
            if(key == KEY_VALUE_TYPE_RIGHT)
            {
                MenuData.current_id = ENTRY_MODIFY;
                Modify_Info_Timer = sys_timer_add(&RoterData.Mppt_ConSetPara_Info,Mppt_Set_Para_Send,1000);
                Menu_Tab[MenuData.current_id].display_operation();
            }
        }

        void Mppt_Comfir_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);
            Lcd_printf20x20(120 - 20 * 3, 120 - 20, "修改中......");     
        }
        void Mppt_Comfir_Modify_Menu_Operation(uint8_t key)
            {
                if(key == KEY_VALUE_TYPE_MODIFY_OK)
                {
                    if(Modify_Info_Timer)sys_timer_del(Modify_Info_Timer);
                     Lcd_Clear(BLACK);
                    Lcd_printf20x20(120 - 10 * 30, 120 - 20, "修改OK."); 
                }
                if(key == KEY_VALUE_TYPE_LEFT)
                {
                    if(Modify_Info_Timer)sys_timer_del(Modify_Info_Timer);
                }   
                Mppt_Normal_Menu_Select(key);
            }

void Mppt_Ir_Set_Menu(void)
{
    Lcd_Clear(BLACK);   
    Lcd_printf20x20(120 - 24 * 3, 0, "红外参数设置");
    Lcd_printf20x20(30, 30,  "常规遥控器");
    Lcd_printf20x20(30, 60,  "工程遥控器");
    Lcd_printf20x20(30, 90,  "用户码设置");
    Lcd_printf20x20(30, 120, "放电曲线模式：%s",Curve_Mode_Str[RoterData.Mppt_SetPara.DischarCurve_Moed]);
    Lcd_printf20x20(30, 150, "曲线设置");
    Lcd_printf20x20(30, 180, "曲线确认");
    Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
}
void Mppt_Ir_Set_Menu_Operation(uint8_t key)  
{
    uint8_t i;
    uint8_t ir_tx_data[20];
    uint16_t Check_Sum = 0;
    Mppt_Normal_Menu_Select(key);
    switch (key)
    {
        case KEY_VALUE_TYPE_ENTRE:
        case KEY_VALUE_TYPE_RIGHT:      
            switch (MenuData.index[MenuData.current_id])
            {
                case 1:
                    MenuData.current_id = IR_NORMAL_MENU; // 切换到下级菜单
                    Menu_Tab[MenuData.current_id].display_operation();  
                    break;
                case 2:
                     MenuData.current_id = IR_ENGINEER_MENU; // 切换到下级菜单
                     Menu_Tab[MenuData.current_id].display_operation();  
                    break;
                case 3:
                     MenuData.current_id = IR_USERCODE_MENU; // 切换到下级菜单
                     Menu_Tab[MenuData.current_id].display_operation();  
                    break;
                case 4:
                    if(++RoterData.Mppt_SetPara.DischarCurve_Moed>2)RoterData.Mppt_SetPara.DischarCurve_Moed = 0;
                     Lcd_printf20x20(30, 120, "放电曲线模式：%s",Curve_Mode_Str[RoterData.Mppt_SetPara.DischarCurve_Moed]);
                    break;
                case 5:
                     MenuData.current_id = IR_CRUVESET_MENU; // 切换到下级菜单
                     Menu_Tab[MenuData.current_id].display_operation();  
                    break;
                case 6: //红外数据发送
                    memset(&ir_tx_data,0,sizeof(ir_tx_data)) ;    //清空数据
                    
                    if(RoterData.Mppt_SetPara.DischarCurve_Moed == 2)
                    {
                        ir_tx_data[0] = 0X5F; ir_tx_data[1] = 0XFA;  //头码
                        ir_tx_data[2] = 0X00; ir_tx_data[3] = 0X0f;  //头码
                    }
                    else 
                    {
                        ir_tx_data[0] = 0X5F; ir_tx_data[1] = 0XF5;  //头码
                        for(i=0;i<=8;i++)   //曲线数据发送
                        {
                            ir_tx_data[2 + i*2] = (uint8_t)(RoterData.Mppt_SetPara.Curv_Data[i][0] * 10);
                            ir_tx_data[2 + i*2 + 1] = (uint8_t)(RoterData.Mppt_SetPara.Curv_Data[i][1]);
                        }
                    }
                    
                    Check_Sum = 0;
                    for(i=0;i<18;i++)Check_Sum += ir_tx_data[i];
                    ir_tx_data[18] = Check_Sum >> 8;    // 高位
                    ir_tx_data[19] = Check_Sum & 0xff;  //地位
                    Ir_tx_star_x(ir_tx_data,sizeof(ir_tx_data));
                    break; 
            }           
            //if (Menu_Tab[MenuData.current_id].display_operation != NULL)
               //  Menu_Tab[MenuData.current_id].display_operation();                                
            break;
    }
}
   
    void IR_Normal_Menu(void)
    {
        Lcd_Clear(BLACK);   
        Lcd_printf20x20( 120 - 10 * 5 , 0, "常规遥控器");
        Lcd_printf20x20(30, 30, "1:0N   2:0FF");
        Lcd_printf20x20(30, 60, "3:FULL 4:HALF");
        Lcd_printf20x20(30, 90,"+:PWM+ -:PWM-");
        Lcd_printf20x20(30, 120,"5:Rader 6:Rader 4H ");
        Lcd_printf20x20(30, 150,"7:3H 8:5H 9:8h ");
        Lcd_printf20x20(30, 180,"0:Auto");
    }
    void IR_Normal_Menu_Operation(uint8_t key)
    {
        switch (key)
        {
            case KEY_VALUE_TYPE_1: // ON 
                Ir_tx_star(0x00,0xC2);
                break;
            case KEY_VALUE_TYPE_2:
                Ir_tx_star(0x00,0XB0);
                break;  
            case KEY_VALUE_TYPE_3:
                Ir_tx_star(0x00,0X10);
                break;
            case KEY_VALUE_TYPE_4:
                Ir_tx_star(0x00,0X5A);
                break;
            case KEY_VALUE_TYPE_5:
                Ir_tx_star(0x00,0XA2);
                break;
            case KEY_VALUE_TYPE_6:
                Ir_tx_star(0x00,0X38);
                break;
            case KEY_VALUE_TYPE_7:
                Ir_tx_star(0x00,0X22);
                break;
            case KEY_VALUE_TYPE_8:
                Ir_tx_star(0x00,0XA8);
                break;
            case KEY_VALUE_TYPE_9:
                Ir_tx_star(0x00,0X68);
                break;
            case KEY_VALUE_TYPE_0:
                Ir_tx_star(0x00,0X62);
                break;
            case KEY_VALUE_TYPE_INCRE:
                Ir_tx_star(0x00,0Xe0);
                break;
            case KEY_VALUE_TYPE_DECRE:
                Ir_tx_star(0x00,0X90);
                break;
        }
        Mppt_Normal_Menu_Select(key);
    }

    static u8 Ir_current_Gear = 20;
    void IR_Engineer_Menu(void)
    {
        Lcd_Clear(BLACK);   
        Lcd_printf20x20(120 - 10 * 5 , 0, "工程遥控器");
        Lcd_printf20x20(30, 60, "1:0N 2:0FF");
        Lcd_printf20x20(30, 90, "2:IR On 3:IR Off");
        Lcd_printf20x20(30, 120,"Rader 4: On 5: Off");
        Lcd_printf20x20(30, 150,"6:Current Gare:%d",Ir_current_Gear);
        Lcd_printf20x20(0, 30 *5 , "＞");
    }
    void IR_Engineer_Menu_Operation(uint8_t key)
    {
        switch (key)
        {
            case KEY_VALUE_TYPE_1: // ON 
                Ir_tx_star_adr(0x861b,0x39);
                break;
            case KEY_VALUE_TYPE_2:
                Ir_tx_star_adr(0x861B,0XB9);
                break;  
            case KEY_VALUE_TYPE_6:
                Ir_tx_star_adr(0x861B,IR_Current_Code[Ir_current_Gear-1]);
                break;
            case KEY_VALUE_TYPE_INCRE:
                if(Ir_current_Gear<20)Ir_current_Gear++;
                Lcd_printf20x20(30, 150,"6:Current Gare:%d ",Ir_current_Gear);
                break;
            case KEY_VALUE_TYPE_DECRE:
                if(Ir_current_Gear>2)Ir_current_Gear--;
                Lcd_printf20x20(30, 150,"6:Current Gare:%d ",Ir_current_Gear);
                break;
        }
        Mppt_Normal_Menu_Select(key);
    }

    void IR_Usercode_Menu(void)
    {
        Lcd_Clear(BLACK);   
        Lcd_printf20x20( 120 - 12 * 5 , 0, "用户码设置");
        Lcd_printf20x20( 30 , 30, "用户码设置:%d",RoterData.Mppt_SetPara.Usercode);
        Lcd_printf20x20( 30 , 60, "确认修改");
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
    }
    void IR_Usercode_Menu_Operation(uint8_t key)
    {
        int num_key = Get_Num_Map(key);
        float indata;
        if(num_key != -1) // 有数字输入 第一个点不能是字符串输入
        {
            if(!InputMode  )
            {
                InputMode = 1; //  输入模式
                InputFlash_Timer = sys_timer_add(NULL,Mppt_Normal_InPutFlash,300);
                input_index = 0;
                memset(instr,0,sizeof(instr));
            }  
        }
        if(InputMode)
        {
            if(num_key != -1)
            {
                if(input_index<5)input_index++;
                instr[input_index - 1] = num_key + '0';
            }
            switch (key)
            {
                case KEY_VALUE_TYPE_BACKSPACE: // 退格
                    if(input_index)
                    {   
                        if(input_index == 1)instr[input_index - 1] = '0';
                        else instr[input_index - 1] = 0; 
                        input_index --;       
                    }
                    break;
                case KEY_VALUE_TYPE_ENTRE:
                    indata = string_to_float(instr);
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            if(indata>65535)indata = 65535; 
                            RoterData.Mppt_SetPara.Usercode = (u16)indata;
                            break;
                    }    
                    InputMode = 0;
                    //Mppt_ChargePara_Display(SetPara);
                    sys_timer_del(InputFlash_Timer);
                    IR_Usercode_Menu();
                    return;
                    break;
                case KEY_VALUE_TYPE_LEFT:
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    IR_Usercode_Menu();
                    return;
                    break;
                
            }
            Lcd_printf20x20( 30 , 30, "用户码设置:%s     ",instr);
             
            log_info("KEY %d NUMKRY %d input_index %d",key,num_key,input_index);
            log_info("instr %s value: %d",instr,(uint32_t)(string_to_float(instr)*100));

        }
        else  
        {
            switch(key)
            {
                
                case KEY_VALUE_TYPE_ENTRE:
                case KEY_VALUE_TYPE_RIGHT:
                    u8 data[4];
                    data[0] = 0xAA;data[1] = 0xbb;
                    data[2] = RoterData.Mppt_SetPara.Usercode >> 8;
                    data[3] = (u8)RoterData.Mppt_SetPara.Usercode;
                    Ir_tx_star_Def(data);
                    break;
            }
            Mppt_Normal_Menu_Select(key);
        }
        
    }

    void IR_CruveSet_Menu(void)
    {
         Lcd_Clear(BLACK);
        Mppt_DischarCurve_Display(&RoterData.Mppt_SetPara); 
    }

    void IR_CruveSet_Menu_Operation(uint8_t key)
    {
          Mppt_CurveSet_Operation(&RoterData.Mppt_SetPara,key);
    }

void Mppt_Version_Info_menu(void)
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 24 * 3, 0, "系统信息查询");
    Lcd_printf20x20(30, 60, "用户码:%d",RoterData.Mppt_SetPara.Usercode);
    Lcd_printf20x20(30, 90, "遥控器版本:1.0V");
    Lcd_printf20x20(30, 120, "出厂日期:2023:11:30");
}

void SYS_Set_Menu(void)
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 10 * 6, 0, "系统参数查询");
    Lcd_printf20x20(30, 30, "Power Auto Off: 120s");
    Lcd_printf20x20(30, 60, "Led Backlight: 100%%");
    Lcd_printf20x20(30, 90, "Beep: OFF");
    Lcd_printf20x20(30, 120, "BLE TX POWER: 3dB");
   // Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "＞");
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
    u8 cur_state =  ble_gatt_client_get_work_state();
    if(cur_state != 64 || MenuData.current_id == BL_ATCON_SET_MENU)return ;
    for (i = 0; i < 100; i++) //
    {
        if(RoterData.Ble_Adv_rp[i].useflag)
        {
            if (++RoterData.Ble_Adv_rp[i].Timeout > 30)
            {
                RoterData.Ble_Adv_rp[i].Timeout = 0;
                RoterData.Ble_Adv_rp[i].useflag = 0; // 清除使用标志
                RoterData.Ble_Adv_rp[i].rssi = -99;
                for(j=0;j<6;j++)RoterData.Ble_Adv_rp[i].mac[j] = 0;
                RoterData.Ble_Adv_Rp_Count -- ;
            }
        }
        
    }
}


