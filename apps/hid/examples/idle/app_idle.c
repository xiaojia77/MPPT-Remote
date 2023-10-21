#include "system/app_core.h"
#include "system/includes.h"
#include "server/server_core.h"
#include "app_config.h"
#include "app_action.h"

#define LOG_TAG_CONST APP_IDLE
#define LOG_TAG "[APP_IDLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#include "gpio.h"
#include "asm/mcpwm.h"
#include "irkey.h"
//#include "asm/adc_api.h"
//#include "asm/power/p33.h"

#if CONFIG_APP_IDLE

#if TCFG_USER_EDR_ENABLE || TCFG_USER_BLE_ENABLE
// 默认应用不需要打开配置,有需要自己添加
/* #error " confirm, need disable !!!!!!" */
#endif

#define LOCK_EN IO_PORTA_06  // 电源自锁使能
#define CDS_EN IO_PORTB_05   // 充电使能
#define POWER_EN IO_PORTB_00 // 电压检测使能
#define CDS_IN IO_PORTA_05   // 开漏上拉

// BUCK MPPT
#define DC_EN IO_PORTB_04   // DC-DC使能
#define S_EN IO_PORTB_03    // S使能
#define VPWM IO_PORTB_01    // V PWM设置  1000  10K
#define IPWM IO_PORTP_00    // I PWM设置  1000  10K

#define LED1 IO_PORTB_07  // LED1
#define LED2 IO_PORTB_08  // LED2

#define WB_EN IO_PORTB_09 // WB_EN
#define WB_RX IO_PORTA_00 // WB_IO
#define IR_RX IO_PORTA_01    // WB_IO

#define PWM1 IO_PORTA_09  // PWM2
#define PWM2 IO_PORTA_02  // PWM2
#define OUT1 IO_PORTA_03  // out1
#define FPIN IO_PORTA_04  //短路保护用

#define OUTV_ADC_PORT IO_PORT_DP1  //BUCK 输出电压
#define OUTV_ADC_CH AD_CH_DP1  //BUCK 输出电压
#define OUTI_ADC IO_PORT_DM1  //BUCK 输出电流

void Adc_init()
{
    //以下初始化代码添加到void adc_init()里
    u32 gpio = OUTV_ADC_PORT;//指定IO，要和ADC通道对应上
    gpio_set_pull_down(gpio, 0);//看需求是否需要开内部下拉，会有20%的误差，不同芯片理论电阻值不一样，看规格书说明
    gpio_set_pull_up(gpio, 0);//看需求是否需要开内部上拉，会有20%的误差，不同芯片理论电阻值不一样，看规格书说明
    gpio_set_die(gpio, 0);//模拟态
    gpio_set_dieh(gpio, 0);//模拟态
    gpio_set_direction(gpio, 1);//输入态
    u32 ch = OUTV_ADC_CH; ///指定ADC通道，要和GPIO对应上
    adc_add_sample_ch(ch);//添加检测通道
    #define CH_SAMPLE_FREQ    100 //ms
    adc_set_sample_freq(ch, CH_SAMPLE_FREQ);//设置固定的采样频率，一般不设置，就是最高的轮询时间
}

void adc_handle(void)
{
    u32 voltage = adc_get_voltage(OUTV_ADC_CH);//一定要用带校准值换算的接口，电压值才是准确的
      log_info("OUTV_ADC_CH: %d", voltage);
}

void pwm_init(void)
{
    struct pwm_platform_data pwm_p_data;
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch0;                 //通道0
    pwm_p_data.frequency = 10000;                    //1KHz
    pwm_p_data.duty = 5000;                          //占空比50%
    pwm_p_data.h_pin = -1;                           //不用则填 -1
    pwm_p_data.l_pin = VPWM;                         //硬件引脚
    mcpwm_init(&pwm_p_data);
    
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch1;                 //通道0
    pwm_p_data.frequency = 10000;                    //1KHz
    pwm_p_data.duty = 10000;                          //占空比50%
    pwm_p_data.h_pin = -1;                           //不用则填 -1
    pwm_p_data.l_pin = IPWM;                         //硬件引脚
    mcpwm_init(&pwm_p_data);

    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch2;                 //通道0
    pwm_p_data.frequency = 10000;                    //1KHz
    pwm_p_data.duty = 0;                          //占空比50%
    pwm_p_data.h_pin = -1;                           //不用则填 -1
    pwm_p_data.l_pin = PWM1;                         //硬件引脚
    mcpwm_init(&pwm_p_data);

    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_ch_num = pwm_ch3;                 //通道0
    pwm_p_data.frequency = 10000;                    //1KHz
    pwm_p_data.duty = 10000;                          //占空比50%
    pwm_p_data.h_pin = -1;                           //不用则填 -1
    pwm_p_data.l_pin = PWM2;                         //硬件引脚
    mcpwm_init(&pwm_p_data);

}

void Buck_Init()
{    
    
    usb1_iomode(1); //设置USB为普通IO

    gpio_set_pull_up(IO_PORT_DM1, 0);
    gpio_set_pull_down(IO_PORT_DM1, 0);
    gpio_set_direction(IO_PORT_DM1, 1);  
    gpio_set_die(IO_PORT_DM1,0);
    
    gpio_set_pull_up(IO_PORT_DP1, 0);
    gpio_set_pull_down(IO_PORT_DP1, 0);
    gpio_set_direction(IO_PORT_DP1, 1); 
    gpio_set_die(IO_PORT_DP1,0);
    
    gpio_set_pull_up(LOCK_EN, 0);
    gpio_set_pull_down(LOCK_EN, 0);
    gpio_set_direction(LOCK_EN, 0); 
    gpio_write(LOCK_EN, 1);

    gpio_set_pull_up(CDS_IN, 1);
    gpio_set_pull_down(CDS_IN, 0);
    gpio_set_direction(CDS_IN, 1); 

    gpio_set_pull_up(LED1, 0);
    gpio_set_pull_down(LED1, 0);
    gpio_set_direction(LED1, 0); 
    gpio_write(LED1, 1);
    
    gpio_set_pull_up(LED2, 0);
    gpio_set_pull_down(LED2, 0);
    gpio_set_direction(LED2, 0); 
    gpio_write(LED2, 1);
    


    gpio_set_pull_up(CDS_EN, 0);
    gpio_set_pull_down(CDS_EN, 0);
    gpio_set_direction(CDS_EN, 0); 
    gpio_write(CDS_EN, 1);

    gpio_set_pull_up(POWER_EN, 0);
    gpio_set_pull_down(POWER_EN, 0);
    gpio_set_direction(POWER_EN, 0);
    gpio_write(POWER_EN, 1);

    gpio_set_pull_up(DC_EN, 0);
    gpio_set_pull_down(DC_EN, 0);
    gpio_set_direction(DC_EN, 0); 
    gpio_write(DC_EN, 1);

    gpio_set_pull_up(S_EN, 0);
    gpio_set_pull_down(S_EN, 0);
    gpio_set_direction(S_EN, 0); 
    gpio_write(S_EN, 1);

    pwm_init();

 /*   gpio_set_pull_up(VPWM, 0);
    gpio_set_pull_down(VPWM, 0);
    gpio_set_direction(VPWM, 0);

    gpio_set_direction(IPWM, 0);
    gpio_set_direction(IPWM, 0);
    gpio_set_direction(IPWM, 0);
    gpio_write(VPWM, 1);
    gpio_write(IPWM, 1);
    
    */
}

static void idle_timer_handle_test(void)
{
    static uint8_t flashflag;
    flashflag = !flashflag;
    gpio_write(LED1, flashflag);
    gpio_write(LED2, !flashflag);
    adc_handle();
}

static void idle_app_start()
{
    log_info("=======================================");
    log_info("---------idle demo---------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

    clk_set("sys", BT_NORMAL_HZ);

   // gpio_set_pull_up(LOCK_EN, 0);
   // gpio_set_pull_down(LOCK_EN, 0);
   // gpio_set_direction(LOCK_EN, 0); 
   // gpio_write(LOCK_EN, 1);

   // os_time_dly(300);
    
    

    Buck_Init();

    Adc_init();
   // irkey_init(&ir_data);
    //系统按键使能
    //sys_key_event_enable();

    sys_timer_add(NULL, idle_timer_handle_test, 500);

    sys_key_event_enable();
    //sys_timer_add(NULL, ir_handle, 10);
}

static void key_event_handler(struct sys_event *event)
{
    /* u16 cpi = 0; */
    u8 event_type = 0;
    u8 key_value = 0;

    static u16 Vduty = 5000;   
    static u16 Iduty = 5000;   

    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_evnet: %d,%d\n", event_type, key_value);
        if(key_value == 7)
        {
            if(Vduty < 10000)Vduty += 200;
        }
        if(key_value == 9)
        {
            if(Vduty)Vduty -= 200;
           
        } 
        if(key_value == 8)
        {
            if(Iduty < 10000)Iduty += 200;
        }
        if(key_value == 90)
        {
             if(Iduty)Iduty -= 200;
        }

        mcpwm_set_duty(pwm_ch0,Vduty);
        mcpwm_set_duty(pwm_ch1,Iduty);
       // hidkey_app_key_deal_test(event_type, key_value);
    }
}

static int idle_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type)
    {
        case SYS_KEY_EVENT: 
            key_event_handler(event);
            return 0;
        case SYS_BT_EVENT:
            return 0;
        case SYS_DEVICE_EVENT:
            return 0;
        case SYS_IR_EVENT:
          //  ir_handle();
            return 0;
            break;
        default:
            return false;
    }
}

static int idle_state_machine(struct application *app, enum app_state state,
                              struct intent *it)
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
            switch (it->action)
            {
            case ACTION_IDLE_MAIN:
                log_info("ACTION_IDLE_MAIN\n");
                /* os_taskq_flush(); */
                idle_app_start();
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
            break;
    }

    return 0;
}

static const struct application_operation app_idle_ops = {
    .state_machine = idle_state_machine,
    .event_handler = idle_event_handler,
};

REGISTER_APPLICATION(app_app_idle) = {
    .name = "idle",
    .action = ACTION_IDLE_MAIN,
    .ops = &app_idle_ops,
    .state = APP_STA_DESTROY,
};

#endif
