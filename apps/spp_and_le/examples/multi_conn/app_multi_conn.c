#include "system/app_core.h"
#include "system/includes.h"
#include "server/server_core.h"
#include "app_config.h"
#include "app_action.h"
#include "os/os_api.h"
#include "btcontroller_config.h"
#include "btctrler/btctrler_task.h"
#include "config/config_transport.h"
#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "bt_common.h"
#include "rcsp_bluetooth.h"
#include "rcsp_user_update.h"
#include "app_charge.h"
#include "app_chargestore.h"
#include "app_power_manage.h"
#include "le_client_demo.h"
#include "app_comm_bt.h"
#include "norflash.h"

#include "asm/mcpwm.h"
#include "asm/ledc.h"
#include "asm/spi.h"

#define LOG_TAG_CONST       MULTI_CONN
#define LOG_TAG             "[MULTI_CONN]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE

#include "debug.h"
#include "lcd7789.h"
#include "lcd_data.h"

#define Enable_beep 0


#if CONFIG_APP_MULTI

#define CFG_USER_VAR	 2
#define CFG_USER_BUF	 3
#define CFG_USER_STRUCT	 4

static u8 is_app_multi_active = 0;

void multi_set_soft_poweroff(void)
{
    log_info("set_soft_poweroff\n");
    is_app_multi_active = 1;
    //必须先主动断开蓝牙链路,否则要等链路超时断开

#if TCFG_USER_BLE_ENABLE
    btstack_ble_exit(0);
#endif

#if TCFG_USER_EDR_ENABLE
    btstack_edr_exit(0);
#endif

#if (TCFG_USER_EDR_ENABLE || TCFG_USER_BLE_ENABLE)
    //延时300ms，确保BT退出链路断开
    sys_timeout_add(NULL, power_set_soft_poweroff, WAIT_DISCONN_TIME_MS);
#else
    power_set_soft_poweroff();
#endif
}

#define Power_En IO_PORTB_01
static u8 Power_Flag = 0;
void Power_Port_Init()
{
    gpio_set_pull_up(Power_En, 0);
    gpio_set_pull_down(Power_En, 0); 
    gpio_set_direction(Power_En, 0);
    gpio_write(Power_En, 1);
    Power_Flag = 1;
}
void Power_Lock()
{
    gpio_write(Power_En, 1);
}
void Power_UnLock()
{
    int ret = 0;
    ret = syscfg_write(CFG_USER_STRUCT, &RoterData.Mppt_SetPara, sizeof(RoterData.Mppt_SetPara));
    log_info("%s[CFG_USER_STRUCT -> syscfg_write:%d]", __func__, ret);
    if(ret != sizeof(RoterData.Mppt_SetPara))
        log_error("CFG_USER_STRUCT -> syscfg_write -> err:%d", ret);
    Power_Flag = 0;
    gpio_write(Power_En, 0);
    
}

static u16 Bepp_timer  = 0;
void Beep_init(void)
{
    struct pwm_platform_data pwm_p_data;
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch0;                 //通道0
    pwm_p_data.frequency = 2000;                    //1KHz
    pwm_p_data.duty = 0;                          //占空比50%
    pwm_p_data.h_pin = -1;                           //不用则填 -1
    pwm_p_data.l_pin = IO_PORTA_09;                         //硬件引脚
    mcpwm_init(&pwm_p_data);
}
static void Beep_Func(void *priv)
{
    log_info("Bepp stop");
    mcpwm_set_duty(pwm_ch0,0);
    Bepp_timer = 0;//ID号清0,必须要有！！！
}
void Beep_Star(void)
{
    log_info("%s[id:%d]", __func__, Bepp_timer);
    if(Bepp_timer)
    {
        sys_hi_timeout_del(Bepp_timer);//注册前,要判断是否注册过
        Bepp_timer = 0;
    }
    mcpwm_set_duty(pwm_ch0,5000);
    Bepp_timer = sys_hi_timeout_add(NULL, Beep_Func, 50);//ms单位
}
void Beep_Timer_Del(void)
{
    log_info("%s[id:%d]", __func__, Bepp_timer);
    if(Bepp_timer)
    {
        sys_timeout_del(Bepp_timer);
        Bepp_timer = 0;
    }
}

#define IR_TX_PORT IO_PORTA_08
void Ir_pwm_init(void)
{
    struct pwm_platform_data pwm_p_data;
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch3; // 通道0
    pwm_p_data.frequency = 38000;    // 1KHz
    pwm_p_data.duty = 0;             // 占空比50%
    pwm_p_data.h_pin = -1;           // 不用则填 -1
    pwm_p_data.l_pin = IR_TX_PORT;  // 硬件引脚
    mcpwm_init(&pwm_p_data);
}

u8 ir_tx_fsm = 0, ir_Data[20] = {0xAA, 0x55, 0xF0, 0x0F}; // 地址正吗 地址反码 数据正码 数据反码
u8 Ir_tx_num;
void Ir_tx_stop()
{
    JL_TIMER3->CON &= ~0x00000001; // 停止模式
}
void Ir_tx_star(u8 adr,u8 data) //
{
    ir_Data[0] = adr;ir_Data[1] = ~adr;
    ir_Data[2] = data;ir_Data[3] = ~data;
    Ir_tx_num = 4;
    JL_TIMER3->PRD = 16 * 500;
    JL_TIMER3->CON |= 1; // 计数模式
    log_info("Ir Tx Data %x", reverse_u32(*(u32 *)ir_Data));
}
void Ir_tx_star_adr(u16 adr,u8 data) //
{
    ir_Data[0] = adr >> 8;ir_Data[1] = (u8)(adr);
    ir_Data[2] = data;ir_Data[3] = ~data;
     Ir_tx_num = 4;
    JL_TIMER3->PRD = 16 * 500;
    JL_TIMER3->CON |= 1; // 计数模式
    log_info("Ir Tx Data %x", reverse_u32(*(u32 *)ir_Data));
}
void Ir_tx_star_Def(u8 *data) //
{
    memcpy(ir_Data,data,4);
     Ir_tx_num = 4;
    JL_TIMER3->PRD = 16 * 500;
    JL_TIMER3->CON |= 1; // 计数模式
    log_info("Ir Tx Data %x", reverse_u32(*(u32 *)ir_Data));
}
static uint8_t ConvertChar(uint8_t u8Inchar)
{
    u8Inchar = (char) ((u8Inchar << 4) + (u8Inchar >> 4));
    u8Inchar = ((u8Inchar & 0x33) << 2) + ((u8Inchar & 0xCC) >> 2);
    u8Inchar = ((u8Inchar & 0x55) << 1) + ((u8Inchar & 0xAA) >> 1);
    return u8Inchar;
}
void Ir_tx_star_x(uint8_t *data,uint8_t len) //
{
    uint8_t i;
    memcpy(ir_Data,data,len);
   // for(i=0;i<len;i++)ir_Data[i] = ConvertChar(ir_Data[i]);
    Ir_tx_num = len;
    JL_TIMER3->PRD = 16 * 500;
    JL_TIMER3->CON |= 1; // 计数模式
   // log_info("Ir Tx Data %x", reverse_u32(*(u32 *)ir_Data));
}

___interrupt
    AT_VOLATILE_RAM_CODE static void
    timer3_isr()
{
    static u8 i, j, ishighFlag;
    JL_TIMER3->CON |= BIT(14);
    switch (ir_tx_fsm)
    {
    case 0: // 引导码1
        mcpwm_set_duty(pwm_ch3, 5000);
        JL_TIMER3->PRD = 6 * 9000; // 9MS
        ir_tx_fsm++;
        break;
    case 1: // 引导高0
        mcpwm_set_duty(pwm_ch3, 0);
        JL_TIMER3->PRD = 6 * 4500; // 9MS
        ir_tx_fsm++;
        break;
    case 2:
        if (i >= 8)
        {
            i = 0;
            if (++j >= Ir_tx_num)
            {
                j = 0;
                mcpwm_set_duty(pwm_ch3, 5000);
                JL_TIMER3->PRD = 6 * 560; // 9MS
                ir_tx_fsm = 5;
                break;
            }
        }
        if (ir_Data[j] & (0X80 >> i))
        {
            ishighFlag = 1;
        }
        else
            ishighFlag = 0;
        ir_tx_fsm++;
    case 3: //  数据码1
        mcpwm_set_duty(pwm_ch3, 5000);
        JL_TIMER3->PRD = 6 * 560; // 9MS
        ir_tx_fsm++;
        break;
    case 4: //  数据码0
        mcpwm_set_duty(pwm_ch3, 0);
        if (ishighFlag)
            JL_TIMER3->PRD = 6 * 1600; // 9MS
        else
            JL_TIMER3->PRD = 6 * 560;
        i++;
        ir_tx_fsm = 2;
        break;
    case 5: // 结束码
        mcpwm_set_duty(pwm_ch3, 0);
        ir_tx_fsm = 0;
        Ir_tx_stop();
        break;

    default:
        ir_tx_fsm = 0;
        break;
    }
}
void ir_tx_init(void)
{
    u32 u_clk = 24000000;
    JL_TIMER3->CON |= (0b110 << 10); // 时钟源选择STD_24M时钟源
    JL_TIMER3->CON |= (0b0001 << 4); // 时钟源再4分频
    JL_TIMER3->CNT = 0;              // 清计数值
    JL_TIMER3->PRD = 6 * 560;
    ; // 设置周期 1US + 1
    // JL_TIMER3->CON |= 1 ;	//计数模式
    request_irq(IRQ_TIME3_IDX, 3, timer3_isr, 0); // 注册中断函数
    log_info("Ir Time Init");
    Ir_pwm_init();
    log_info("Ir Pwm Init");
}

void Sys_Auto_Off() // 90秒自动关机
{
    Power_UnLock();
}

static u16 Sys_Auto_Off_Timer;
void Timer_Auto_Off_ReCount()
{
    sys_timer_re_run(Sys_Auto_Off_Timer);
}
/*
NORFLASH_DEV_PLATFORM_DATA_BEGIN(norflash_fat_dev_datatest)
    .spi_hw_num     = TCFG_FLASH_DEV_SPI_HW_NUM,
    .spi_cs_port    = TCFG_FLASH_DEV_SPI_CS_PORT,
    .spi_read_width = 4,
#if (TCFG_FLASH_DEV_SPI_HW_NUM == 1)
    .spi_pdata      = &spi1_p_data,
#elif (TCFG_FLASH_DEV_SPI_HW_NUM == 2)
    .spi_pdata      = &spi2_p_data,
#endif
    .start_addr     = 0,
    .size           = 16*1024*1024,
NORFLASH_DEV_PLATFORM_DATA_END()*/
static void multi_app_start()
{
    uint8_t i;
    log_info("=======================================");
    log_info("-----------multi_conn demo-------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

    

    clk_set("sys", BT_NORMAL_HZ);
    
    #if (TCFG_USER_EDR_ENABLE || TCFG_USER_BLE_ENABLE)
        u32 sys_clk =  clk_get("sys");
        bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    #if TCFG_USER_EDR_ENABLE
         btstack_edr_start_before_init(NULL, 0);
    #endif

    #if TCFG_USER_BLE_ENABLE
        btstack_ble_start_before_init(NULL, 0);
    
    #endif
        btstack_init();

    // WP 
    gpio_set_pull_up(IO_PORTB_04, 0);
    gpio_set_pull_down(IO_PORTB_04, 0); 
    gpio_set_direction(IO_PORTB_04, 0);
    gpio_write(IO_PORTB_04, 1);

    //HOLD
    gpio_set_pull_up(IO_PORTB_08, 0);
    gpio_set_pull_down(IO_PORTB_08, 0); 
    gpio_set_direction(IO_PORTB_08, 0);
    gpio_write(IO_PORTB_08, 1);
    
    //LOCK
    Power_Port_Init();
        //SPI测试
      //  devices_init();

        uint32_t temp = 0xfffffffff;
       // uint8_t *Rtemp = malloc(512);

       // for(int i =0;i<512;i++)Rtemp[i]=0;
      
        //log_info("Rtemp = %s", Rtemp);
        /*struct  device *Norflashdevice;        
        norfs_dev_ops.open("lcdtxt",&Norflashdevice,NULL);
        norfs_dev_ops.write(Norflashdevice,DotTbl16,512,0);
        norfs_dev_ops.read(Norflashdevice,Rtemp,512,0);*/
        
     
        void *Norflash_Device = dev_open("lcdtxt",NULL);
        log_info("Norflash_Device = %d", Norflash_Device);
        // dev_bulk_write(Norflash_Device,&ttemp,0,sizeof(ttemp));    // 第一次烧进 225300
        // dev_bulk_write(Norflash_Device,&ttemp,225300,sizeof(ttemp)); // 180480
        // dev_bulk_write(Norflash_Device,&gImage_image,405780,sizeof(gImage_image)); // 180480
        // dev_bulk_read(Norflash_Device,Rtemp,225300-60,512);
        

        // Rtemp[511] = 0;
        // log_info("Rtemp = %s", Rtemp);
        // put_buf(Rtemp,512);

        
        spi_open(SPI1);
        ST7789Lcd_Init();
        Lcd_SetFlashDevice(Norflash_Device);
        

        
        #if Enable_beep
        Beep_init();
        Beep_Star();
        #endif

        //默认
        for(i=0;i<sizeof(MenuData.index);i++)MenuData.index[i] = 1;
        for(i=0;i<sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]);i++)
            RoterData.Ble_Adv_rp[i].rssi = -99;

        RoterData.Mppt_SetPara.Bat_Capcity = 7.5;
        RoterData.Mppt_SetPara.Charge_Current_Max = 10;
        RoterData.Mppt_SetPara.Charge_Power_Max = 75;
        RoterData.Mppt_SetPara.Trickle_Current = 0.5f;
        RoterData.Mppt_SetPara.DischarCurve_Moed = 0;

        RoterData.Mppt_SetPara.Current_Gear = 20;
        RoterData.Mppt_SetPara.Ledar_Dly_Time = 15;
        RoterData.Mppt_SetPara.Ledar_Pwm = 10;
        RoterData.Mppt_SetPara.Led_Set_Pwm = 100;
        RoterData.Mppt_SetPara.Low_voltage_Protect = 2.65f;

        RoterData.Mppt_SetPara.Lock_Mode = 0;

        RoterData.Mppt_SetPara.Curv_Data[0][0] = 2;
        RoterData.Mppt_SetPara.Curv_Data[0][1] = 80;

        RoterData.Mppt_SetPara.Curv_Data[1][0] = 4.5f;
        RoterData.Mppt_SetPara.Curv_Data[1][1] = 60;
        
        RoterData.Mppt_SetPara.Curv_Data[2][0] = 7;
        RoterData.Mppt_SetPara.Curv_Data[2][1] = 40; 

        RoterData.Mppt_SetPara.Curv_Data[3][0] = 8.5f;
        RoterData.Mppt_SetPara.Curv_Data[3][1] = 20;
        
        RoterData.Mppt_SetPara.Curv_Data[4][0] = 0;
        RoterData.Mppt_SetPara.Curv_Data[4][1] = 0;
        
        RoterData.Mppt_SetPara.Curv_Data[5][0] = 0;
        RoterData.Mppt_SetPara.Curv_Data[5][1] = 0;
        
        RoterData.Mppt_SetPara.Curv_Data[6][0] = 0;
        RoterData.Mppt_SetPara.Curv_Data[6][1] = 0;
        
        RoterData.Mppt_SetPara.Curv_Data[7][0] = 0;
        RoterData.Mppt_SetPara.Curv_Data[7][1] = 0;

        // char str[10];
        // floatToString(10.0f,1,str);
        // log_info("floatToString%s", str);
        // floatToString(9.0f,1,str);
        // log_info("floatToString%s", str);
        // floatToString(20.0f,1,str);
        // log_info("floatToString%s", str);
        // floatToString(0.118,3,str);
        // log_info("floatToString%s", str);
      
        // ret = syscfg_write(CFG_USER_STRUCT, &RoterData.Mppt_SetPara, sizeof(RoterData.Mppt_SetPara));
        // log_info("%s[CFG_USER_STRUCT -> syscfg_write:%d]", __func__, ret);
        // if(ret != sizeof(RoterData.Mppt_SetPara))
        //     log_error("CFG_USER_STRUCT -> syscfg_write -> err:%d", ret);

        
        
        int ret = 0;
        ret = syscfg_read(CFG_USER_STRUCT, &RoterData.Mppt_SetPara, sizeof(RoterData.Mppt_SetPara) );
        log_info("%s[CFG_USER_STRUCT -> syscfg_read:%d]", __func__, ret);
        if(ret != sizeof(RoterData.Mppt_SetPara))
        {
            log_error("CFG_USER_STRUCT -> syscfg_read -> err:%d", ret);
        }
        else
        {
            log_info("CFG_USER_STRUCT1:%d %d", RoterData.Mppt_SetPara.Current_Gear, RoterData.Mppt_SetPara.Led_Set_Pwm);
        }

       // Mppt_Main_Menu();
          Mppt_Log_Menu();   
      // Mppt_Info_Display(&RoterData.Mppt_Info);
       // Lcd_ShowPicture(0,0,240,240,gImage_image);
        sys_timer_add(NULL,Ble_Timeout_Check,500);
        ir_tx_init();


        Sys_Auto_Off_Timer = sys_timer_add(NULL, Sys_Auto_Off, 60000 * 2);

    #endif
    /* 按键消息使能 */
    sys_key_event_enable();
}

static int multi_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) 
    {
        case APP_STA_CREATE:
            break;
        case APP_STA_START:
            if (!it) 
            {
                break;
            }
            switch (it->action) {
            case ACTION_MULTI_MAIN:
                multi_app_start();
                break;
            }
            break;
        case APP_STA_PAUSE:
            break;
        case APP_STA_RESUME:
            break;
        case APP_STA_STOP:
            break;
        case APP_STA_DESTROY:
            log_info("APP_STA_DESTROY\n");
            break;
    }

    return 0;
}

static int multi_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    #if TCFG_USER_EDR_ENABLE
        bt_comm_edr_hci_event_handler(bt);
    #endif

    #if TCFG_USER_BLE_ENABLE
        bt_comm_ble_hci_event_handler(bt);
    #endif
        return 0;
    }

static int multi_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);

    #if TCFG_USER_EDR_ENABLE
        bt_comm_edr_status_event_handler(bt);
    #endif

    #if TCFG_USER_BLE_ENABLE
        bt_comm_ble_status_event_handler(bt);
    #endif
        return 0;
}

static void multi_key_event_handler(struct sys_event *event)
{
    /* u16 cpi = 0; */  
    u8 event_type = event->u.key.event;
    u8 key_value = event->u.key.value;

    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY)
    {
        sys_timer_re_run(Sys_Auto_Off_Timer);
        log_info("app_key_evnet: %d,%d\n", event_type, key_value);
        if( (event_type == KEY_EVENT_CLICK) & ( key_value == KEY_VALUE_TYPE_ON_OFF ) )
        {
            #if Enable_beep
            Beep_Star();
            #endif

            if(!Power_Flag)
            {
                Power_Flag = 1;
                Power_Lock(); 
                
            }
            // else
            // {
            //     Power_Flag = 0;
            //     Power_UnLock();
            // }
        }
        if(key_value < (KEY_VALUE_TYPE_MAX - 1) )
        {
            if( key_value == KEY_VALUE_TYPE_OFF)
            {
                Power_Flag = 0;
                Power_UnLock();
            }

            #if Enable_beep
            Beep_Star();
            #endif

            if( Menu_Tab[MenuData.current_id].current_operation != NULL) Menu_Tab[MenuData.current_id].current_operation(key_value);
        }
        
//         if (event_type == KEY_EVENT_TRIPLE_CLICK
//             && (key_value == TCFG_ADKEY_VALUE3 || key_value == TCFG_ADKEY_VALUE0)) {
//             //for test
//             multi_set_soft_poweroff();
//             return;
//         }

//         if (event_type == KEY_EVENT_CLICK && key_value == TCFG_ADKEY_VALUE0) {

// #if TCFG_USER_BLE_ENABLE
// #if CONFIG_BT_GATT_CLIENT_NUM
//             multi_client_clear_pair();
// #endif
// #if CONFIG_BT_GATT_SERVER_NUM
//             multi_server_clear_pair();
// #endif
// #endif
//         }


//         if (event_type == KEY_EVENT_DOUBLE_CLICK && key_value == TCFG_ADKEY_VALUE0) {
// #if TCFG_USER_EDR_ENABLE
//             //for test
//             static u8 edr_en = 1;
//             edr_en = !edr_en;
//             bt_comm_edr_mode_enable(edr_en);
// #endif
//         }

//         if (event_type == KEY_EVENT_LONG && key_value == TCFG_ADKEY_VALUE0) {
//             //for test
//             //r_printf("ble_list_clear_all\n");
//             //ble_list_clear_all();
// #if TCFG_USER_BLE_ENABLE
//             static u8 en_value = 1;
//             en_value = !en_value;
//             ble_module_enable(en_value);
// #endif
//             return;
//         }

    }
}

static int multi_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        multi_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
#if (TCFG_USER_EDR_ENABLE || TCFG_USER_BLE_ENABLE)
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            multi_bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            multi_bt_hci_event_handler(&event->u.bt);
        }
#endif
        return 0;

    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, multi_set_soft_poweroff);
        }

#if TCFG_CHARGE_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
            app_charge_event_handler(&event->u.dev);
        }
#endif
        return 0;

    default:
        return FALSE;
    }
    return FALSE;
}

static const struct application_operation app_multi_ops = {
    .state_machine  = multi_state_machine,
    .event_handler 	= multi_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_multi) = {
    .name 	= "multi_conn",
    .action	= ACTION_MULTI_MAIN,
    .ops 	= &app_multi_ops,
    .state  = APP_STA_DESTROY,
};

//-----------------------
//system check go sleep is ok
static u8 multi_state_idle_query(void)
{
    return !is_app_multi_active;
}

REGISTER_LP_TARGET(multi_state_lp_target) = {
    .name = "multi_state_deal",
    .is_idle = multi_state_idle_query,
};

#endif


