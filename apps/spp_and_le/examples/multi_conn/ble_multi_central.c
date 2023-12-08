#include "system/app_core.h"
#include "system/includes.h"

#include "app_config.h"
#include "app_action.h"

#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "user_cfg.h"
#include "vm.h"
#include "btcontroller_modules.h"
#include "bt_common.h"
#include "3th_profile_api.h"
#include "le_common.h"
#include "rcsp_bluetooth.h"
#include "JL_rcsp_api.h"
#include "custom_cfg.h"
#include "btstack/btstack_event.h"
#include "ble_multi.h"
#include "le_client_demo.h"
#include "gatt_common/le_gatt_common.h"

#include "lcd7789.h"

#include "malloc.h"


#if CONFIG_APP_MULTI && CONFIG_BT_GATT_CLIENT_NUM

#if LE_DEBUG_PRINT_EN
#define log_info(x, ...)  printf("[MUL-CEN]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  put_buf

#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

//搜索类型
#define SET_SCAN_TYPE       SCAN_ACTIVE   // 主动扫描
//搜索 周期大小
#define SET_SCAN_INTERVAL   ADV_SCAN_MS(24) // unit: 0.625ms
//搜索 窗口大小
#define SET_SCAN_WINDOW     ADV_SCAN_MS(8)  // unit: 0.625ms, <= SET_SCAN_INTERVAL

//连接周期
#define BASE_INTERVAL_MIN   (6)//最小的interval
#define SET_CONN_INTERVAL   (BASE_INTERVAL_MIN*8) //(unit:1.25ms)
//连接latency
#define SET_CONN_LATENCY    0  //(unit:conn_interval)
//连接超时
#define SET_CONN_TIMEOUT    400 //(unit:10ms)

//建立连接超时
#define SET_CREAT_CONN_TIMEOUT    8000 //(unit:ms)

//配对信息表
#define CLIENT_PAIR_BOND_ENABLE    0 //CONFIG_BT_SM_SUPPORT_ENABLE
#define CLIENT_PAIR_BOND_TAG       0x56

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))
uint32_t const bl_ckey[4]= {12,34,56,78}; 

void btea(uint32_t *v, int n, uint32_t const key[4])
{
    uint32_t y, z, sum;
    unsigned p, rounds, e;
    if (n > 1)            /* Coding Part */
    {
        rounds = 6 + 52/n;
        sum = 0;
        z = v[n-1];
        do
        {
            sum += DELTA;
            e = (sum >> 2) & 3;
            for (p=0; p<n-1; p++)
            {
                y = v[p+1];
                z = v[p] += MX;
            }
            y = v[0];
            z = v[n-1] += MX;
        }
        while (--rounds);
    }
    else if (n < -1)      /* Decoding Part */
    {
        n = -n;
        rounds = 6 + 52/n;
        sum = rounds*DELTA;
        y = v[0];
        do
        {
            e = (sum >> 2) & 3;
            for (p=n-1; p>0; p--)
            {
                z = v[p-1];
                y = v[p] -= MX;
            }
            z = v[n-1];
            y = v[0] -= MX;
            sum -= DELTA;
        }
        while (--rounds);
    }
}


struct ctl_pair_info_t 
{
    u8 head_tag;        
    u8 match_dev_id;   // 设备的UUID
    u8 pair_flag;      // 配对FLAG
    u8 peer_address_info[7];
    u16 conn_handle;   // 连接handle
    u16 conn_interval; // 连接间隔
    u16 conn_latency;  // 连接延迟
    u16 conn_timeout;  // 连接超时
};

/* static u8 cur_peer_addr_info[7];    //当前连接对方地址信息 */
/* static u8 client_pair_bond_info[8]; //tag + addr_type + address */
static struct ctl_pair_info_t cur_conn_info; //当前连接的信息
static struct ctl_pair_info_t record_bond_info[SUPPORT_MAX_GATT_CLIENT]; //绑定的值

static u8 multi_pair_reconnect_search_profile = 1; /*配对回连是否搜索profile*/
static u8 multi_sm_master_pair_redo; /*配对回连keymiss,是否重新执行配对*/

static int multi_client_event_packet_handler(int event, u8 *packet, u16 size, u8 *ext_param);
static void multi_scan_conn_config_set(struct ctl_pair_info_t *pair_info);

const gatt_client_cfg_t mul_client_init_cfg = 
{
    .event_packet_handler = multi_client_event_packet_handler,
};
//---------------------------------------------------------------------------

static scan_conn_cfg_t multi_client_scan_cfg;
static u16 multi_ble_client_write_handle;

#define MULTI_TEST_WRITE_SEND_DATA            1 //测试发数
//---------------------------------------------------------------------------
//指定搜索uuid
//指定搜索uuid
static const target_uuid_t  jl_multi_search_uuid_table[] = 
{
    // for uuid16
    // PRIMARY_SERVICE, ae30
    // CHARACTERISTIC,  ae01, WRITE_WITHOUT_RESPONSE | DYNAMIC,
    // CHARACTERISTIC,  ae02, NOTIFY,
    {
        .services_uuid16 = 0xae00,
        .characteristic_uuid16 = 0xae01,
        .opt_type = ATT_PROPERTY_WRITE_WITHOUT_RESPONSE,
    },
    {
        .services_uuid16 = 0xae00,
        .characteristic_uuid16 = 0xae02,
        .opt_type = ATT_PROPERTY_NOTIFY,
    },
};

//配置多个扫描匹配设备
static const u8 cetl_test_remoter_name1[] ="SQ20P75SA-B";//
static const client_match_cfg_t multi_match_device_table[] = 
{
    {
        .create_conn_mode = BIT(CLI_CREAT_BY_NAME),
        .compare_data_len = sizeof(cetl_test_remoter_name1) - 1, //去结束符
        .compare_data = cetl_test_remoter_name1,
        .filter_pdu_bitmap = 0,
    },
    
    {
        .create_conn_mode = BIT(CLI_CREAT_BY_NAME),
        .compare_data_len = sizeof(cetl_test_remoter_name1) - 1, //去结束符
        .compare_data = cetl_test_remoter_name1,
        .filter_pdu_bitmap = 0,
    },
};

//带绑定的设备搜索
static client_match_cfg_t *multi_bond_device_table;
static u16  bond_device_table_cnt; // 绑定表计数

#define DATA_PACKET_MAX 512  // MTU最大值

static inline u8 make_packet_data(u8 *buf, u16 offset, u8 data_type, u8 *data, u8 data_len)
{
    if (DATA_PACKET_MAX - offset < data_len + 2) {
        return offset + data_len + 2;
    }

    buf[0] = data_len + 1;
    buf[1] = data_type;
    memcpy(buf + 2, data, data_len);
    return data_len + 2;
}

static inline u8 make_packet_val(u8 *buf, u16 offset, u8 data_type, u32 val, u8 val_size)
{
    if (DATA_PACKET_MAX - offset < val_size + 2) {
         return offset + val_size + 2;
    }

    buf[0] = val_size + 1;
    buf[1] = data_type;
    memcpy(buf + 2, &val, val_size);
    return val_size + 2;
}
//测试write数据操作
void Get_Mppt_Report1(void) // 获取MPPT 信息的指令
{
    uint32_t i, ret = 0;
    uint32_t encry_key[4] ;     //通过+MAC地址来改变密钥
    uint16_t offset = 0;
    uint16_t tmp_handle;

    uint8_t data[8]  = //发送查询指令
    {
        0XCC,0X55,0x00,0x00,0x00,0x00,0x00,0x00
    };
    for(i=0;i<4;i++) encry_key[i] = bl_ckey[i];
    for(i=0;i<6;i++) encry_key[i%4] += RoterData.Ble_Connect_Mac[i];

    data[0] = 0xCC; //包头
    data[1] = 0X55;//包尾

    // put_buf(RoterData.Ble_Connect_Mac,6); // 蓝牙地址
    // put_buf(encry_key,16); // 加密密钥

    // put_buf(data,8);
    btea(data,2,encry_key); //加密数据
    //put_buf(data,8);

    for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) 
    {
        tmp_handle = ble_comm_dev_get_handle(i, GATT_ROLE_CLIENT); //获取连接的handle
        if (tmp_handle && multi_ble_client_write_handle) 
        {
            ret = ble_comm_att_send_data(tmp_handle, multi_ble_client_write_handle, data, 8, ATT_OP_WRITE_WITHOUT_RESPOND);
            log_info("test_write:%04x,%d", tmp_handle, ret);
        }
    }
}

void Get_Mppt_Report(void) // 获取MPPT 信息的指令
{
    uint32_t i, ret = 0;
    uint32_t encry_key[4] ;     //通过+MAC地址来改变密钥
    uint16_t offset = 0;
    uint16_t tmp_handle;

    uint8_t data[8]  = //发送查询指令
    {
        0XBB,0X55,0x00,0x00,0x00,0x00,0x00,0x00
    };
    for(i=0;i<4;i++) encry_key[i] = bl_ckey[i];
    for(i=0;i<6;i++) encry_key[i%4] += RoterData.Ble_Connect_Mac[i];

    data[0] = 0xBB; //包头
    data[1] = 0X55;//包尾

    // put_buf(RoterData.Ble_Connect_Mac,6); // 蓝牙地址
    // put_buf(encry_key,16); // 加密密钥

    // put_buf(data,8);
     btea(data,2,encry_key); //加密数据
    // put_buf(data,8);

    for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) 
    {
        tmp_handle = ble_comm_dev_get_handle(i, GATT_ROLE_CLIENT); //获取连接的handle
        if (tmp_handle && multi_ble_client_write_handle) 
        {
            ret = ble_comm_att_send_data(tmp_handle, multi_ble_client_write_handle, data, 8, ATT_OP_WRITE_WITHOUT_RESPOND);
            log_info("test_write:%04x,%d", tmp_handle, ret);
        }
    }
}

void Mppt_Set_Para_Send(void *priv) // 发MPPT的设置参数
{
    uint32_t i, ret = 0;
    uint8_t *data ;
    uint32_t encry_key[4] ;     //通过+MAC地址来改变密钥
    uint16_t offset = 0;
    uint16_t tmp_handle;

    Mppt_Set_Parm_t *SetParm = priv;

    if( priv == NULL ) 
    {
        log_info("SetParm Null");
        return;
    }

    

    uint16_t  Curv_Data[8][2]=   // 曲线数据  单位S 占空比%
    {
       // {60*60*1,90},{60*60*2,80},{60*60*3,70},{60*60*4,60},
       // {60*60*5,50},{60*60*6,40},{60*60*7,30},{60*60*8,20},
    };

    for(i=0;i<8;i++)
    {
        Curv_Data[i][0] = 60*60*SetParm->Curv_Data[i][0];
        Curv_Data[i][1] = SetParm->Curv_Data[i][1];

         log_info("curv %d %d", Curv_Data[i][0], Curv_Data[i][1]);
    }

    uint16_t const Time_Data[3]=   // 日期数据  年月日
    {
        2023,11,18
    };

    for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) 
    {
        tmp_handle = ble_comm_dev_get_handle(i, GATT_ROLE_CLIENT); //获取连接的handle
        if (tmp_handle && multi_ble_client_write_handle) 
        {
            data = malloc(256);
            mem_stats();
            for(i=0;i<4;i++) encry_key[i] = bl_ckey[i]; // 密码获取
            for(i=0;i<6;i++) encry_key[i%4] += RoterData.Ble_Connect_Mac[i]; //加密密钥

            data[offset++] = 0xAA; //包头
            offset += make_packet_val(&data[offset],offset,0x01,SetParm->Bat_Capcity*1000,4); // 发10A
            offset += make_packet_val(&data[offset],offset,0x02,SetParm->Charge_Current_Max*1000,4); // 发5A  0.5C;
            offset += make_packet_val(&data[offset],offset,0x03,SetParm->Charge_Power_Max*1000,4); // 发50W 
            offset += make_packet_val(&data[offset],offset,0x04,SetParm->Trickle_Current*1000,4);   // 发500mah
            offset += make_packet_val(&data[offset],offset,0x05,SetParm->Low_voltage_Protect*1000,4);  // 发2600MV
            offset += make_packet_val(&data[offset],offset,0x06,SetParm->Current_Gear,4);       // 发20挡位
            offset += make_packet_val(&data[offset],offset,0x07,SetParm->Ledar_Pwm,4);          // 雷达10%
            offset += make_packet_val(&data[offset],offset,0x08,SetParm->Ledar_Dly_Time,4);     // 雷达延迟15S
            offset += make_packet_val(&data[offset],offset,0x09,SetParm->Led_Set_Pwm,4);        // 默认亮度100%
            offset += make_packet_val(&data[offset],offset,0x0A,SetParm->DischarCurve_Moed,4);  // 曲线模式0 pwm模式
            offset += make_packet_data(&data[offset],offset,0x0B,&Curv_Data,sizeof(Curv_Data));               // 默认曲线
            offset += make_packet_val(&data[offset],offset,0x0C,SetParm->Lock_Mode,4);          // 锁定模式
            offset += make_packet_val(&data[offset],offset,0x0D,SetParm->Usercode,4);           // 用户数据
            offset += make_packet_data(&data[offset],offset,0x0E,&Time_Data,sizeof(Time_Data));               // 时间参数
            offset += make_packet_val(&data[offset],offset,0x0F,SetParm->Solar_Mode,4);         // 太阳能模式
            offset += make_packet_val(&data[offset],offset,0x10,SetParm->Extern_Mode,4);        // 外部通信方式

            data[offset++] = 0X55;//包尾
            for(i=0;i<offset%4;i++)
            {
                data[offset] = 0;//补0
                offset++;
            }
            log_info("MPPT Set Para Data len:%d \r\n",offset);
            put_buf(RoterData.Ble_Connect_Mac,6); // 蓝牙地址
            put_buf(encry_key,16);                // 加密密钥
        
            put_buf(data,offset);           //打印原始数据
            btea(data,offset/4 ,encry_key); //加密数据
            put_buf(data,offset);

            ret = ble_comm_att_send_data(tmp_handle, multi_ble_client_write_handle, data, offset , ATT_OP_WRITE_WITHOUT_RESPOND);
            log_info("test_write:%04x,%d", tmp_handle, ret);
            free(data);
            mem_stats();
        }
    }
    
  
}

static u16 Get_Info_Timer = 0;
static void  MPPT_Get_Info_Timer_Star(void)
{
    log_info("%s[id:%d] star", __func__, Get_Info_Timer);
    if(Get_Info_Timer)
    {
        sys_timer_del(Get_Info_Timer);
        Get_Info_Timer = 0; 
    }   
    Get_Info_Timer = sys_timer_add(NULL, Get_Mppt_Report1, 1000);
}

static void Get_Info_Timer_Del(void)
{
    log_info("%s[id:%d]", __func__, Get_Info_Timer);
    if(Get_Info_Timer)
    {
        sys_timer_del(Get_Info_Timer);
        Get_Info_Timer = 0;
    }
}

void Mppt_Data_Decode(u8 *packet,u16 size) // MPPT 信息解码
{
    uint8_t *data;
    uint32_t i,encry_key[4],temp;     //通过+MAC地址来改变密钥
    u8 command;
    u8 data_i = 0 , next_len;
    uint16_t data_position = 0;
    struct sys_event e;
    if(size < 8)
    {
        log_info("recive data len < 8\r\n");
        return ; //数据长度不够直接退出 
    }
    data = malloc(256);
    for(i=0;i<4;i++) encry_key[i] = bl_ckey[i];
    for(i=0;i<6;i++) encry_key[i%4] += RoterData.Ble_Connect_Mac[i];

    put_buf(RoterData.Ble_Connect_Mac,6); // 蓝牙地址
    put_buf(encry_key,16); // 加密密钥

    memcpy(data,packet,size);
    put_buf(data,size);
    btea(data,-(size/4),encry_key); //加密数据
    put_buf(data,size);

    switch (data[data_i++])
    {
        case 0xAA:  // 设置参数返回
            log_info("find data head 0xAA\r\n");
            if(data[data_i] == 0x00) 
            {
                log_info("Set Para Success\r\n");
            }
            if(data[data_i] == 0x01) log_info("invail connect\r\n");
            if(data[data_i] == 0x02) log_info("invail data len\r\n");
            if(data[data_i] == 0x03) log_info("head error\r\n");
            if(data[data_i] == 0x04) log_info("invail Curve Data\r\n");
            if(cur_conn_info.conn_handle)
            {
                uint8_t location;

                if(MenuData.current_id == ENTRY_MODIFY)
                {
                   // location = Ble_Find_Mac_RepAddr(RoterData.Ble_Adv_rp,sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]),RoterData.Ble_SetConnect_Mac);
                    e.type = SYS_KEY_EVENT;
                    e.u.key.init = 1;
                    e.u.key.type = KEY_DRIVER_TYPE_AD;//区分按键类型
                    e.u.key.event = 0;
                    e.u.key.value = KEY_VALUE_TYPE_MODIFY_OK;
                    e.arg  = (void *)DEVICE_EVENT_FROM_KEY;
                    sys_event_notify(&e);

                }
                else
                {
                    location = Ble_Find_Mac_RepAddr(RoterData.Ble_Adv_rp,sizeof(RoterData.Ble_Adv_rp)/sizeof(RoterData.Ble_Adv_rp[0]),RoterData.Ble_Connect_Mac);
                    
                    if(!RoterData.Ble_Adv_rp[location].IsModifyFlag)
                    {
                        RoterData.Ble_Adv_rp[location].IsModifyFlag = 1; 

                        e.type = SYS_KEY_EVENT;
                        e.u.key.init = 1;
                        e.u.key.type = KEY_DRIVER_TYPE_AD;//区分按键类型
                        e.u.key.event = 0;
                        e.u.key.value = KEY_VALUE_TYPE_MODIFY_OK;
                        e.arg  = (void *)DEVICE_EVENT_FROM_KEY;
                        sys_event_notify(&e);
                        RoterData.SetCount++;
                    }

                    if(MenuData.current_id != ENTRY_MODIFY)
                        ble_comm_disconnect(cur_conn_info.conn_handle);
                }
                
            }
           // ble_gatt_client_disconnect_all();
            break;
        case 0XBB: // 信息参数返回
            log_info("find data head 0xBB\r\n");
            while(data_i<size - 1)
            {
                next_len = data[data_i] + 1; //长度
                if(next_len<2)break; //非法数据 
                command = data[data_i + 1];  //命令
                if(!command)break;
                data_position = data_i + 2;
                //log_info("cmd:%d next_len:%d ",command,next_len);
                switch (command)
                {
                    case 0x01:
                        temp = little_endian_read_32(&data[data_position],0); 
                        log_info("charge capcity data:%xMA ",temp);
                        RoterData.Mppt_Info.Charge_Capcity = temp;
                        if(RoterData.Mppt_Info.Charge_Capcity)
                            RoterData.Mppt_Info.Charge_Capcity /= 1000;
                        break;
                    case 0x02:
                        temp = little_endian_read_32(&data[data_position],0);     
                        log_info("charge current data:%xMA ",temp);
                        RoterData.Mppt_Info.Charge_Current = temp;
                        if(RoterData.Mppt_Info.Charge_Current)
                            RoterData.Mppt_Info.Charge_Current /= 1000;
                        break;
                    case 0x03: 
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("charge power:%xW ",temp); 
                        RoterData.Mppt_Info.Charge_Power = temp;
                        if(RoterData.Mppt_Info.Charge_Power)
                            RoterData.Mppt_Info.Charge_Power /= 1000;
                        break;
                    case 0x04:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Dischar_Current :%xma ",temp);
                        RoterData.Mppt_Info.Dischar_Current = temp;
                        if(RoterData.Mppt_Info.Dischar_Current)
                            RoterData.Mppt_Info.Dischar_Current /= 1000;
                        break;
                    case 0x05:
                        temp = little_endian_read_32(&data[data_position],0); 
                        log_info("Bat_Resistance :%xMR ",temp);
                        RoterData.Mppt_Info.Bat_Resistance = temp; 
                        break;
                    case 0x06:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Bat_Capcity :%x%% ",temp);
                        RoterData.Mppt_Info.Bat_Capcity = temp;
                        break;
                    case 0x07:
                        temp = little_endian_read_32(&data[data_position],0);     
                        log_info("OutPut_Staus :%d ",temp);
                        RoterData.Mppt_Info.OutPut_Staus = temp;
                        break;
                    case 0x08:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Bat Volatge :%dR ",temp);
                        RoterData.Mppt_Info.Bat_Voltage = temp;            
                        if(RoterData.Mppt_Info.Bat_Voltage)
                            RoterData.Mppt_Info.Bat_Voltage /= 1000;
                        break;
                    default:    
                        break;
                }
                data_i = data_i+next_len;
            } 
            break;
        case 0xCC:  // 返回设置的信息
            log_info("find data head 0xCC\r\n");
            Get_Info_Timer_Del(); 
            while(data_i<size)
            {
                next_len = data[data_i] + 1; //长度
                if(next_len<2)break; //非法数据 
                command = data[data_i + 1];  //命令
                if(!command)break;
                data_position = data_i + 2;
              //  log_info("cmd:%d next_len:%d ",command,next_len);
                switch (command)
                {
                    case 0x01:
                        temp = little_endian_read_32(&data[data_position],0);     
                        log_info("bat capcity data:%dMA/H ",temp);
                        RoterData.Mppt_ConSetPara_Info.Bat_Capcity = temp;
                        if(RoterData.Mppt_ConSetPara_Info.Bat_Capcity)
                            RoterData.Mppt_ConSetPara_Info.Bat_Capcity /= 1000;
                        break;
                    case 0x02:
                        temp = little_endian_read_32(&data[data_position],0);     
                        log_info("Max charge current data:%dMA ",temp);
                        RoterData.Mppt_ConSetPara_Info.Charge_Current_Max = temp;
                        if(RoterData.Mppt_ConSetPara_Info.Charge_Current_Max)
                            RoterData.Mppt_ConSetPara_Info.Charge_Current_Max /= 1000;
                        break;
                    case 0x03: 
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Max charge power:%dW ",temp);
                        RoterData.Mppt_ConSetPara_Info.Charge_Power_Max = temp;
                        if(RoterData.Mppt_ConSetPara_Info.Charge_Power_Max)
                            RoterData.Mppt_ConSetPara_Info.Charge_Power_Max /= 1000;
                        break;
                    case 0x04:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Trickle_Current :%dA ",temp);
                        RoterData.Mppt_ConSetPara_Info.Trickle_Current = temp;
                        if(RoterData.Mppt_ConSetPara_Info.Trickle_Current)
                            RoterData.Mppt_ConSetPara_Info.Trickle_Current /= 1000;
                        break;
                    case 0x05:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Low Percent :%dV ",temp);
                        RoterData.Mppt_ConSetPara_Info.Low_voltage_Protect = temp;
                        if(RoterData.Mppt_ConSetPara_Info.Low_voltage_Protect)
                            RoterData.Mppt_ConSetPara_Info.Low_voltage_Protect /= 1000;
                        break;
                    case 0x06:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Current Gear : ",temp);
                        RoterData.Mppt_ConSetPara_Info.Current_Gear = temp;
                        break;
                    case 0x07:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Ladear On Pwm :%d%% ",temp);
                        RoterData.Mppt_ConSetPara_Info.Ledar_Pwm = temp;
                        break;
                    case 0x08:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Ladear Check Time:%ds ",temp);
                        RoterData.Mppt_ConSetPara_Info.Ledar_Dly_Time = temp;
                        break;
                    case 0x09:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Led Default Pwm:%d%% ",temp);
                        RoterData.Mppt_ConSetPara_Info.Led_Set_Pwm = temp;
                        break;
                    case 0x0A:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("DischarCurve_Moed Mode:%d%% ",temp);
                        RoterData.Mppt_ConSetPara_Info.DischarCurve_Moed = temp;
                        break;
                    case 0x0B:
                            temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Curve Data:%d%% ",temp);    
                        for(int i=0;i<8;i++)
                        {
                           RoterData.Mppt_ConSetPara_Info.Curv_Data[i][0] = little_endian_read_16(&data[data_position],(i*2)*2) / 3600.;  
                           RoterData.Mppt_ConSetPara_Info.Curv_Data[i][1] = little_endian_read_16(&data[data_position],(i*2+1)*2);  
                        }
                        break;
                    case 0x0C:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Lock Mode:%d%% ",temp);
                        RoterData.Mppt_ConSetPara_Info.Lock_Mode = temp;
                        break;
                    case 0x0D:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Roter UserCode:%d%% ",temp);
                        break;
                    case 0x0E: // 日期
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Time Data:%d%% ",temp);
                        break;
                    case 0x0F: //太阳能
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Solar Mode:%d%% ",temp);
                        RoterData.Mppt_ConSetPara_Info.Solar_Mode = temp;
                        break;
                    case 0x10:
                        temp = little_endian_read_32(&data[data_position],0);  
                        log_info("Extern_Mode Mode:%d%% ",temp);
                         RoterData.Mppt_ConSetPara_Info.Extern_Mode = temp;
                        break;
                    case 0x11:
                        break;

                    default:    
                        break;
                }
                data_i = data_i+next_len;
            } 
            break;
        default:
            break;
    }
    free(data);
}

//配置扫描匹配连接的设备，已经连上后搜索匹配的profile uuid
static const gatt_search_cfg_t multil_client_search_config = 
{
    .match_devices = multi_match_device_table,
    .match_devices_count = (sizeof(multi_match_device_table) / sizeof(client_match_cfg_t)),
    .match_rssi_enable = 0,

    .search_uuid_group = jl_multi_search_uuid_table,
    .search_uuid_count = (sizeof(jl_multi_search_uuid_table) / sizeof(target_uuid_t)),
    .auto_enable_ccc = 1,
};

//配置扫描匹配连接绑定后的设备
static gatt_search_cfg_t multil_client_bond_config = 
{
    .match_devices = NULL,
    .match_devices_count = 0,
    .match_rssi_enable = 0,

    .search_uuid_group = jl_multi_search_uuid_table,
    .search_uuid_count = (sizeof(jl_multi_search_uuid_table) / sizeof(target_uuid_t)),
    .auto_enable_ccc = 1,
};

/*更新连接设备的匹配配置*/
static void multi_client_reflash_bond_search_config(void)
{
    int i;

    if (!multi_bond_device_table) {
        log_info("device tabl is null!");
        return;
    }

    /*建立对应关系 配对表(匹配地址) + 默认匹配表 multi_match_device_table*/
    for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
        multi_bond_device_table[i].create_conn_mode = BIT(CLI_CREAT_BY_ADDRESS);
        multi_bond_device_table[i].compare_data_len = 6;
        multi_bond_device_table[i].compare_data = &record_bond_info[i].peer_address_info[1];

        if (CLIENT_PAIR_BOND_TAG == record_bond_info[i].head_tag) {
            r_printf("set bond search: %d\n", i);
            multi_bond_device_table[i].filter_pdu_bitmap = BIT(EVENT_ADV_SCAN_IND) | BIT(EVENT_ADV_NONCONN_IND);
        } else {
            multi_bond_device_table[i].filter_pdu_bitmap = EVENT_DEFAULT_REPORT_BITMAP;
        }
    }
    memcpy(&multi_bond_device_table[SUPPORT_MAX_GATT_CLIENT], multi_match_device_table, sizeof(multi_match_device_table));

    multil_client_bond_config.match_devices = multi_bond_device_table;
    multil_client_bond_config.match_devices_count = bond_device_table_cnt;
    ble_gatt_client_set_search_config(&multil_client_bond_config);
}

//-------------------------------------------------------------------------------------
//vm 绑定对方信息读写
static int multi_client_pair_vm_do(struct ctl_pair_info_t *info, u8 rw_flag)
{
    int ret = 0;

#if CLIENT_PAIR_BOND_ENABLE
    int i;
    int uint_len = sizeof(struct ctl_pair_info_t);

    log_info("-conn_pair_info vm_do:%d\n", rw_flag);

    if (rw_flag == 0) {
        ret = syscfg_read(CFG_BLE_BONDING_REMOTE_INFO2, (u8 *)&record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
        if (!ret) {
            log_info("-null--\n");
            memset((u8 *)&record_bond_info, 0xff, uint_len * SUPPORT_MAX_GATT_CLIENT);
            ret = 1;
        }

        /*检查合法性*/
        for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
            if (CLIENT_PAIR_BOND_TAG != record_bond_info[i].head_tag || record_bond_info[i].pair_flag != 1) {
                memset((u8 *)&record_bond_info[i], 0xff, uint_len);
            }
        }

    } else {
        int fill_index = -1;

        if (info == NULL) {
            log_info("vm clear\n");
            memset((u8 *)&record_bond_info, 0xff, uint_len * SUPPORT_MAX_GATT_CLIENT);
        } else {

            for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                if (0 == memcmp(info, &record_bond_info[i], uint_len)) {
                    log_info("dev in table\n");
                    return ret;
                }
            }

            put_buf((u8 *)&record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
            put_buf((u8 *)info, uint_len);
            r_printf("write vm start\n");
            log_info("find table\n");

            /*find same*/
            if (fill_index == -1) {
                for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                    if (0 == memcmp(info->peer_address_info, &record_bond_info[i].peer_address_info, 7)) {
                        log_info("replace old\n");
                        fill_index = i;
                        break;
                    }
                }
            }

            /*find idle*/
            if (fill_index == -1) {
                for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                    if (CLIENT_PAIR_BOND_TAG != record_bond_info[i].head_tag) {
                        log_info("find idle\n");
                        fill_index = i;
                        break;
                    }
                }
            }

            /*find first*/
            if (fill_index == -1) {
                for (i = 1; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                    memcpy(&record_bond_info[i - 1], &record_bond_info[i], uint_len);
                    record_bond_info[i - 1].match_dev_id = i - 1; /*change id*/
                }
                log_info("replace first\n");
                fill_index = SUPPORT_MAX_GATT_CLIENT - 1;
            }

            /*连接顺序不同，handle是不一样,防止重复相同*/
            for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                if (info->conn_handle == record_bond_info[i].conn_handle) {
                    record_bond_info[i].conn_handle = 0;
                    log_info("clear repeat handle\n");
                }
            }

            info->match_dev_id = fill_index;/*change id*/
            memcpy(&record_bond_info[fill_index], info, uint_len);

        }

        syscfg_write(CFG_BLE_BONDING_REMOTE_INFO2, (u8 *)&record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
    }

    put_buf((u8 *)&record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
    multi_client_reflash_bond_search_config();/*配对表发生变化，更新scan设备匹配*/

#endif

    return ret;
}
//清配对信息
int multi_client_clear_pair(void)
{
#if CLIENT_PAIR_BOND_ENABLE
    ble_gatt_client_disconnect_all();
    memset(&cur_conn_info, 0, sizeof(cur_conn_info));
    multi_client_pair_vm_do(NULL, 1);
    if (BLE_ST_SCAN == ble_gatt_client_get_work_state()) {
        ble_gatt_client_scan_enable(0);
        ble_gatt_client_scan_enable(1);
    }
#endif
    return 0;
}

//处理gatt 回调的事件，hci & gatt
static int multi_client_event_packet_handler(int event, u8 *packet, u16 size, u8 *ext_param)
{
    /* log_info("event: %02x,size= %d\n",event,size); */
    switch (event) 
    {
        case GATT_COMM_EVENT_GATT_DATA_REPORT:  // service数据接收
        {
            att_data_report_t *report_data = (void *)packet;
            log_info("data_report:hdl=%04x,pk_type=%02x,size=%d\n", report_data->conn_handle, report_data->packet_type, report_data->blob_length);
            put_buf(report_data->blob, report_data->blob_length);   
            Mppt_Data_Decode(report_data->blob,report_data->blob_length); // MPPT数据解析
            switch (report_data->packet_type) 
            {
                case GATT_EVENT_NOTIFICATION://notify
                    break;
                case GATT_EVENT_INDICATION://indicate
                    break;
                case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT://read
                    break;
                case GATT_EVENT_LONG_CHARACTERISTIC_VALUE_QUERY_RESULT://read long
                    break;
                default:
                    break;
            }
        }
        break;

        case GATT_COMM_EVENT_CAN_SEND_NOW: // 可以发数据通知
            break;
 
        case GATT_COMM_EVENT_CONNECTION_COMPLETE: // 连接完成通知
            log_info("connection_handle:%04x\n", little_endian_read_16(packet, 0));
            log_info("con_interval = %d\n", little_endian_read_16(ext_param, 14 + 0));
            log_info("con_latency = %d\n", little_endian_read_16(ext_param, 14 + 2));
            log_info("cnn_timeout = %d\n", little_endian_read_16(ext_param, 14 + 4));
            log_info("peer_address_info:"); put_buf(&ext_param[7], 7); // MAC 地址
           

            memcpy(cur_conn_info.peer_address_info, &ext_param[7], 7); // 当前连接的MAC地址
            cur_conn_info.conn_handle =   little_endian_read_16(packet, 0);
            RoterData.conn_handle = cur_conn_info.conn_handle;
            cur_conn_info.conn_interval = little_endian_read_16(ext_param, 14 + 0);
            cur_conn_info.conn_latency =  little_endian_read_16(ext_param, 14 + 2);
            cur_conn_info.conn_timeout =  little_endian_read_16(ext_param, 14 + 4);
            cur_conn_info.pair_flag = 0;
            cur_conn_info.head_tag = 0;
            break;

        case GATT_COMM_EVENT_DISCONNECT_COMPLETE: //蓝牙断开连接 
            log_info("disconnect_handle:%04x,reason= %02x\n", little_endian_read_16(packet, 0), packet[2]);
            break;
    
        case GATT_COMM_EVENT_ENCRYPTION_CHANGE:   //蓝牙加密完成
            log_info("ENCRYPTION_CHANGE:handle=%04x,state=%d,process =%d", little_endian_read_16(packet, 0), packet[2], packet[3]);

            if (packet[3] == LINK_ENCRYPTION_RECONNECT) 
            {
                log_info("reconnect...\n");
            }
            else
            {
                log_info("first pair...\n");
            }

    #if CLIENT_PAIR_BOND_ENABLE
            cur_conn_info.head_tag = CLIENT_PAIR_BOND_TAG;
            cur_conn_info.pair_flag = 1;
            multi_client_pair_vm_do(&cur_conn_info, 1);
    #endif
            break;

        case GATT_COMM_EVENT_CONNECTION_UPDATE_COMPLETE: // 链路参数更新完成
            log_info("conn_param update_complete:%04x\n", little_endian_read_16(packet, 0));
            log_info("update_interval = %d\n", little_endian_read_16(ext_param, 6 + 0));
            log_info("update_latency = %d\n",  little_endian_read_16(ext_param, 6 + 2));
            log_info("update_timeout = %d\n",  little_endian_read_16(ext_param, 6 + 4));

            if (cur_conn_info.conn_handle == little_endian_read_16(packet, 0)) 
            {
                cur_conn_info.conn_interval = little_endian_read_16(ext_param, 6 + 0);
                cur_conn_info.conn_latency =  little_endian_read_16(ext_param, 6 + 2);
                cur_conn_info.conn_timeout =  little_endian_read_16(ext_param, 6 + 4);
            }

            #if CLIENT_PAIR_BOND_ENABLE
                if (cur_conn_info.conn_handle == little_endian_read_16(packet, 0)) 
                {
                    if (cur_conn_info.pair_flag) 
                    {
                        log_info("update_cur_conn\n");
                        multi_client_pair_vm_do(&cur_conn_info, 1);
                    }
                } 
                else 
                {
                    struct ctl_pair_info_t tmp_conn_info;
                    for (int i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) 
                    {
                        if (record_bond_info[i].pair_flag && record_bond_info[i].conn_handle == little_endian_read_16(packet, 0))
                        {
                            log_info("update_record_conn\n");
                            memcpy(&tmp_conn_info, &record_bond_info[i], sizeof(struct ctl_pair_info_t));
                            tmp_conn_info.conn_interval = little_endian_read_16(ext_param, 6 + 0);
                            tmp_conn_info.conn_latency =  little_endian_read_16(ext_param, 6 + 2);
                            tmp_conn_info.conn_timeout =  little_endian_read_16(ext_param, 6 + 4);
                            multi_client_pair_vm_do(&tmp_conn_info, 1);
                            break;
                        }
                    }
                }
            #endif

            break;

        case GATT_COMM_EVENT_SCAN_DEV_MATCH:   // 扫描到匹配的设备
        {
            log_info("match_dev:addr_type= %d\n", packet[0]);
            put_buf(&packet[1], 6);
            if (packet[8] == 2)
            {
                log_info("is TEST_BOX\n");
            }
            client_match_cfg_t *match_cfg = ext_param;
            if (match_cfg) 
            {
                log_info("match_mode: %d\n", match_cfg->create_conn_mode);
                if (match_cfg->compare_data_len) 
                {
                    put_buf(match_cfg->compare_data, match_cfg->compare_data_len);
                }
            }

            //update info
            cur_conn_info.conn_handle = 0;
            cur_conn_info.pair_flag = 0;
            cur_conn_info.match_dev_id = packet[9];

            #if CLIENT_PAIR_BOND_ENABLE
                    if (packet[9] < SUPPORT_MAX_GATT_CLIENT) {
                        /*记录表回连，使用记录的连接参数建立*/
                        r_printf("match bond,reconnect\n");
                        multi_scan_conn_config_set(&record_bond_info[packet[9]]);
                        if (!multi_pair_reconnect_search_profile) {
                            multil_client_bond_config.search_uuid_count = 0;//set no search
                        }
                    } else {
                        /*记录表回连，使用记录的连接参数建立*/
                        r_printf("match config\n");
                        multi_scan_conn_config_set(NULL);
                    }
            #endif

        }
        break;
        case GATT_COMM_EVENT_GATT_SEARCH_MATCH_UUID:  // 找到对应的UUID
        {
            opt_handle_t *opt_hdl = packet;
            
            log_info("match:server_uuid= %04x,charactc_uuid= %04x,value_handle= %04x\n", \
                    opt_hdl->search_uuid->services_uuid16, opt_hdl->search_uuid->characteristic_uuid16, opt_hdl->value_handle);

            #if MULTI_TEST_WRITE_SEND_DATA
                    //for test
                    if (opt_hdl->search_uuid->characteristic_uuid16 == 0xae01)  // 这里就配置写handle就好了
                    {
                        multi_ble_client_write_handle = opt_hdl->value_handle;
                    }
            #endif
        }
        break;


        case GATT_COMM_EVENT_MTU_EXCHANGE_COMPLETE: // MTU交换完毕
            log_info("con_handle= %02x, ATT MTU = %u\n", little_endian_read_16(packet, 0), little_endian_read_16(packet, 2));
            break;

        case GATT_COMM_EVENT_GATT_SEARCH_PROFILE_COMPLETE:

        #if CLIENT_PAIR_BOND_ENABLE
                if (!multi_pair_reconnect_search_profile) {
                    multil_client_bond_config.search_uuid_count = (sizeof(jl_multi_search_uuid_table) / sizeof(target_uuid_t));//recover
                }
        #endif
            break;

        case GATT_COMM_EVENT_CLIENT_STATE:      // client 状态变化
            log_info("client_state: handle=%02x,%02x\n", little_endian_read_16(packet, 1), packet[0]);
            break;

        case GATT_COMM_EVENT_SM_PASSKEY_INPUT: // 密钥输入
        {
            u32 *key = little_endian_read_32(packet, 2);

            u32 key_t ;
            uint8_t data[8]  =
            {
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
            };

            for(int i=0;i<6;i++)data[i] = RoterData.Ble_Connect_Mac[i];

            btea(data,2,bl_ckey);

            key_t = little_endian_read_32(data,0);


            *key = key_t%999999;
          //  *key = 777777;
            r_printf("input_key:%6u\n", *key);
        }

        default:
            break;
    }
    return 0;

}

static void multi_scan_conn_config_set(struct ctl_pair_info_t *pair_info)//scan参数设置
{
    multi_client_scan_cfg.scan_auto_do = 1;
    multi_client_scan_cfg.creat_auto_do = 1;
    multi_client_scan_cfg.scan_type = SET_SCAN_TYPE;
    multi_client_scan_cfg.scan_filter = 1;
    multi_client_scan_cfg.scan_interval = SET_SCAN_INTERVAL;
    multi_client_scan_cfg.scan_window = SET_SCAN_WINDOW;

    if (pair_info)
    {
        log_info("pair_to_creat:%d,%d,%d\n", pair_info->conn_interval, pair_info->conn_latency, pair_info->conn_timeout);
        multi_client_scan_cfg.creat_conn_interval = pair_info->conn_interval;
        multi_client_scan_cfg.creat_conn_latency = pair_info->conn_latency;
        multi_client_scan_cfg.creat_conn_super_timeout = pair_info->conn_timeout;
    } 
    else 
    {
        multi_client_scan_cfg.creat_conn_interval = SET_CONN_INTERVAL;
        multi_client_scan_cfg.creat_conn_latency = SET_CONN_LATENCY;
        multi_client_scan_cfg.creat_conn_super_timeout = SET_CONN_TIMEOUT;
    }

    multi_client_scan_cfg.conn_update_accept = 1;
    multi_client_scan_cfg.creat_state_timeout_ms = SET_CREAT_CONN_TIMEOUT;

    ble_gatt_client_set_scan_config(&multi_client_scan_cfg);
}

//multi client 初始化
void multi_client_init(void)
{
    log_info("%s", __FUNCTION__);
    int i;

    #if CLIENT_PAIR_BOND_ENABLE
        if (!multi_bond_device_table)
         {
            int table_size = sizeof(multi_match_device_table) + sizeof(client_match_cfg_t) * SUPPORT_MAX_GATT_CLIENT;
            bond_device_table_cnt = multil_client_search_config.match_devices_count + SUPPORT_MAX_GATT_CLIENT;
            multi_bond_device_table = malloc(table_size);
            ASSERT(multi_bond_device_table != NULL, "%s,malloc fail!", __func__);
            memset(multi_bond_device_table, 0, table_size);
        }

        if (0 == multi_client_pair_vm_do(NULL, 0))
        {
            log_info("client already bond dev");
        }

        if (multi_sm_master_pair_redo) 
        {
            sm_set_master_pair_redo(multi_sm_master_pair_redo);
        }

    #else
        ble_gatt_client_set_search_config(&multil_client_search_config);
    #endif

    multi_scan_conn_config_set(NULL);

    #if MULTI_TEST_WRITE_SEND_DATA  //测试写数据
    // sys_timer_add(0, Mppt_Set_Para_Send, 1000);
    // sys_timer_add(0, Get_Mppt_Report, 1000);
    #endif
}

//multi exit
void multi_client_exit(void)
{
    if (!multi_bond_device_table) 
    {
        free(multi_bond_device_table);
        multi_bond_device_table = NULL;
    }
}

#endif


