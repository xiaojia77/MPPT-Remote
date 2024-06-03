#include "lcd7789.h"
#include "lcd_data.h"
#include "asm/spi.h"

#include "key_driver.h"
#include "ble_multi.h"


#include "stdarg.h"
#include "stdlib.h"
#include "math.h"

#define log_info(x, ...) printf("[INFO]" x " ", ##__VA_ARGS__)

static void* spiflashdevice;    // FLASH�豸

// ��ʾ�ַ�����
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
}; // ��������ӳ���

MenuData_t MenuData = {0}; // 5���˵�

roter_t RoterData = {0};

void Lcd_SetFlashDevice(void *device)
{
    spiflashdevice = device;
}

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
    uint8_t temp; // ����
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
void Lcd_Show10x20(uint8_t x, uint8_t y, uint8_t *p)
{
    uint8_t i, j, k;
    uint8_t temp; // ����
  
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
void Lcd_Show16x16(uint8_t x, uint8_t y, uint8_t *p)
{
    uint8_t i, j, k;
    uint8_t temp; // ����
    Lcd_Address_Set(x, y, x + 15, y + 15);
    for (i = 1; i <= 32; i++)
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
    uint8_t temp; // ����
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

// void Lcd_printf20x20(uint8_t x, uint8_t y, uint8_t *format, ...) // must be string
// {
//     uint8_t SearchData[4];
//     char *str_data = malloc(128);
//     va_list ap;
//     va_start(ap, format);
//     vsnprintf(str_data, 128, format, ap);
//     char *str = str_data;
//     char *w;
//     while (*str)
//     {
//         if (*str & 0x80) // ����
//         {
//             SearchData[0] = str[0];
//             SearchData[1] = str[1];
//             SearchData[2] = str[2];
//             SearchData[3] = 0;
//             w = strstr(DotTbl24String, SearchData);
//             if (!w)break;
//             Lcd_Show20x20(x, y, DotTbl24[ (w - DotTbl24String)/3 ]);
//             x += 20;
//             str += 3;
//         }
//         else
//         {
//             SearchData[0] = str[0];
//             SearchData[1] = 0;
//             w = strstr(DotTbl24AsciiString, SearchData);
//             if (!w)break;
//             Lcd_Show10x20(x, y, DotTbl24Ascii[w - DotTbl24AsciiString]);
//             x += 10;
//             str++;
//         }
//     }
//     free(str_data);
// }

void Lcd_printf20x20(uint8_t x, uint8_t y, uint8_t *format, ...) // must be string
{
    uint8_t SearchData[4];
    char *str_data = malloc(128);
    char *fon_data = malloc(64);
    va_list ap;
    va_start(ap, format);
    vsnprintf(str_data, 128, format, ap);
    char *str = str_data;
    char *w;
    uint8_t BH ;
    uint8_t BL ;
    uint32_t py_add ;
    while (*str)
    {
        if (*str & 0x80) // ����
        {
            BH = str[0];
            BL = str[1];
            py_add=((BH-0xb0)*94+BL-0xa1)*60; 
            if(BH >= 0XD8)py_add -= 5*60;
            dev_bulk_read(spiflashdevice,fon_data,py_add,60);
            Lcd_Show20x20(x, y, fon_data);
            x += 20;
            str += 2;
        }
        else
        {
            SearchData[0] = str[0];
            SearchData[1] = 0;
            w = strstr(DotTbl24AsciiString, SearchData);
            if (!w)break;
            Lcd_Show10x20(x, y, DotTbl24Ascii[w - DotTbl24AsciiString]);
            x += 10;
            str++;
            
        }
    }
    free(str_data);
    free(fon_data);
}
 
void Lcd_printf16x16(uint8_t x, uint8_t y, uint8_t *str) // must be string
{
    uint8_t SearchData[4] = {0, 0, 0};
    char *w;
    uint8_t BH ;
    uint8_t BL ;
    uint32_t py_add ;
    while (*str)
    {
        if (*str & 0x80) // ����
        {
            /*SearchData[0] = str[0];
            SearchData[1] = str[1];
            SearchData[2] = str[2];
            SearchData[3] = 0;
            w = strstr(DotTbl24String, SearchData);
            if (!w)
                return;
            Lcd_Show10x20(x, y, DotTbl24[w - DotTbl24String]);
            x += 24;
            str += 3;*/
        //    BH = str[0];
        //    BL = str[1];
            BH = 0XBF;
            BL = 0XBF;
            py_add=((BH-0xb0)*94+BL-0xa1)*32;
          //  Lcd_Show16x16(x, y,&DotTbl16[py_add]);
            x += 16;
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
    uint8_t temp; // ����
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
    os_time_dly(20);
    gpio_write(LCD_RST, 1);
    os_time_dly(20);
    Lcd_WriteCmd(0x11); // �޴�ָ�����������ʼ��оƬ������ʾ
    os_time_dly(20);
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
    Lcd_WriteCmd(0xe7);  // SPI2 enable    ����2����ͨ��ģʽ
    Lcd_WriteData(0x00);

   
    //Lcd_WriteCmd(0x21); // Display inversion on
    Lcd_Clear(WHITE);
    Lcd_WriteCmd(0x29); // Display o
   
}

void floatToString(float d, int l,char *str) 
{

    int n = (int)d; //ȥ��С����

    int b = 0;

    int i,j;
    float m = d;//

    u8 len;

     // char *buf;
    char buf[15];

   //buf = malloc(128);
    itoa(n,str,10);
    i=strlen(str);

    
    str[i] = '.'; //С����
    str[i+1] = 0;
    len = i+1;
    while(d>0.99999f)
    {
        d /= 10.0f;
        b++;
    }

    for(i=0;i<l;i++)
    {
        m *= 10;//����
        if(!(int)m%10)
        {
            str[len] = '0';
            str[len+1] = 0;
            if(len<l   )len++;
        }
    }

    n = (int)m; //��������С����

     //log_info("floatToString%d B IS %d ", n , b);

    itoa(n,buf,10);//ת�����ַ��������ò���

    //log_info("floatToString%s %d %d ", buf ,strlen(str) , strlen(buf));

    for(i=b,j=strlen(str);i<strlen(buf);i++,j++)
    {
        str[j] = buf[i];
    }

    //log_info("floatToString%s %d %d ", str ,i , j);

    str[j] = 0;

    //log_info("floatToString%s %d %d ", str ,strlen(str) , strlen(buf)); 
         
    //free(buf);
} 

float stringtofloat(char *string)
{
	unsigned int i=0,j=0;
	unsigned char flag=0;  			//�ж������ŵı�־
	unsigned char flag_dot=1; 	//�ж�С����ı�־
	float num=0,data =0;     					//��ʱ�洢�������ı���
	
	for(i=0;string[i];i++) //ѭ��ֱ���ַ�����β
	{
		if(string[i]>='0'&&string[i]<='9'&&flag_dot==1)  //�����ǰ�ַ�Ϊ��������С����֮ǰ
		{
			if(j==0) num = num*pow(10,j)+(double)(string[i]-'0');     //���㲢�洢�м������
			else     num = num*pow(10,1)+(double)(string[i]-'0');
			j++;
		}
		else if(string[i]>='0'&&string[i]<='9'&&flag_dot==0) //�����ǰ�ַ�Ϊ��������С����֮��
		{
			num = num+(double)(string[i]-'0')*pow(0.1,j+1);     //���㲢�洢�м������
			j++;
		}
		else if(string[i]=='.')                               //������С�����򽫶�Ӧ��־λ��ֵ�ı�
		{
			flag_dot=0;
			j=0;
		}
		else if(string[i]=='-')                              //��������ͬ���ı��Ӧ��־λ��ֵ
		{
			flag = 1;
		}
		else if(string[i]==',')                             //����һ�����ݣ����ñ�־λ����¼���ռ�����
		{
			data = num*pow(-1,flag);
			flag = 0;
			flag_dot=1;
			j=0;
			num = 0;
		}
	}
	data = num*pow(-1,flag);                            //�������һ����
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

int Key_NumberMap(uint8_t key)
{
    uint8_t i;
    for(i=0;i<sizeof(Num_Map_Tab);i++)
    {
        if(key == Num_Map_Tab[i])return i;
    }
    return -1;
}


static void Lcd_SwitchWindows(uint8_t id)  // �л�����
{
   // Lcd_WriteCmd(0x28); // Display o
    MenuData.current_id = id;
    Menu_Tab[id].display_operation();
    //Lcd_WriteCmd(0x29); // Display o
}
static void Lcd_BackPreWindows()       //�����ϸ�����
{
    //Lcd_WriteCmd(0x28); // Display o
    MenuData.current_id = Menu_Tab[MenuData.current_id].up_id;
    Menu_Tab[MenuData.current_id].display_operation();
   // Lcd_WriteCmd(0x29); // Display o
}
static void Lcd_IndexDec()
{

}
static void Lcd_IndexInc()
{

}

void Mppt_Log_Menu(void)
{
   //Lcd_ShowPicture(0,0,240,240,gImage_image);
  //Lcd_printf20x20(0,0,"�������������ءآأ");
    // u16 i,j;
	// u32 k=0;
    // u8 length = 240;
    // u8 width = 240;
	// Lcd_Address_Set(0,0,0+length-1,0+width-1);
    // u8 data;
	// for(i=0;i<length;i++)
	// {
	// 	for(j=0;j<width;j++)
	// 	{
    //         dev_bulk_read(spiflashdevice,&data,405780+(k*2+1),1);
	// 		Lcd_WriteData(data);
    //         dev_bulk_read(spiflashdevice,&data,405780+(k*2),1);
    //         Lcd_WriteData(data);
	// 		k++;
	// 	}
	// }		

     u16 i,j;
	u32 k=0;
    u8 length = 240;
    u8 width  = 240;
    u8 *data   = malloc(512);
    Lcd_Address_Set(0,0,0+length-1,0+width-1);
	for(i=0;i<length;i++)
	{
        dev_bulk_read(spiflashdevice,data,405780+(i*480),480);
        k = 0;
		for(j=0;j<width;j++)
		{
			Lcd_WriteData(data[k*2+1]);
            Lcd_WriteData(data[k*2]);
			k++;
		}
	}		
    free(data);
   // Lcd_SwitchWindows(DISCHAR_CURVE_SET_MENU);
}
void Mppt_Log_MenuOps(uint8_t key)
{
    switch (key)
    {
        case KEY_VALUE_TYPE_ENTRE:
        case KEY_VALUE_TYPE_RIGHT: 
            Lcd_SwitchWindows (MAIN_MENU); 
            break;
    }   
}


void Mppt_Menu_Select_Display(void)
{
    uint8_t i;
    for (i = 1; i <= 7; i++)
        Lcd_Clear20x20(5, 30 * i);

    // �����±곤��
    if (!MenuData.index[MenuData.current_id])
        MenuData.index[MenuData.current_id] = 1;
    if (MenuData.index[MenuData.current_id] > 7)
        MenuData.index[MenuData.current_id] = 7;
    Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
}
void Mppt_Normal_Menu_Select(uint8_t key)
{
    switch (key)
    {
        case KEY_VALUE_TYPE_UP: // ��
            if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
            if (MenuData.index[MenuData.current_id] > 1)
                MenuData.index[MenuData.current_id]--;
            else
            {
                MenuData.index[MenuData.current_id] = Menu_Tab[MenuData.current_id].max_menu_num;
            }
                
            Mppt_Menu_Select_Display();
            break;
        case KEY_VALUE_TYPE_DOWN: // ��
            if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
            if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                MenuData.index[MenuData.current_id]++;
            else
                MenuData.index[MenuData.current_id] = 1;
            Mppt_Menu_Select_Display();
            break;
        case KEY_VALUE_TYPE_LEFT: // ��
            Lcd_BackPreWindows();
            break;
    }
}

void Mppt_MenuSelect_Dis(void)
{
    uint8_t i;
    for (i = 0; i < 10; i++)
        Lcd_Clear20x20(5, 24 * i);

    // �����±곤��
    if (!MenuData.index[MenuData.current_id])
        MenuData.index[MenuData.current_id] = 1;
    if (MenuData.index[MenuData.current_id] > 8)
        MenuData.index[MenuData.current_id] = 8;
    Lcd_printf20x20(5, 24 * MenuData.index[MenuData.current_id], "->");
}
void Mppt_Menu_Select(uint8_t key)
{
    switch (key)
    {
        case KEY_VALUE_TYPE_UP: // ��
            if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
            if (MenuData.index[MenuData.current_id] > 1)
                MenuData.index[MenuData.current_id]--;
            else
            {
                MenuData.index[MenuData.current_id] = Menu_Tab[MenuData.current_id].max_menu_num;
            }
                
            Mppt_MenuSelect_Dis();
            break;
        case KEY_VALUE_TYPE_DOWN: // ��
            if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
            if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                MenuData.index[MenuData.current_id]++;
            else
                MenuData.index[MenuData.current_id] = 1;
            Mppt_MenuSelect_Dis();
            break;
        case KEY_VALUE_TYPE_LEFT: // ��
            Lcd_BackPreWindows();
            break;
    }
}


void Mppt_Main_Menu(void) // ���˵�
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 20 * 4, 0, "MPPT̫���ܿ�����");
    Lcd_printf20x20(30, 24*1,  "��������������");
    Lcd_printf20x20(30, 24*2,  "��������Ϣ��ѯ");
    Lcd_printf20x20(30, 24*3,  "���ⳣ��ң����");
    Lcd_printf20x20(30, 24*4, "���⹤��ң����");
    Lcd_printf20x20(30, 24*5, "�����û�������");
    Lcd_printf20x20(30, 24*6, "ң������Ϣ��ѯ"); 
    Lcd_printf20x20(30, 24*7, "MPPT�汾����"); 
    Lcd_printf20x20(30, 24*8, "������������:%d",RoterData.Ble_Adv_Rp_Count,RoterData.SetCount);
    Lcd_printf20x20(30, 24*9, "��ǰ�汾:%s","SQ20P75SA-B "); 
    
    Lcd_printf20x20(5, 24 * MenuData.index[MenuData.current_id], "->"); 
    // Lcd_printf20x20(30, 210, "��ǰ�汾:%s","SQ20P75SA-B "); 
    /*Lcd_printf20x20(30, 90, "�����������");
    Lcd_printf20x20(30, 120, "ϵͳ��Ϣ��ѯ");
    Lcd_printf20x20(30, 150, "ϵͳ������ѯ");
    Lcd_printf20x20(30, 180, "MPPT�汾����"); 
    Lcd_printf20x20(30, 210, "��ǰ�汾:%s","SQ20P75SA-B "); 
    Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");*/
}
void Mppt_Main_Menu_Operation(uint8_t key)
{
    Mppt_Menu_Select(key);
    switch (key)
    {
        case KEY_VALUE_TYPE_ENTRE:
        case KEY_VALUE_TYPE_RIGHT:      
            switch (MenuData.index[MenuData.current_id])
            {
                case 1:
                    Lcd_SwitchWindows(DISCHAR_CURVE_SET_MENU);
                    break;
                case 2:
                    Lcd_SwitchWindows(BL_CON_MENU);
                    break;
                case 3:
                    Lcd_SwitchWindows(IR_NORMAL_MENU);
                    break;
                case 4:
                    Lcd_SwitchWindows(IR_ENGINEER_MENU);
                    break;
                case 5:
                    Lcd_SwitchWindows(IR_USERCODE_MENU);
                    break;
                case 6:
                    Lcd_SwitchWindows(VERSION_CHECK_MENU);
                    break;
                case 7:
                    Lcd_SwitchWindows(MPPT_VERSION_SELECT_MENU);
                    break;
            }                                      
            break;
    }
}

const char *Curve_Mode_Str[]={"PWM ","����","AI  "};
const char *Connect_Mode_Str[]={"IO","����"}; 
const char *AotoSetStr[]={"��","��"};
const char *Lock_Mode_Str[]={"��","��"}; 
const char *SolarModeStr[]={"�Զ�����","�ֶ�����"};
const char *Dischar_Curve_Str[]={"PWM","Cur","AI"};

void Mppt_Ble_BatchSet_Display()
{
    Lcd_printf20x20(120 - 20 * 3, 0, "������������");
    Lcd_printf20x20(30, 30, "����������");
    /*Lcd_printf20x20(30, 60, "�ŵ��������");
    Lcd_printf20x20(30, 90, "�ŵ�����ģʽ��%s",Curve_Mode_Str[RoterData.Mppt_SetPara.DischarCurve_Moed]);
    Lcd_printf20x20(30, 120,"�ŵ���������");
    Lcd_printf20x20(30, 150,"����ģʽ��%s",Lock_Mode_Str[RoterData.Mppt_SetPara.Lock_Mode]);
    Lcd_printf20x20(30, 180,"��������");*/
    Lcd_printf20x20(5, 30 * MenuData.index[BLE_BATCHSET_MENU], "->");
}
void Mppt_Ble_BatchSet_Menu(void)
{
    Lcd_Clear(BLACK);
    Mppt_Ble_BatchSet_Display();
}
void Mppt_Ble_BatchSet_MenuOps(uint8_t key)
{
    Mppt_Normal_Menu_Select(key);
    switch (key)
    {
        case KEY_VALUE_TYPE_ENTRE:
        case KEY_VALUE_TYPE_RIGHT: // 
                switch (MenuData.index[MenuData.current_id])
                {
                    case 1:
                        MenuData.current_id = CHAEGE_SET_MENU; // �л����¼��˵�
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                                Menu_Tab[MenuData.current_id].display_operation();
                        return ;
                         break;
                    case 2:
                        MenuData.current_id = DISCHAR_SET_MENU; // �л����¼��˵�
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                             Menu_Tab[MenuData.current_id].display_operation();
                        return ;
                        break;
                    case 3:
                        if(++RoterData.Mppt_SetPara.DischarCurve_Moed>2)RoterData.Mppt_SetPara.DischarCurve_Moed = 0;
                        Mppt_Ble_BatchSet_Display();
                        break;
                    case 4:
                        MenuData.current_id = DISCHAR_CURVE_SET_MENU; // �л����¼��˵�
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                             Menu_Tab[MenuData.current_id].display_operation();
                         return ;
                    break;
                    case 5:
                        RoterData.Mppt_SetPara.Lock_Mode = !RoterData.Mppt_SetPara.Lock_Mode;
                        Mppt_Ble_BatchSet_Display();
                        break;
                    case 6:
                        MenuData.current_id = BL_ATCON_SET_MENU; // �л����¼��˵�
                        if (Menu_Tab[MenuData.current_id].display_operation != NULL)
                             Menu_Tab[MenuData.current_id].display_operation();
                         return ;
                    break;
                }
               // Mppt_DischarPara_Dis();
            break;
        case KEY_VALUE_TYPE_DECRE:    
               // Mppt_DischarPara_Dis();
            break;
    }   
}

    void Mppt_InPutFlash(void)
    {
        static u8 flash = 1;
        flash = !flash;
        if(flash)Lcd_printf20x20(5, 24 * MenuData.index[MenuData.current_id], "->");
        else Lcd_printf20x20(5, 24 * MenuData.index[MenuData.current_id], "  ");
    }
    void Mppt_Normal_InPutFlash(void)
    {
        static u8 flash = 1;
        flash = !flash;
        if(flash)Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
        else Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "  ");
    }
    void Mppt_ChargePara_Display(Mppt_Set_Parm_t *SetPara)
    {
        char str[10] = ""; 
        Lcd_printf20x20(120 - 24 * 3, 0, "����������");
        floatToString(SetPara->Bat_Capcity,1,str);
        Lcd_printf20x20(30, 30,"�������:%sAh  ",str);
        floatToString(SetPara->Charge_Current_Max,1,str);
        Lcd_printf20x20(30,60,"������:%sA    ",str);
        floatToString(SetPara->Charge_Power_Max,1,str);
        Lcd_printf20x20(30,90,"�����:%sw    ",str);
        floatToString(SetPara->Trickle_Current,1,str);
      //  Lcd_printf20x20(30,120,"�������:%sA   ",str);

    }
    void Mppt_ChargeSet_Menu(void)
    {
        Lcd_Clear(BLACK);
        MenuData.index[MenuData.current_id] = 1;
        Mppt_ChargePara_Display(&RoterData.Mppt_SetPara);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
        Lcd_printf20x20(30,120,"��һҳ");
        Lcd_Show20x20(30+60,120,SpecialDot[0]);
    }
    void Mppt_ChargeSet_Ops(Mppt_Set_Parm_t *SetPara,uint8_t key)
    {
        int num_key = Key_NumberMap(key);
        float indata;
        if(num_key != -1) // ���������� ��һ���㲻�����ַ�������
        {
            if(!InputMode)
            {
                InputMode = 1; //  ����ģʽ
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

                case KEY_VALUE_TYPE_DOT: // ����С����
                    if(input_index<5)input_index++;
                    instr[input_index - 1] = '.';
                    instr[input_index] = 0;
                    break;
                
                case KEY_VALUE_TYPE_BACKSPACE: // �˸�
                    if(input_index)
                    {   
                        if(input_index == 1)instr[input_index - 1] = '0';
                        else instr[input_index - 1] = 0; 
                        input_index --;       
                    }
                    break;
                case KEY_VALUE_TYPE_ENTRE:
                    indata = stringtofloat(instr);
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

            float input_num = stringtofloat(instr);
            if(MenuData.index[MenuData.current_id] == 1)
            {
                if( input_num > 100)
                {
                    strcpy(instr,"100"); 
                    input_index = sizeof("100") - 1;
                }
                Lcd_printf20x20(30, 30,"�������:%sAh    ",instr);
               
            }
            else if(MenuData.index[MenuData.current_id] == 2)
            {
                if( input_num > 25)
                {
                    strcpy(instr,"25"); 
                    input_index = sizeof("25") - 1;
                }
                Lcd_printf20x20(30, 60,"������:%sA    ",instr);
            }
            if(MenuData.index[MenuData.current_id] == 3)
            {
                if( input_num > 100)
                {
                    strcpy(instr,"100"); 
                    input_index = sizeof("100") - 1;
                }
                Lcd_printf20x20(30, 90,"�����:%sw    ",instr);
            }
            if(MenuData.index[MenuData.current_id] == 4)
            {
                if( input_num > 5)
                {
                    strcpy(instr,"5"); 
                    input_index = sizeof("5") - 1;
                }
                Lcd_printf20x20(30, 120,"�������:%sA   ",instr);
            }
             
            log_info("KEY %d NUMKRY %d input_index %d",key,num_key,input_index);
            log_info("instr %s value: %d",instr,(uint32_t)(input_num*100));

        }
        else
        {
            //Mppt_Normal_Menu_Select(key);
            switch (key)
            {
                case KEY_VALUE_TYPE_INCRE: // ��
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
    void Mppt_ChargeSet_MenuOps(uint8_t key)
    {
        Mppt_ChargeSet_Ops(&RoterData.Mppt_SetPara,key);
        if(InputMode)return ;
        switch (key)
        {
            case KEY_VALUE_TYPE_UP: // ��
                if (MenuData.index[MenuData.current_id] > 1)
                    MenuData.index[MenuData.current_id]--;
                else{
                    Lcd_BackPreWindows();
                    return ;
                }
                Mppt_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_DOWN: // ��
                if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                    MenuData.index[MenuData.current_id]++;
                else{
                     Lcd_SwitchWindows(BL_ATCON_SET_MENU);
                     return ;
                }
                Mppt_Menu_Select_Display();
                break;
            case KEY_VALUE_TYPE_LEFT: // ��
                Lcd_BackPreWindows();
                break;
        }
    }
  
    void Mppt_DischarPara_Dis(Mppt_Set_Parm_t *SetPara)
    {   
        char str[7] = "";      
        Lcd_printf20x20(120 - 24 * 3, 0, "�ŵ��������"); 
        floatToString(SetPara->Low_voltage_Protect,2,str);
        Lcd_printf20x20(30, 24*1, "��ص�ѹ����:%sV    ",str);
        Lcd_printf20x20(30, 24*2, "�ŵ������λ:%d     ",SetPara->Current_Gear);
        // Lcd_printf20x20(30, 24*3, "�״��Ӧ����:%d%%   ",SetPara->Ledar_Pwm);
        // Lcd_printf20x20(30, 24*4, "�״��ӳ�ʱ��:%dS   ",SetPara->Ledar_Dly_Time);
        // Lcd_printf20x20(30, 24*5, "�״�����ʱ��:%dH   ",SetPara->Ledar_Dly_On);
        // Lcd_printf20x20(30, 24*6, "�״���������:%s   ",AotoSetStr[SetPara->Ledar_Dly_IsOn]);
        Lcd_printf20x20(30, 24*3, "���ģʽ:%s   ",SolarModeStr[SetPara->Solar_Mode]);
        Lcd_printf20x20(30, 24*4, "ͨ��ģʽ:%s   ",Connect_Mode_Str[SetPara->Extern_Mode]);
       
    }
    void Mppt_DischarSet_Menu(void)
    {
        Lcd_Clear(BLACK);
        MenuData.index[MenuData.current_id] = 1;
        Mppt_DischarPara_Dis(&RoterData.Mppt_SetPara);
        Lcd_printf20x20(5, 24 * MenuData.index[MenuData.current_id], "->");
        Lcd_printf20x20(30, 24*9, "��һҳ");
        Lcd_Show20x20(90,24*9,SpecialDot[0]);
    }
    void Mppt_DischarSet_Ops(Mppt_Set_Parm_t *SetPara,uint8_t key)
    {
        int num_key = Key_NumberMap(key);
        float indata; 
        if(num_key != -1) // ���������� ��һ���㲻�����ַ�������
        {
            if(!InputMode && MenuData.index[MenuData.current_id]<=2)
            {
                InputMode = 1; //  ����ģʽ
                InputFlash_Timer = sys_timer_add(NULL,Mppt_InPutFlash,300);
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

                case KEY_VALUE_TYPE_DOT: // ����С����
                    if(input_index<6)input_index++;
                    instr[input_index - 1] = '.';
                    break;
                
                case KEY_VALUE_TYPE_BACKSPACE: // �˸�
                    if(input_index)
                    {   
                        if(input_index == 1)instr[input_index - 1] = '0';
                        else instr[input_index - 1] = 0; 
                        input_index --;       
                    }
                    break;
                case KEY_VALUE_TYPE_ENTRE:
                    indata = stringtofloat(instr);
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:          
                            if(indata<2.4)indata = 2.4;
                            if(indata>2.851)indata = 2.85;
                            SetPara->Low_voltage_Protect=indata;
                            break;
                        case 2:
                            if(indata<1)indata = 1;
                            if(indata>20)indata = 20;
                            SetPara->Current_Gear=indata;
                            break;
                    }    
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_DischarPara_Dis(SetPara);
                    return;
                    break;       
                 case KEY_VALUE_TYPE_LEFT:
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_DischarPara_Dis(SetPara);
                    return;
                    break;
            }
            
            float input_num = stringtofloat(instr);
            if(MenuData.index[MenuData.current_id] == 1)
            {
                if( input_num > 2.85)
                {
                    strcpy(instr,"2.85"); 
                    input_index = sizeof("2.85") - 1;
                }
                Lcd_printf20x20(30, 24,"��ص�ѹ����:%sV   ",instr);
            }
            else if(MenuData.index[MenuData.current_id] == 2)
            {
                if( input_num > 20)
                {
                    strcpy(instr,"20"); 
                    input_index = sizeof("20") - 1;
                }
                Lcd_printf20x20(30, 24*2,"�ŵ������λ:%s     ",instr);
            }
        }
        else
        {
            //Mppt_Normal_Menu_Select(key);
            switch (key)
            {
                
                case KEY_VALUE_TYPE_RIGHT:
                    if(MenuData.index[MenuData.current_id] == 3)
                    {

                        if(SetPara->Solar_Mode) SetPara->Solar_Mode= 0;
                        else SetPara->Solar_Mode = 1;
                        Mppt_DischarPara_Dis(SetPara);
                    }       
                    else if(MenuData.index[MenuData.current_id] == 4)
                    {
                        if(SetPara->Extern_Mode) SetPara->Extern_Mode = 0;
                        else SetPara->Extern_Mode = 1;
                       
                        Mppt_DischarPara_Dis(SetPara);
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

                        }
                        Mppt_DischarPara_Dis(SetPara);
                    break;
                case KEY_VALUE_TYPE_DECRE:
                        switch (MenuData.index[MenuData.current_id])
                        {
                            case 1:
                                SetPara->Low_voltage_Protect -= 0.0501;
                                if(SetPara->Low_voltage_Protect<2.4)SetPara->Low_voltage_Protect = 2.4;
                                break;
                            case 2:
                                if(--SetPara->Current_Gear <= 0) SetPara->Current_Gear = 20;
                                break;
                        }
                        Mppt_DischarPara_Dis(SetPara);
                    break;
            }   
        }
    }
    void Mppt_DischarSet_MenuOps(uint8_t key)
    {
        Mppt_DischarSet_Ops(&RoterData.Mppt_SetPara,key);
        if(InputMode)return ;
        switch (key)
        {
            case KEY_VALUE_TYPE_UP: // ��
                if (MenuData.index[MenuData.current_id] > 1)
                    MenuData.index[MenuData.current_id]--;
                else
                {
                    Lcd_BackPreWindows();
                    return ;
                }      
                //Mppt_Menu_Select_Display();
                Mppt_MenuSelect_Dis();
                break;
            case KEY_VALUE_TYPE_DOWN: // ��
                if (MenuData.index[MenuData.current_id] < 4)
                    MenuData.index[MenuData.current_id]++;
                else{
                    Lcd_SwitchWindows(RADARPARA_SET_MENU);
                    return ;
                }
                //Mppt_Menu_Select_Display();
                Mppt_MenuSelect_Dis();
                break;
            case KEY_VALUE_TYPE_LEFT: // ��
                Lcd_BackPreWindows();
                break;
        }
    }

    void Mppt_RadarPara_Dis(Mppt_Set_Parm_t *SetPara)
    {
        char str[7] = "";  
        Lcd_printf20x20(120 - 24 * 3, 0, "�״��������"); 
        Lcd_printf20x20(30, 24*1, "�״��Ӧ����:%d%%   ",SetPara->Ledar_Pwm);
        Lcd_printf20x20(30, 24*2, "�״��ӳ�ʱ��:%dS   ",SetPara->Ledar_Dly_Time);
        floatToString(SetPara->Ledar_Dly_On,1,str);
        Lcd_printf20x20(30, 24*3, "�״�����ʱ��:%sH   ",str);
        Lcd_printf20x20(30, 24*4, "�״�������λ:%d   ",SetPara->Ledar_sens);
        Lcd_printf20x20(30, 24*5, "�״���������:%s   ",AotoSetStr[SetPara->Ledar_Dly_IsOn]);
    }
    void Mppt_RadarPara_Menu(void)
    {
        Lcd_Clear(BLACK);
        MenuData.index[MenuData.current_id] = 1;
        Mppt_RadarPara_Dis(&RoterData.Mppt_SetPara);
        Lcd_printf20x20(5, 24 * MenuData.index[MenuData.current_id], "->");
        Lcd_printf20x20(30, 24*9, "��һҳ");
        Lcd_Show20x20(90,24*9,SpecialDot[0]);
    }
    void Mppt_RadarPara_Ops(Mppt_Set_Parm_t *SetPara,uint8_t key)
    {
        int num_key = Key_NumberMap(key);
        float indata; 
        if(num_key != -1) // ���������� ��һ���㲻�����ַ�������
        {
            if(!InputMode && MenuData.index[MenuData.current_id]<=5)
            {
                InputMode = 1; //  ����ģʽ
                InputFlash_Timer = sys_timer_add(NULL,Mppt_InPutFlash,300);
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

                case KEY_VALUE_TYPE_DOT: // ����С����
                    if(input_index<6)input_index++;
                    instr[input_index - 1] = '.';
                    break;
                
                case KEY_VALUE_TYPE_BACKSPACE: // �˸�
                    if(input_index)
                    {   
                        if(input_index == 1)instr[input_index - 1] = '0';
                        else instr[input_index - 1] = 0; 
                        input_index --;       
                    }
                    break;
                case KEY_VALUE_TYPE_ENTRE:
                    indata = stringtofloat(instr);
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            if(indata<0)indata = 0;
                            if(indata>30)indata = 30;
                            SetPara->Ledar_Pwm=indata;
                            break;
                        case 2:
                            if(indata<0)indata = 0;
                            if(indata>60)indata = 60;
                            SetPara->Ledar_Dly_Time=indata;
                            break; 
                        case 3:
                            if(indata<0)indata = 0;
                            if(indata>10)indata = 10;
                            SetPara->Ledar_Dly_On=indata;
                            break;   
                        case 4:
                            if(indata<0)indata = 0;
                            if(indata>10)indata = 10;
                            SetPara->Ledar_sens=indata;
                            break;   
                    }    
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_RadarPara_Dis(SetPara);
                    return;
                    break;       
                 case KEY_VALUE_TYPE_LEFT:
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_RadarPara_Dis(SetPara);
                    return;
                    break;
            }
            
            float input_num = stringtofloat(instr);      
            if(MenuData.index[MenuData.current_id] == 1)
            {
                if( input_num > 30)
                {
                    strcpy(instr,"30"); 
                    input_index = sizeof("30") - 1;
                }
                Lcd_printf20x20(30, 24*1,"�״��Ӧ����:%s%%    ",instr);
            }
            else if(MenuData.index[MenuData.current_id] == 2)
            {
                if( input_num > 60)
                {
                    strcpy(instr,"60"); 
                    input_index = sizeof("60") - 1;
                }
                Lcd_printf20x20(30, 24*2,"�״��ӳ�ʱ��:%sS   ",instr);
            }
            else if(MenuData.index[MenuData.current_id] == 3)
            { 
                if( input_num > 10)
                {
                    strcpy(instr,"10"); 
                    input_index = sizeof("10") - 1;
                }
                Lcd_printf20x20(30, 24*3,"�״�����ʱ��:%sh   ",instr);
            }        
            else if(MenuData.index[MenuData.current_id] == 4)
            { 
                if( input_num > 10)
                {
                    strcpy(instr,"10"); 
                    input_index = sizeof("10") - 1;
                }
                Lcd_printf20x20(30, 24*4,"�״�������λ:%s   ",instr);
            }        
            log_info("KEY %d NUMKRY %d input_index %d",key,num_key,input_index);
            log_info("instr %s  value: %d",instr,(uint32_t)(stringtofloat(instr)*100));

        }
        else
        {
            switch (key)
            {
                
                case KEY_VALUE_TYPE_RIGHT:
                    if(MenuData.index[MenuData.current_id] == 5)
                    {
                        if(SetPara->Ledar_Dly_IsOn) SetPara->Ledar_Dly_IsOn = 0;
                        else SetPara->Ledar_Dly_IsOn = 1;
                       
                        Mppt_RadarPara_Dis(SetPara);
                    }                
                    break;
                case KEY_VALUE_TYPE_INCRE: // 
                        switch (MenuData.index[MenuData.current_id])
                        {
                            case 1:
                                if(++SetPara->Ledar_Pwm > 30) SetPara->Ledar_Pwm = 10;
                                break;                          
                            case 2:
                                if(++SetPara->Ledar_Dly_Time>15)SetPara->Ledar_Dly_Time = 5;
                                break;      
                            case 3:
                                if(++SetPara->Ledar_Dly_On>10)SetPara->Ledar_Dly_On = 1;
                                break;
                            case 4:
                                if(++SetPara->Ledar_sens>10)SetPara->Ledar_sens = 1;
                                break;
                        }
                        Mppt_RadarPara_Dis(SetPara);
                    break;
                case KEY_VALUE_TYPE_DECRE:
                        switch (MenuData.index[MenuData.current_id])
                        {

                            case 1:
                                if(--SetPara->Ledar_Pwm < 10) SetPara->Ledar_Pwm = 30;
                                break;
                            case 2:
                                if(--SetPara->Ledar_Dly_Time<5)SetPara->Ledar_Dly_Time = 15;
                                break;      
                            case 3:
                                if(--SetPara->Ledar_Dly_On<=0)SetPara->Ledar_Dly_On = 10;
                                break;     
                            case 4:
                                if(--SetPara->Ledar_sens<=0)SetPara->Ledar_sens = 10;
                                break;
                        }
                        Mppt_RadarPara_Dis(SetPara);
                    break;
            }   
        }
    }
    void Mppt_RadarPara_MenuOps(uint8_t key)
    {
        Mppt_RadarPara_Ops(&RoterData.Mppt_SetPara,key);
        if(InputMode)return ;
        switch (key)
        {
            case KEY_VALUE_TYPE_UP: // ��
                if (MenuData.index[MenuData.current_id] > 1)
                    MenuData.index[MenuData.current_id]--;
                else
                {
                    Lcd_BackPreWindows();
                    return ;
                }      
                Mppt_MenuSelect_Dis();
                break;
            case KEY_VALUE_TYPE_DOWN: // ��
                if (MenuData.index[MenuData.current_id] < 5)
                    MenuData.index[MenuData.current_id]++;
                else{
                    Lcd_SwitchWindows(CHAEGE_SET_MENU);
                    return ;
                }
                Mppt_MenuSelect_Dis();
                break;
            case KEY_VALUE_TYPE_LEFT: // ��
                Lcd_BackPreWindows();
                break;
        }
    }


    void Mppt_CurceSet_Menu(void)
    {
        Lcd_Clear(BLACK);
        MenuData.index[MenuData.current_id] = 1;
        Lcd_printf20x20(120 - 20 * 3, 0, "���߲�������");
        Lcd_printf20x20(30, 30, "�ŵ�����ģʽ:%s",Curve_Mode_Str[RoterData.Mppt_SetPara.DischarCurve_Moed]);
        Lcd_printf20x20(30, 60,"�ŵ���������");
        Lcd_printf20x20(30, 90,"��һҳ");
        Lcd_Show20x20(30+60,90,SpecialDot[0]);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
    }
    void Mppt_CurceSet_MenuOps(uint8_t key)
    {
        uint8_t id = MenuData.current_id;
        switch (key)
        {
            case KEY_VALUE_TYPE_UP: // ��
                if (MenuData.index[id] > 1)
                    MenuData.index[id]--;
                else
                {
                    Lcd_BackPreWindows();
                    return ;
                }               
                Mppt_Menu_Select_Display();
                break;

            case KEY_VALUE_TYPE_DOWN: // ��
                if(!Menu_Tab[id].max_menu_num)return;
                if (MenuData.index[id] < Menu_Tab[id].max_menu_num)
                    MenuData.index[id]++;
                else{
                    Lcd_SwitchWindows(BL_ATCON_SET_MENU);
                    return ;
                }
                Mppt_Menu_Select_Display();
                break;

            case KEY_VALUE_TYPE_LEFT: // ��
                Lcd_BackPreWindows();
                return ;
                break;

            case KEY_VALUE_TYPE_RIGHT: 
            case KEY_VALUE_TYPE_ENTRE:
                switch (MenuData.index[id])
                {  
                    case 1:
                        if(++RoterData.Mppt_SetPara.DischarCurve_Moed>2)RoterData.Mppt_SetPara.DischarCurve_Moed = 0;
                        Mppt_CurceSet_Menu();
                        break;      
                    case 2:
                        Lcd_SwitchWindows(DISCHAR_CURVE_SET_MENU);
                        return ;
                        break;
                }  
                break;
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
                 Lcd_printf20x20(30 + 10*10, 20 * (MenuData.index[MenuId]-8) + 30, "->");
            else
                Lcd_printf20x20(0, 20 * MenuData.index[MenuId] + 30, "->");
        }
        else
        {
            if( MenuData.index[MenuData.current_id] / 9 )
                 Lcd_printf20x20(30 + 10*10, 20 * (MenuData.index[MenuId]-8) + 30, "  ");
            else
                Lcd_printf20x20(0, 20 * MenuData.index[MenuId] + 30, "  ");
                
        }
    }
    void Mppt_Curve_MenuIndexDis(void)
    {
        uint8_t i;
        uint8_t MenuId = MenuData.current_id;
        for (i = 1; i <= 8; i++)
        {
              Lcd_Clear20x20(0, 20 * i + 30);
              Lcd_Clear20x20(30 + 10*10, 20 * i + 30);
        }

        //�����±곤��
        //if (!MenuData.index[MenuId])MenuData.index[MenuId] = 1;        
        //if (MenuData.index[MenuId] >= 16)MenuData.index[MenuId] = 16;
        Lcd_printf20x20(0, 25,"  ");
        if(!MenuData.index[MenuId])
        {
            Lcd_printf20x20(0, 25,"->");
        }
        else
        {
            if( MenuData.index[MenuId] / 9 )
                Lcd_printf20x20(30 + 10*10, 20 * (MenuData.index[MenuId]-8) + 30, "->");
            else
                Lcd_printf20x20(0, 20 * MenuData.index[MenuId] + 30, "->");
        }
        
    }
    void Mppt_DischarCurve_Display(Mppt_Set_Parm_t *Para)
    {
        uint8_t i;
        uint8_t str[10];
        float (*CurvData)[2] = Para->Curv_Data;
        float avg;
        const char *Dischar_Curve_Str[]={"PWM","Cur","AI"};
        //const char *Str1[]={"һ","��","��","��","��","��","��","��"};
        const char *Str1[]={"1","2","3","4","5","6","7","8"};
        Lcd_printf20x20(120 - 20 * 3, 0, "�ŵ���������");
        Lcd_printf20x20(30, 25, "�ŵ�����ģʽ:%s",Dischar_Curve_Str[Para->DischarCurve_Moed]); 
        if(Para->DischarCurve_Moed < 2)
        {     
            for(i=0;i<8;i++)
            {    
                floatToString(CurvData[i][0],1,str);
                Lcd_printf20x20(30,20*(i+1)+ 30,"��%s %sH ",Str1[i],str);
                Lcd_printf20x20(30 + 12*10, 20*(i+1)+ 30,"%s:%d%%  ",Dischar_Curve_Str[Para->DischarCurve_Moed],(uint32_t)CurvData[i][1]);
            }
            avg = 0;
            avg += CurvData[0][0] * (CurvData[0][1] /100);
            for(i=1;i<8;i++)
            {    
                if(fabs(CurvData[i][0]) < 1e-6)break;
                avg += (CurvData[i][0] - CurvData[i-1][0]) * (CurvData[i][1] /100);
            }
            floatToString(avg,1,str);
            Lcd_printf20x20(140,210,"ϵ��:%s",str);
        }
        else{
             Lcd_printf20x20(10, 80 , "AIģʽ�Ե�ص������ο�"); 
             Lcd_printf20x20(40, 100 , "�Զ������ŵ�����");  
             Lcd_printf20x20(60, 120 , "�ӳ��ŵ�ʱ��"); 
             //Lcd_printf20x20(0, 140 , "����->�л��Զ�������"); 
        }
        Lcd_printf20x20(30, 210, "��һҳ");
        Lcd_Show20x20(30+60,210,SpecialDot[0]);
    }
    void Mppt_DischarCurveSet_Menu(void)
    {   
        Lcd_Clear(BLACK);
        MenuData.index[MenuData.current_id] = 0;
        Mppt_Curve_MenuIndexDis();
        Mppt_DischarCurve_Display(&RoterData.Mppt_SetPara); 
    }
    void Mppt_CurveSet_Ops(Mppt_Set_Parm_t *Para,uint8_t key)
    {
        float (*CurvData)[2] = Para->Curv_Data;
        char *Curvestr = Dischar_Curve_Str[Para->DischarCurve_Moed];
        uint8_t Menu_Id = MenuData.current_id; // ��ǰID��
        uint8_t *Menu_Index = &MenuData.index[Menu_Id];
        uint8_t data_index;
        int num_key = Key_NumberMap(key);
        float indata;

        if(num_key != -1 && *Menu_Index) // ���������� ��һ���㲻�����ַ�������
        {
            if(!InputMode && *Menu_Index<=16)
            {
                InputMode = 1; //  ����ģʽ
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

                case KEY_VALUE_TYPE_DOT: // ����С����
                    if(input_index<4)input_index++;
                    instr[input_index - 1] = '.';
                    break;
                
                case KEY_VALUE_TYPE_BACKSPACE: // �˸�
                    if(input_index)
                    {   
                        if(input_index == 1)instr[input_index - 1] = '0';
                        else instr[input_index - 1] = 0; 
                        input_index --;       
                    }
                    break;
                case KEY_VALUE_TYPE_ENTRE:
                    indata = stringtofloat(instr);
                    data_index = (*Menu_Index-1)%8;  
                    if((*Menu_Index)/9)
                    {
                        CurvData[data_index][1] = indata;
                    }
                    else
                    {
                        //ǰ������ݲ��ܴ��ں��治����0������
                        if( data_index !=8 && indata >= CurvData[data_index+1][0] && (fabs(CurvData[data_index+1][0])>1e-6))
                            indata = CurvData[data_index][0];

                        CurvData[data_index][0] = indata;
                        if(fabs(indata) < 1e-6) //����������0 ��պ��������
                        {
                            for(uint8_t i=data_index;i<=8;i++)
                            {
                                CurvData[i][0] = 0;
                                CurvData[i][1] = 0;
                            }           
                        }

                    }      
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_DischarCurve_Display(Para);
                    Mppt_Curve_MenuIndexDis();
                    return;
                    break;
                 case KEY_VALUE_TYPE_LEFT:
                    InputMode = 0;
                    sys_timer_del(InputFlash_Timer);
                    Mppt_DischarCurve_Display(Para);
                    return;
                    break;
            }
             
            float input_num = stringtofloat(instr);
            log_info("KEY %d NUMKRY %d input_index %d",key,num_key,input_index);
            log_info("instr %s  value: %d",instr,(uint32_t)(input_num*100)); //�Ŵ�С���� ���Ƿ��д���

            if( (*Menu_Index/9) == 0)
            {
                if( input_num > 24)
                {
                    strcpy(instr,"24"); 
                    input_index = sizeof("24") - 1;
                }    
                Lcd_printf20x20(30,20*(*Menu_Index) + 30,"��%d %sH   ",*Menu_Index,instr);
            }
            else
            {
                if( input_num > 100)
                {
                    strcpy(instr,"100"); 
                    input_index = sizeof("100") - 1;
                }    
                Lcd_printf20x20(30 + 12*10, 20*(*Menu_Index - 8) + 30,"%s:%s%%   ",Curvestr,instr);
            }
               
        }
        else
        {
            switch (key)
            {
                case KEY_VALUE_TYPE_LEFT: // ��    
                    if( ( *Menu_Index / 9 ) && Para->DischarCurve_Moed <= 1)
                    {
                        *Menu_Index -= 8;
                        Mppt_Curve_MenuIndexDis();
                    }
                    else
                    {
                        Lcd_BackPreWindows();
                    }
                    break;
                case KEY_VALUE_TYPE_RIGHT:
                    if(*Menu_Index == 0)
                    {
                        Para->DischarCurve_Moed+=1;
                        if(Para->DischarCurve_Moed>2)Para->DischarCurve_Moed= 0;
                        Lcd_Clear(BLACK);
                        Mppt_Curve_MenuIndexDis();
                        Mppt_DischarCurve_Display(Para); 
                        return;
                    }
                    
                    if(Para->DischarCurve_Moed > 1)return;
                    if( ( *Menu_Index / 9 ) == 0)
                    {
                        *Menu_Index += 8; 
                        Mppt_Curve_MenuIndexDis();
                    }
                    break;
                case KEY_VALUE_TYPE_INCRE:
                    if( ( *Menu_Index / 9 ) == 0)
                    {
                        data_index =  *Menu_Index -1;
                        // 1 - 8  ǰ��ҪС�ں��� || �����<0.1  
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
    void Mppt_DischarCurveSet_MenuOps(uint8_t key)
    {   
        uint8_t Menu_Id = MenuData.current_id; // ��ǰID��
        uint8_t *Menu_Index = &MenuData.index[Menu_Id];
        Mppt_Set_Parm_t *Para = &RoterData.Mppt_SetPara;
        Mppt_CurveSet_Ops(Para,key);
        if(InputMode)return ;
        switch (key)
        {
            case KEY_VALUE_TYPE_UP: // ��
                if(*Menu_Index == 0)
                {
                    Lcd_BackPreWindows();
                    return;
                }
                if(Para->DischarCurve_Moed > 1)return;
                if(!Menu_Tab[Menu_Id].max_menu_num)return;
                if (*Menu_Index )*Menu_Index-=1;    
                else *Menu_Index = Menu_Tab[Menu_Id].max_menu_num;                
                Mppt_Curve_MenuIndexDis();
                break;
            case KEY_VALUE_TYPE_DOWN: // ��
                if(Para->DischarCurve_Moed > 1)
                {
                    Lcd_SwitchWindows(DISCHAR_SET_MENU);
                    return;
                }
                if(!Menu_Tab[Menu_Id].max_menu_num)return;
                if (*Menu_Index < Menu_Tab[Menu_Id].max_menu_num)
                {
                    if(*Menu_Index+1 == 9 || *Menu_Index+1 == 17 )
                    {
                        Lcd_SwitchWindows(DISCHAR_SET_MENU);
                        return;
                    }
                    else *Menu_Index+=1;
                   
                }
                else {
                    Lcd_SwitchWindows(DISCHAR_SET_MENU);
                    return;
                }
                Mppt_Curve_MenuIndexDis();
                break;
        }
    }

    static u16 Ble_AutoConnect_timer = NULL;

    void Mppt_BleAutoConnSet_Dis(void)  
    {
        uint8_t location;
        location = Ble_Find_Mac_RepAddr(RoterData.Ble_Adv_rp,sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]),RoterData.Ble_Connect_Mac);
                
        if(!RoterData.Ble_Adv_rp[location].IsModifyFlag)
        {
            Mppt_Set_Para_Send(&RoterData.Mppt_SetPara);
        }
        Lcd_printf20x20(120 - 24 * 3, 0, "������������");
        Lcd_printf20x20(30, 30, "��������:%s",AotoSetStr[RoterData.ConnenctOnFlag]); 
        Lcd_printf20x20(30, 60, "��������:%s",Lock_Mode_Str[RoterData.Filter_LockFlag]);
        Lcd_printf20x20(30, 90, "������ü�¼");
        Lcd_printf20x20(30, 120,"������:%d  ������:%d",RoterData.Ble_Adv_Rp_Count,RoterData.SetCount);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
    }
    void Mppt_BleAutoConnSet_Menu(void)
    {
        Lcd_Clear(BLACK);
        RoterData.ConnenctOnFlag = 0;
        RoterData.Filter_LockFlag = 1;
        Mppt_BleAutoConnSet_Dis();
    }
    void Mppt_BleAutoConnSet_MenuOps(uint8_t key)
    {
        uint32_t i;  
        uint8_t id = MenuData.current_id;
        switch (key)
        {
            case KEY_VALUE_TYPE_ENTRE:
            case KEY_VALUE_TYPE_RIGHT: // 
                switch (MenuData.index[MenuData.current_id])
                {
                    case 1:
                        RoterData.ConnenctOnFlag = !RoterData.ConnenctOnFlag;
                        if(Ble_AutoConnect_timer == NULL)
                            Ble_AutoConnect_timer = sys_timer_add(NULL,Mppt_BleAutoConnSet_Dis,500);
                        Mppt_BleAutoConnSet_Dis();
                        break;  
                    case 2:
                        RoterData.Filter_LockFlag = !RoterData.Filter_LockFlag;
                        Mppt_BleAutoConnSet_Dis();   
                        break;
                    case 3:
                        for(i=0;i<sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]);i++)
                            RoterData.Ble_Adv_rp[i].IsModifyFlag = 0;
                        RoterData.ConnenctOnFlag = 0;
                        RoterData.SetCount = 0;
                        Mppt_BleAutoConnSet_Dis();       
                        return ;
                        break;
                }
            break;
            case KEY_VALUE_TYPE_LEFT:
                sys_timer_del(Ble_AutoConnect_timer);
                RoterData.ConnenctOnFlag = 0;
                RoterData.Filter_LockFlag = 1;
                Ble_AutoConnect_timer = NULL;
                Lcd_BackPreWindows();
                break;

            case KEY_VALUE_TYPE_UP: // ��
                if (MenuData.index[id] > 1)
                    MenuData.index[id]--;
                else
                {
                    Lcd_BackPreWindows();
                    return ;
                }               
                Mppt_Menu_Select_Display();
                break;

            case KEY_VALUE_TYPE_DOWN: // ��
                if(!Menu_Tab[id].max_menu_num)return;
                if (MenuData.index[id] < Menu_Tab[id].max_menu_num)
                    MenuData.index[id]++;
                Mppt_Menu_Select_Display();
                break;

        }   
        //Mppt_Normal_Menu_Select(key);
    }
    
static u16 bl_con_menu_timer;
static int Rssi_Cmpsort(const void*p1,const void*p2)
{
    return(*(Ble_Adv_Rp_t*)p1).rssi<(*(Ble_Adv_Rp_t*)p2).rssi?1:-1;
}


static u16 Cutdown_cnt_Timer = 0;
static u8 Cutdown_cnt = 6;  //����ʱ
void Mppt_Ble_connCount_Cutdown()
{
    int len = sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]);
    int size = sizeof(RoterData.Ble_Adv_rp[0]);
    Cutdown_cnt--;
    Lcd_printf20x20(120 - 10 * 4, 120, "����ʱ:%d",Cutdown_cnt);
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
void Mppt_Ble_con_Menu(void) //�����ʼ��
{
    Lcd_Clear(BLACK);
    Cutdown_cnt = 5;
    Lcd_printf20x20(120 - 20 * 4, 120 - 20, "�豸������......");
    Lcd_printf20x20(120 - 10 * 4, 120, "����ʱ:%d",Cutdown_cnt);
    Cutdown_cnt_Timer = sys_timer_add(NULL,Mppt_Ble_connCount_Cutdown,1000);
}
void Mppt_Ble_con_MenuOps(uint8_t key)
{
    switch (key)
    {
        case KEY_VALUE_TYPE_RIGHT: 
            Lcd_SwitchWindows(BL_CON_SELECT_MENU);
            memcpy(RoterData.Ble_SetConnect_Mac,RoterData.Ble_Adv_rp[0].mac,6);
            if(Cutdown_cnt_Timer)
            {
                sys_timer_del(Cutdown_cnt_Timer);
                Cutdown_cnt_Timer = 0;
            }
            break;
        case KEY_VALUE_TYPE_LEFT: 
            Lcd_BackPreWindows();
            if(Cutdown_cnt_Timer)
            {
                sys_timer_del(Cutdown_cnt_Timer);
                Cutdown_cnt_Timer = 0;
            }
            break;
    }
}


void Mppt_BLeMenuSelect_Dis(void)
{
    uint8_t i;
    for (i = 1; i <= 13; i++)
        Lcd_printf16x16(5, 8 + 16 * i, " ");
    // �����±곤��
    if (!MenuData.index[MenuData.current_id])
        MenuData.index[MenuData.current_id] = 1;
    if (MenuData.index[MenuData.current_id] > 13)
        MenuData.index[MenuData.current_id] = 13;
    Lcd_printf16x16(5, 8 + 16 * MenuData.index[MenuData.current_id], ">");
}
void Mppt_BleMacInfo_Dis(void)
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
        //Lcd_printf16x16(16 + 184, 16 + 16 * (i + 1),"    ");    // ��պ������ʾ ��ֹ֮ǰ�Ĳ���
        Lcd_printf16x16(16, 8 + 16 * (i + 1), str);
    }
}
void Mppt_BleConnSelect_Menu(void)
{
    //MPPT_Get_Info_Timer_Star();
    Lcd_Clear(BLACK); 
    Lcd_printf20x20(120 - 24 * 3, 0, "����ָ������");
    MenuData.index[MenuData.current_id] = 1;
    Lcd_printf16x16(5, 8 + 16 * MenuData.index[MenuData.current_id], ">");
    Mppt_BleMacInfo_Dis();
    bl_con_menu_timer = sys_timer_add(NULL, Mppt_BleMacInfo_Dis, 200);
}
void Mppt_BleConnSelect_MenuOps(uint8_t key)
{
    u16 conn_handle = ble_comm_dev_get_handle(0, 1);
    uint8_t *index = &MenuData.index[MenuData.current_id];
    switch (key) 
    {
        case KEY_VALUE_TYPE_UP: // ��
            if (index[0] > 1)index[0]--;      
            else index[0] = Menu_Tab[MenuData.current_id].max_menu_num;

            ble_gatt_client_disconnect_all();
            memcpy(RoterData.Ble_SetConnect_Mac,RoterData.Ble_Adv_rp[index[0]-1].mac,6);
            //MPPT_Get_Info_Timer_Star();
            Mppt_BLeMenuSelect_Dis();

            //log_info("mac set");
            put_buf(RoterData.Ble_SetConnect_Mac,6);
            put_buf(RoterData.Ble_Adv_rp[index[0]-1].mac,6);    

            
            break;
        case KEY_VALUE_TYPE_DOWN: // ��

         
            if (index[0] < Menu_Tab[MenuData.current_id].max_menu_num)
                index[0]++;
            else
                index[0] = 1;   
                
            ble_gatt_client_disconnect_all();
            memcpy(RoterData.Ble_SetConnect_Mac,RoterData.Ble_Adv_rp[index[0]-1].mac,6);
           // MPPT_Get_Info_Timer_Star();
            Mppt_BLeMenuSelect_Dis();

            break;
        case KEY_VALUE_TYPE_LEFT: // ��
            sys_timer_del(bl_con_menu_timer); 
            
            ble_gatt_client_disconnect_all();
            memset(RoterData.Ble_SetConnect_Mac,0,6);

           
            Lcd_BackPreWindows();
            break;
        case KEY_VALUE_TYPE_RIGHT: // ��
            sys_timer_del(bl_con_menu_timer);
            //ble_gatt_client_disconnect_all();

            Lcd_SwitchWindows(BL_CONNECTING_MENU);
            MPPT_Get_Info_Timer_Star();
            break;
    }
}


    static u16 Get_Info_Timer = 0;
    static u16 Modify_Info_Timer = 0;
    static u16 Display_Info_Timer = 0;
    void Mppt_Ble_Set_Menu(void)
    {
        Lcd_Clear(BLACK);
        Lcd_printf20x20(120 - 24 * 3, 0, "����ָ������");
        Lcd_printf20x20(30,30, "��������Ϣ��ѯ");
        Lcd_printf20x20(30,60, "����ң�ؿ���");
        Lcd_printf20x20(30,90, "�����������޸�");
        /*Lcd_printf20x20(30,90, "�ŵ�����޸�");
        Lcd_printf20x20(30,120,"����ģʽ�޸ģ�%s",Curve_Mode_Str[RoterData.Mppt_ConSetPara_Info.DischarCurve_Moed]);
        Lcd_printf20x20(30,150,"�ŵ������޸�");
        Lcd_printf20x20(30,180,"����ģʽ��%s",Lock_Mode_Str[RoterData.Mppt_ConSetPara_Info.Lock_Mode]);*/

        uint32_t const bl_ckey[4]= {12,34,56,78}; 
        uint8_t data[8]  =
        {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        };
        for(int i=0;i<6;i++)data[i] = RoterData.Ble_Connect_Mac[i];
        btea(data,2,bl_ckey);
        RoterData.paircode = little_endian_read_32(data,0);
         RoterData.paircode%=999999;

        Lcd_printf20x20(30,120,"������������:%d",RoterData.paircode);
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
        //MPPT_Get_Info_Timer_Star();
    }     
    void Mppt_Ble_Set_Ops(uint8_t key)
    {
        switch (key)
        {
            case KEY_VALUE_TYPE_ENTRE:
            case KEY_VALUE_TYPE_RIGHT: // 
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            Lcd_SwitchWindows(MPPT_INFO);
                            break;
                        case 2: 
                            Lcd_SwitchWindows(BLE_CTR);                                         
                            break;
                        case 3:
                            Lcd_SwitchWindows(CURVE_PARAT_MENU);
                            break;
                    }
                break;
            case KEY_VALUE_TYPE_LEFT: //�Ͽ���������
                memset(RoterData.Ble_SetConnect_Mac,0,6);
                ble_gatt_client_disconnect_all();
                break;
        }  
        Mppt_Normal_Menu_Select(key);
    }

    void Mppt_BleConnecting_Menu(void)
    {
        Lcd_Clear(BLACK);
        Lcd_printf20x20(120 - 20 * 4, 120 - 20, "�豸������......");
    }
    void Mppt_BleConnecting_MenuOps(uint8_t key)
    {
        switch (key)
        {
            case KEY_VALUE_TYPE_INFO_OK:
            case KEY_VALUE_TYPE_RIGHT: 
                Lcd_SwitchWindows(BLE_CONNSET_MENU);
                break;
            case KEY_VALUE_TYPE_LEFT: //�Ͽ���������
                memset(RoterData.Ble_SetConnect_Mac,0,6);
                ble_gatt_client_disconnect_all();
                Lcd_BackPreWindows();
                break;
        }  
    }

        void Mppt_BleCtr_Menu(void) 
        {
            Lcd_Clear(BLACK);
            Lcd_printf20x20(120 - 20 * 3, 0, "����ң�ؿ���");
            Lcd_printf20x20(30, 30, "1:����");
            Lcd_printf20x20(30, 50, "2:�ص�");
            Lcd_printf20x20(30, 70, "3:ȫ��");
            Lcd_printf20x20(30, 90, "4:����");
            Lcd_printf20x20(30, 110, "5:�Զ�");
            Lcd_printf20x20(30, 130, "6:3H��ʱ");
            Lcd_printf20x20(30, 150, "7:5H��ʱ");
            Lcd_printf20x20(30, 170, "8:8H��ʱ");
            Lcd_printf20x20(30, 190, "9:�״￪");
            Lcd_printf20x20(30, 210, "0:�״�4H");
        }
        void Mppt_BleCtr_MenuOps(uint8_t key)
        {
            switch (key)
            {
                case KEY_VALUE_TYPE_LEFT:
                    Lcd_BackPreWindows();
                    break;
                case KEY_VALUE_TYPE_1:
                    Mppt_CmdSend(0x01);
                    break;
                case KEY_VALUE_TYPE_2:
                    Mppt_CmdSend(0x02);
                    break;
                case KEY_VALUE_TYPE_3:
                    Mppt_CmdSend(0x03);
                    break;
                case KEY_VALUE_TYPE_4:
                    Mppt_CmdSend(0x04);
                    break;
                case KEY_VALUE_TYPE_5:
                    Mppt_CmdSend(0x05);
                    break;
                case KEY_VALUE_TYPE_6:
                    Mppt_CmdSend(0x06);
                    break;
                case KEY_VALUE_TYPE_7:
                    Mppt_CmdSend(0x07);
                    break;
                case KEY_VALUE_TYPE_8:
                    Mppt_CmdSend(0x08);
                    break;
                case KEY_VALUE_TYPE_9:
                    Mppt_CmdSend(0x09);
                    break;
                case KEY_VALUE_TYPE_0:
                    Mppt_CmdSend(0x00);
                    break;
                default:
                    break;
            }
        }

        void Mppt_Info_Display(void *priv)
        {
            Mppt_Info_Para_t *Info= priv;
            char *str[10];
            const char* OutPut_Status_Str[]={"��������·"};
            Timer_Auto_Off_ReCount();
            Lcd_printf20x20(120 - 20 * 2, 0, "MPPT��Ϣ");
          //  Info->Charge_Capcity = 0.004;
            floatToString(Info->Charge_Capcity,3,str);
            Lcd_printf20x20(30, 20 * 1, "������:%sAh   ",str);
            floatToString(Info->DisCharge_Capcity,3,str);
            Lcd_printf20x20(30, 20 * 2, "�ŵ����:%sAh   ",str);
            floatToString(Info->Charge_Current,3,str);
            Lcd_printf20x20(30, 20 * 3, "������:%sA    ",str);
            floatToString(Info->Dischar_Current,3,str);
           // floatToString(0.118f,3,str);
            Lcd_printf20x20(30, 20 * 4, "�ŵ����:%sA    ",str);
            floatToString(Info->Charge_Power,3,str);
            Lcd_printf20x20(30, 20 * 5, "��繦��:%sW    ",str);
            Lcd_printf20x20(30, 20 * 6, "�������:%dm    ",Info->Bat_Resistance);
            Lcd_printf20x20(30, 20 * 7, "��ص���:%d%%   ",Info->Bat_Capcity);
            floatToString(Info->Bat_Voltage,3,str);
            Lcd_printf20x20(30, 20 * 8, "��ص�ѹ:%sV    ",str);
            //Lcd_printf20x20(30, 25 * 8, "���״̬:%s     ",OutPut_Status_Str[Info->OutPut_Staus]);
           
            floatToString(RoterData.Mppt_ConSetPara_Info.Bat_Capcity,1,str);
          //  Lcd_printf20x20(30,20 * 9,"�������:%sAh  ",str);

            uint8_t len;
            floatToString(Info->Temp,3,str);
            len = strlen(str);
            Lcd_printf20x20(30,20 * 9,"ϵͳ�¶�:%s",str);
            Lcd_Show20x20(30+ 90 + len*10,20 * 9 ,SpecialDot[1]);

            floatToString(Info->MaxTemp,3,str);
            len = strlen(str);
            Lcd_printf20x20(30,20 * 10,"��ֵ�¶�:%s",str);
            Lcd_Show20x20(30+ 90 + len*10,20 * 10 ,SpecialDot[1]);

            
            
            Lcd_printf20x20(30,20 * 11,"��һҳ",str);
            Lcd_Show20x20(30+60,20*11,SpecialDot[0]);
            
        }
        void Mppt_Info_Menu(void) //�����Ϣ����
        {
            Lcd_Clear(BLACK);
            Mppt_Info_Display(&RoterData.Mppt_Info);
            Get_Info_Timer = sys_timer_add(NULL,Get_Mppt_Report,500);
            Display_Info_Timer = sys_timer_add(&RoterData.Mppt_Info,Mppt_Info_Display,500);
        }
        void Mppt_Info_MenuOps(uint8_t key)
        {
            if(key == KEY_VALUE_TYPE_LEFT ||key == KEY_VALUE_TYPE_UP || key == KEY_VALUE_TYPE_DOWN)
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

                if(key == KEY_VALUE_TYPE_LEFT ||key == KEY_VALUE_TYPE_UP)
                {
                    Lcd_BackPreWindows();
                }

                if(key == KEY_VALUE_TYPE_DOWN){
                    Lcd_SwitchWindows(INFO_CURPARA);
                }
            }

            //Mppt_Normal_Menu_Select(key);
            
        }

        void Mppt_InfoChargePara_Menu(void) //��������ʾ
        {
            Lcd_Clear(BLACK);
            Mppt_ChargePara_Display(&RoterData.Mppt_ConSetPara_Info);
            Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "  ");
            //Lcd_printf20x20(30,120,"��һҳ");
            //Lcd_Show20x20(30+60,120,SpecialDot[0]);
        }
        void Mppt_InfoChargePara_MenuOps(uint8_t key)
        {
            if(key == KEY_VALUE_TYPE_UP || key == KEY_VALUE_TYPE_LEFT )
            {    
                Lcd_BackPreWindows();
            }
           /* if(key == KEY_VALUE_TYPE_DOWN ){
                Lcd_SwitchWindows(INFO_DISCHARGEPARA);
            }*/
        }

        void Mppt_InfoDischarPara_Menu(void)    //�ŵ������ʾ
        {
            Lcd_Clear(BLACK);
            Mppt_DischarPara_Dis(&RoterData.Mppt_ConSetPara_Info);  
            Lcd_printf20x20(30, 24*9, "��һҳ");
            Lcd_Show20x20(90,24*9,SpecialDot[0]);
        }
        void Mppt_InfoDischarPara_MenuOps(uint8_t key)
        {
            if(key == KEY_VALUE_TYPE_UP || key == KEY_VALUE_TYPE_LEFT ){
                Lcd_BackPreWindows();
            }
            if(key == KEY_VALUE_TYPE_DOWN)
            {
                Lcd_SwitchWindows(INFO_RADARPARA);
            }
        }

        void Mppt_InfoCurvePara_Menu(void)    //�ŵ������ʾ
        {
            Lcd_Clear(BLACK);
            MenuData.index[MenuData.current_id] = 0;
            Mppt_DischarCurve_Display(&RoterData.Mppt_ConSetPara_Info);
            Lcd_printf20x20(30,210,"��һҳ");
            Lcd_Show20x20(30+60,210,SpecialDot[0]);
        }
        void Mppt_InfoCurvePara_MenuOps(uint8_t key)
        {
            if(key == KEY_VALUE_TYPE_UP || key == KEY_VALUE_TYPE_LEFT ){
                Lcd_BackPreWindows();
                MPPT_Get_Info_Timer_Star();
            }
            if(key == KEY_VALUE_TYPE_DOWN)
            {
                Lcd_SwitchWindows(INFO_DISCHARGEPARA);
            }
        }

        void Mppt_InfoRadarPara_Menu(void)
        {
            Lcd_Clear(BLACK);
            MenuData.index[MenuData.current_id] = 1;
            Mppt_RadarPara_Dis(&RoterData.Mppt_SetPara);
            Lcd_printf20x20(30, 24*9, "��һҳ");
            Lcd_Show20x20(90,24*9,SpecialDot[0]);
        }
        void Mppt_InfoRadarPara_MenuOps(uint8_t key)
        {
            if(key == KEY_VALUE_TYPE_UP || key == KEY_VALUE_TYPE_LEFT ){
                Lcd_BackPreWindows();
            }
            if(key == KEY_VALUE_TYPE_DOWN)
            {
                Lcd_SwitchWindows(INFO_CHARGEPARA);
            }
        }

        void Mppt_ChargeParaM_Menu(void)
        {
            Lcd_Clear(BLACK);        
            MenuData.index[MenuData.current_id] = 1;
            Lcd_printf20x20(120 - 24 * 3, 0, "�������޸�");
            Mppt_ChargePara_Display(&RoterData.Mppt_ConSetPara_Info);
            Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
            Lcd_printf20x20(30,120,"��һҳ");
            Lcd_Show20x20(30+60,120,SpecialDot[0]);
        }
        void Mppt_ChargeParaM_MenuOps(uint8_t key)
        {
            Mppt_ChargeSet_Ops(&RoterData.Mppt_ConSetPara_Info,key);
            if(InputMode)return ;
            switch (key)
            {
                case KEY_VALUE_TYPE_UP: // ��
                    if (MenuData.index[MenuData.current_id] > 1)
                        MenuData.index[MenuData.current_id]--;
                    else
                    {
                        Lcd_BackPreWindows();
                        return ;
                    }
                        
                    Mppt_Menu_Select_Display();
                    break;
                case KEY_VALUE_TYPE_DOWN: // ��
                    if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
                    if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                        MenuData.index[MenuData.current_id]++;
                    else{
                        Lcd_SwitchWindows(MODIFY_MENU);
                        return ;
                    }
                    Mppt_Menu_Select_Display();
                    break;
                case KEY_VALUE_TYPE_LEFT: // ��
                    Lcd_BackPreWindows();
                    return ;
                    break;
                /*case KEY_VALUE_TYPE_RIGHT:
                    Lcd_SwitchWindows(DISCHAR_PARA_MODIFY);
                    break;*/
            }
        }

        void Mppt_DischarParaM_Menu(void)
        {
            Lcd_Clear(BLACK);
            MenuData.index[MenuData.current_id] = 1;
           // Lcd_printf20x20(120 - 24 * 3, 0, "�ŵ�����޸�");
            Mppt_DischarPara_Dis(&RoterData.Mppt_ConSetPara_Info);
            Lcd_printf20x20(5, 24 * MenuData.index[MenuData.current_id], "->");     
            Lcd_printf20x20(30, 24*9, "��һҳ");
            Lcd_Show20x20(90,24*9,SpecialDot[0]);
        }    
        void Mppt_DischarParaM_MenuOps(uint8_t key)
        {
            Mppt_DischarSet_Ops(&RoterData.Mppt_ConSetPara_Info,key);
            if(InputMode)return ;
            switch (key)
            {
                case KEY_VALUE_TYPE_UP: // ��
                    if (MenuData.index[MenuData.current_id] > 1)
                        MenuData.index[MenuData.current_id]--;
                    else
                    {
                        Lcd_BackPreWindows();
                        return ;
                    }
                        
                  //  Mppt_Menu_Select_Display();
                    Mppt_MenuSelect_Dis();
                    break;
                case KEY_VALUE_TYPE_DOWN: // ��
                    if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
                    if (MenuData.index[MenuData.current_id] < 4)
                        MenuData.index[MenuData.current_id]++;
                    else{
                        Lcd_SwitchWindows(RADAR_PARA_MODIFY);
                        return ;
                    }
                   // Mppt_Menu_Select_Display();
                   Mppt_MenuSelect_Dis();
                    break;
                case KEY_VALUE_TYPE_LEFT: // ��
                    Lcd_BackPreWindows();
                    return ;
                    break;
               /* case KEY_VALUE_TYPE_RIGHT:
                    Lcd_SwitchWindows(CURVE_MODIFY);
                    break;*/
            }
        }

        void Mppt_RadarParaM_Menu(void)
        {
            Lcd_Clear(BLACK);
            MenuData.index[MenuData.current_id] = 1;
            Mppt_RadarPara_Dis(&RoterData.Mppt_ConSetPara_Info);
            Lcd_printf20x20(5, 24 * MenuData.index[MenuData.current_id], "->");
            Lcd_printf20x20(30, 24*9, "��һҳ");
            Lcd_Show20x20(90,24*9,SpecialDot[0]);
        }
        void Mppt_RadarParaM_MenuOps(uint8_t key)
        {
            Mppt_RadarPara_Ops(&RoterData.Mppt_ConSetPara_Info,key);
            if(InputMode)return ;
            switch (key)
            {
                case KEY_VALUE_TYPE_UP: // ��
                    if (MenuData.index[MenuData.current_id] > 1)
                        MenuData.index[MenuData.current_id]--;
                    else
                    {
                        Lcd_BackPreWindows();
                        return ;
                    }      
                    Mppt_MenuSelect_Dis();
                    break;
                case KEY_VALUE_TYPE_DOWN: // ��
                    if (MenuData.index[MenuData.current_id] < 5)
                        MenuData.index[MenuData.current_id]++;
                    else{
                        Lcd_SwitchWindows(CHAEGE_PARA_MODIFY);
                        return ;
                    }
                    Mppt_MenuSelect_Dis();
                    break;
                case KEY_VALUE_TYPE_LEFT: // ��
                    Lcd_BackPreWindows();
                    break;
            }
        }

        void Mppt_Curce_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);
            MenuData.index[MenuData.current_id] = 1;
            Lcd_printf20x20(120 - 20 * 3, 0, "���߲�������");
            Lcd_printf20x20(30, 30, "�ŵ�����ģʽ:%s",Curve_Mode_Str[RoterData.Mppt_ConSetPara_Info.DischarCurve_Moed]);
            Lcd_printf20x20(30, 60,"�ŵ���������");
            Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
        }
        void Mppt_Curce_Modify_MenuOps(uint8_t key)
        {
            switch (key)
            {
                case KEY_VALUE_TYPE_UP: // ��
                    if (MenuData.index[MenuData.current_id] > 1)
                        MenuData.index[MenuData.current_id]--;
                    else
                    {
                        Lcd_BackPreWindows();
                    }
                        
                    Mppt_Menu_Select_Display();
                    break;
                case KEY_VALUE_TYPE_DOWN: // ��
                    if(!Menu_Tab[MenuData.current_id].max_menu_num)return;
                    if (MenuData.index[MenuData.current_id] < Menu_Tab[MenuData.current_id].max_menu_num)
                        MenuData.index[MenuData.current_id]++;
                    else{
                            Lcd_SwitchWindows(MODIFY_MENU);
                    }
                    Mppt_Menu_Select_Display();
                    break;
                case KEY_VALUE_TYPE_LEFT: // ��
                    Lcd_BackPreWindows();
                    break;
            }
            switch (key)
            {
                case KEY_VALUE_TYPE_ENTRE:
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 2:
                            Lcd_SwitchWindows(CURVE_PARAT_MENU);
                            return ;
                            break;
                    }  
                case KEY_VALUE_TYPE_INCRE:
                    switch (MenuData.index[MenuData.current_id])
                    {
                        case 1:
                            if(++RoterData.Mppt_SetPara.DischarCurve_Moed>2)RoterData.Mppt_SetPara.DischarCurve_Moed = 0;
                            Mppt_CurceSet_Menu();
                            return ;
                            break;      
                    }  
                /*case KEY_VALUE_TYPE_RIGHT: // 
                    Lcd_SwitchWindows(CURVE_PARAT_MENU);
                    break;*/
            }   
        }

        void Mppt_CurvePara_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);
            MenuData.index[MenuData.current_id] = 0;
            Mppt_Curve_MenuIndexDis();
            Mppt_DischarCurve_Display(&RoterData.Mppt_ConSetPara_Info);
            Lcd_printf20x20(30,210,"��һҳ");
            Lcd_Show20x20(30+60,210,SpecialDot[0]);
        }
        void Mppt_CurvePara_Modify_MenuOps(uint8_t key)
        {
            uint8_t Menu_Id = MenuData.current_id; // ��ǰID��
            uint8_t *Menu_Index = &MenuData.index[Menu_Id];
            Mppt_Set_Parm_t *Para = &RoterData.Mppt_ConSetPara_Info;
            Mppt_CurveSet_Ops(Para,key);
            switch (key)
            {
                case KEY_VALUE_TYPE_UP: // ��
                    if(*Menu_Index == 0)
                    {
                        Lcd_BackPreWindows();
                        return;
                    }
                    if(Para->DischarCurve_Moed > 1)return;
                    if(!Menu_Tab[Menu_Id].max_menu_num)return;
                    if (*Menu_Index )*Menu_Index-=1;    
                    else *Menu_Index = Menu_Tab[Menu_Id].max_menu_num;                
                    Mppt_Curve_MenuIndexDis();
                    break;
                case KEY_VALUE_TYPE_DOWN: // ��
                    if(Para->DischarCurve_Moed > 1)
                    {
                        Lcd_SwitchWindows(DISCHAR_PARA_MODIFY);
                        return;
                    }
                    if(!Menu_Tab[Menu_Id].max_menu_num)return;
                    if (*Menu_Index < Menu_Tab[Menu_Id].max_menu_num)
                    {
                        if(*Menu_Index+1 == 9 || *Menu_Index+1 == 17 )
                        {
                            Lcd_SwitchWindows(DISCHAR_PARA_MODIFY);
                            return;
                        }
                        else *Menu_Index+=1;
                        
                    }
                    else {
                        Lcd_SwitchWindows(DISCHAR_PARA_MODIFY);
                        return;
                    }
                    Mppt_Curve_MenuIndexDis();
                    break;
            }
        }

        void Mppt_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);
          //  Lcd_printf20x20(120 - 20 * 2, 0, "�����޸�");
            Lcd_printf20x20(120 - 70,100,"ȷ���޸Ĳ���?");
            Lcd_printf20x20(120 - 90,120,"����ȷ�ϼ������޸�");
            Lcd_printf20x20(120 - 80,140,"����<-���˳��޸�");
        }
        void Mppt_Modify_MenuOps(uint8_t key)
        {
            Mppt_Normal_Menu_Select(key);
            if(key == KEY_VALUE_TYPE_ENTRE)
            {
                Lcd_SwitchWindows(ENTRY_MODIFY);
                Modify_Info_Timer = sys_timer_add(&RoterData.Mppt_ConSetPara_Info,Mppt_Set_Para_Send,1000);    
            }
           // MPPT_Get_Info_Timer_Star();
        }

        void Mppt_Comfir_Modify_Menu(void)
        {
            Lcd_Clear(BLACK);
            Lcd_printf20x20(120 - 60, 100, "�޸���......");     
        }
        void Mppt_Comfir_Modify_MenuOps(uint8_t key)
        {
            if(key == KEY_VALUE_TYPE_MODIFY_OK)
            {
                if(Modify_Info_Timer)sys_timer_del(Modify_Info_Timer);
                Lcd_Clear(BLACK);
                Lcd_printf20x20(120 - 10 * 30, 100, "�޸ĳɹ�"); 
                Lcd_printf20x20(120 - 100, 120, "����<-���������ӽ���"); 
            }
            if(key == KEY_VALUE_TYPE_LEFT)
            {
                if(Modify_Info_Timer)sys_timer_del(Modify_Info_Timer);
            }   
            Mppt_Normal_Menu_Select(key);
           // MPPT_Get_Info_Timer_Star();
        }

void Mppt_Ir_Set_Menu(void)
{
    Lcd_Clear(BLACK);   
    Lcd_printf20x20(120 - 24 * 3, 0, "�����������");
    Lcd_printf20x20(30, 30,  "����ң����");
    Lcd_printf20x20(30, 60,  "����ң����");
    //Lcd_printf20x20(30, 120,  "�ŵ���������");
    Lcd_printf20x20(30, 90, "�û�������");
   // Lcd_printf20x20(30, 180, "����ȷ��");
    Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
}
void Mppt_Ir_Set_MenuOps(uint8_t key)  
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
                    Lcd_SwitchWindows(IR_NORMAL_MENU);
                    break;
                case 2:            
                    Lcd_SwitchWindows(IR_ENGINEER_MENU);
                    break;
                case 3:
                    Lcd_SwitchWindows(IR_USERCODE_MENU);
                    break;
                case 4:                  
                    Lcd_SwitchWindows(IR_ENGINEER_MENU);
                    break;
                case 5: //�������ݷ���
                    memset(&ir_tx_data,0,sizeof(ir_tx_data)) ;    //�������
                    
                    if(RoterData.Mppt_SetPara.DischarCurve_Moed == 2)
                    {
                        ir_tx_data[0] = 0X5F; ir_tx_data[1] = 0XFA;  //ͷ��
                        ir_tx_data[2] = 0X00; ir_tx_data[3] = 0X0f;  //ͷ��
                    }
                    else 
                    {
                        ir_tx_data[0] = 0X5F; ir_tx_data[1] = 0XF5;  //ͷ��
                        for(i=0;i<=8;i++)   //�������ݷ���
                        {
                            ir_tx_data[2 + i*2] = (uint8_t)(RoterData.Mppt_SetPara.Curv_Data[i][0] * 10);
                            ir_tx_data[2 + i*2 + 1] = (uint8_t)(RoterData.Mppt_SetPara.Curv_Data[i][1]);
                        }
                    }
                    
                    Check_Sum = 0;
                    for(i=0;i<18;i++)Check_Sum += ir_tx_data[i];
                    ir_tx_data[18] = Check_Sum >> 8;    // ��λ
                    ir_tx_data[19] = Check_Sum & 0xff;  //��λ
                    Ir_tx_star_x(ir_tx_data,sizeof(ir_tx_data));
                    break; 
            }                                   
            break;
    }
}
   
    void IR_Normal_Menu(void)
    {
        Lcd_Clear(BLACK);   
        Lcd_printf20x20( 120 - 10 * 5 , 0, "����ң����");
        Lcd_printf20x20(20, 30, "1:����   2:�ص�");
        Lcd_printf20x20(20, 60, "3:ȫ��   4:����");
        Lcd_printf20x20(20, 90, "+:����+  -:����-");
        Lcd_printf20x20(20, 120,"5:�״￪ 6:�״�4H��"); 
        Lcd_printf20x20(20, 150,"7:3H��ʱ 8:5H��ʱ");
        Lcd_printf20x20(20, 180,"9:8H��ʱ 0:�Զ�ģʽ");
        //Lcd_printf20x20(30, 180,"");
    }
    void IR_Normal_MenuOps(uint8_t key)
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
        Lcd_printf20x20(120 - 10 * 5 , 0, "����ң����");
        Lcd_printf20x20(30, 60, "1:���� 2:�ص�");
        Lcd_printf20x20(30, 90, "2:���⿪ 3:�����");
        Lcd_printf20x20(30, 120,"4:�״￪ 5:�״��");
        Lcd_printf20x20(30, 150,"6:������λ����:%d",Ir_current_Gear);
        Lcd_printf20x20(0, 30 *5 , "->");
    }
    void IR_Engineer_MenuOps(uint8_t key)
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
                Lcd_printf20x20(30, 150,"6:������λ����:%d ",Ir_current_Gear);
                break;
            case KEY_VALUE_TYPE_DECRE:
                if(Ir_current_Gear>1)Ir_current_Gear--;
                Lcd_printf20x20(30, 150,"6:������λ����:%d ",Ir_current_Gear);
                break;
        }
        Mppt_Normal_Menu_Select(key);
    }

    void IR_Usercode_Menu(void)
    {
        Lcd_Clear(BLACK);   
        Lcd_printf20x20( 120 - 12 * 5 , 0, "�û�������");
        Lcd_printf20x20( 30 , 30, "�û�������:%d",RoterData.Mppt_SetPara.Usercode);
        Lcd_printf20x20( 30 , 60, "ȷ���޸�");
        Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
    }
    void IR_Usercode_MenuOps(uint8_t key)
    {
        int num_key = Key_NumberMap(key);
        float indata;
        if(num_key != -1) // ���������� ��һ���㲻�����ַ�������
        {
            if(!InputMode  )
            {
                InputMode = 1; //  ����ģʽ
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
                case KEY_VALUE_TYPE_BACKSPACE: // �˸�
                    if(input_index)
                    {   
                        if(input_index == 1)instr[input_index - 1] = '0';
                        else instr[input_index - 1] = 0; 
                        input_index --;       
                    }
                    break;
                case KEY_VALUE_TYPE_ENTRE:
                    indata = stringtofloat(instr);
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
            Lcd_printf20x20( 30 , 30, "�û�������:%s     ",instr);
             
            log_info("KEY %d NUMKRY %d input_index %d",key,num_key,input_index);
            log_info("instr %s value: %d",instr,(uint32_t)(stringtofloat(instr)*100));

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

    void IR_CruveSet_MenuOps(uint8_t key)
    {
          Mppt_CurveSet_Ops(&RoterData.Mppt_SetPara,key);
    }

void Mppt_Version_Info_menu(void)
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 24 * 3, 0, "ϵͳ��Ϣ��ѯ");
    Lcd_printf20x20(30, 60, "�û���:%d",RoterData.Mppt_SetPara.Usercode);
    Lcd_printf20x20(30, 90, "ң�����汾:1.1V");
    Lcd_printf20x20(30, 120, "��������:2024-5-1");
}

void SYS_Set_Menu(void)
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 10 * 6, 0, "ң����������ѯ");
    Lcd_printf20x20(30, 30, "�Զ��ػ�ʱ��: 120s");
    Lcd_printf20x20(30, 60, "����: 100%%");
    Lcd_printf20x20(30, 90, "������: OFF");
    Lcd_printf20x20(30, 120, "���书��: 3dB");
   // Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
}

void Mppt_Version_Select_Menu(void)
{
    Lcd_Clear(BLACK);
    Lcd_printf20x20(120 - 20 * 4, 0, "MPPT�汾����");
    Lcd_printf20x20(30, 30, "SQ20P75SA-B");
    Lcd_printf20x20(5, 30 * MenuData.index[MenuData.current_id], "->");
    Lcd_printf20x20(30, 210, "��ǰ�汾:%s","SQ20P75SA-B "); 
}


// ���������ظ�λ��
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

// ���ҿյ�ַ
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
                RoterData.Ble_Adv_rp[i].useflag = 0; // ���ʹ�ñ�־
                RoterData.Ble_Adv_rp[i].rssi = -99;
                for(j=0;j<6;j++)RoterData.Ble_Adv_rp[i].mac[j] = 0;
                RoterData.Ble_Adv_Rp_Count -- ;
            }
        }
        
    }
}


