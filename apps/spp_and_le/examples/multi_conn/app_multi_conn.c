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


#if CONFIG_APP_MULTI

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
        
       

        spi_open(SPI1);
        ST7789Lcd_Init();
        Beep_init();

        Power_Port_Init();
        Beep_Star();

        for(i=0;i<sizeof(MenuData.index);i++)MenuData.index[i] = 1;
        for(i=0;i<sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]);i++)
            RoterData.Ble_Adv_rp[i].rssi = -99;

        RoterData.Mppt_SetPara.Bat_Capcity = 7.5;
        RoterData.Mppt_SetPara.Charge_Current_Max = 20;
        RoterData.Mppt_SetPara.Charge_Power_Max = 75;
        RoterData.Mppt_SetPara.Trickle_Current = 0.5;
        RoterData.Mppt_SetPara.DischarCurve_Moed = 0;

        RoterData.Mppt_SetPara.Current_Gear = 20;
        RoterData.Mppt_SetPara.Ledar_Dly_Time = 15;
        RoterData.Mppt_SetPara.Ledar_Pwm = 10;
        RoterData.Mppt_SetPara.Led_Set_Pwm = 100;
        RoterData.Mppt_SetPara.Low_voltage_Protect = 2.65;

        RoterData.Mppt_SetPara.Lock_Mode = 0;

        RoterData.Mppt_SetPara.Curv_Data[0][0] = 2;
        RoterData.Mppt_SetPara.Curv_Data[0][1] = 80;

        RoterData.Mppt_SetPara.Curv_Data[1][0] = 4.5;
        RoterData.Mppt_SetPara.Curv_Data[1][1] = 60;
        
        RoterData.Mppt_SetPara.Curv_Data[2][0] = 7;
        RoterData.Mppt_SetPara.Curv_Data[2][1] = 40;

        RoterData.Mppt_SetPara.Curv_Data[3][0] = 8.5;
        RoterData.Mppt_SetPara.Curv_Data[3][1] = 20;
        
        RoterData.Mppt_SetPara.Curv_Data[4][0] = 0;
        RoterData.Mppt_SetPara.Curv_Data[4][1] = 0;
        
        RoterData.Mppt_SetPara.Curv_Data[5][0] = 0;
        RoterData.Mppt_SetPara.Curv_Data[5][1] = 0;
        
        RoterData.Mppt_SetPara.Curv_Data[6][0] = 0;
        RoterData.Mppt_SetPara.Curv_Data[6][1] = 0;
        
        RoterData.Mppt_SetPara.Curv_Data[7][0] = 0;
        RoterData.Mppt_SetPara.Curv_Data[7][1] = 0;

        Mppt_Main_Menu();
        
        // Lcd_ShowPicture(0,0,240,240,gImage_image);
        // sys_timer_add(NULL,Ble_Timeout_Check,2400);

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
        log_info("app_key_evnet: %d,%d\n", event_type, key_value);
        if( (event_type == KEY_EVENT_CLICK) & ( key_value == KEY_VALUE_TYPE_ON_OFF ) )
        {
            Beep_Star();
            if(!Power_Flag)
            {
                Power_Flag = 1;
                Power_Lock(); 
                
            }
            else
            {
                Power_Flag = 0;
                gpio_write(Power_En, 0);
            }
        }
        if(key_value < (KEY_VALUE_TYPE_MAX - 1) )
        {

            Beep_Star();

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


