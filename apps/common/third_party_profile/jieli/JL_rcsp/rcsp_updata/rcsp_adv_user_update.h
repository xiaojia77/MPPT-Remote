#ifndef _RCSP_ADV_USER_UPDATE_H_
#define _RCSP_ADV_USER_UPDATE_H_

//#include "rcsp_protocol.h"
//#include "rcsp_packet.h"
#include "typedef.h"

#define JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET    0xe1    //获取设备更新文件的OFFSET
#define JL_OPCODE_INQUIRE_DEVICE_IF_CAN_UPDATE          0xe2    //获取是否可以更新
#define JL_OPCODE_ENTER_UPDATE_MODE                     0xe3    //进入更新模式
#define JL_OPCODE_EXIT_UPDATE_MODE                      0xe4    //退出更新模式
#define JL_OPCODE_SEND_FW_UPDATE_BLOCK                  0xe5    //发送固件更新锁
#define JL_OPCODE_GET_DEVICE_REFRESH_FW_STATUS          0xe6    //获取更新后的固件状态
#define JL_OPCODE_SET_DEVICE_REBOOT                     0xe7    //设置设备重启
#define JL_OPCODE_NOTIFY_UPDATE_CONENT_SIZE				0xe8    //通知更新内容


void JL_rcsp_update_cmd_resp(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);   //命令答复
void JL_rcsp_update_cmd_receive_resp(void *priv, u8 OpCode, u8 status, u8 *data, u16 len);  //命令接收答复
void JL_rcsp_msg_deal(void *hdl, u8 event, u8 *msg);    //消息处理
u8 get_jl_update_flag(void);            //获取JL更新状态flag
void set_jl_update_flag(u8 flag);       //设置JL更新状态
u8 get_curr_device_type(void);          //获取当前设备类型
void update_slave_adv_reopen(void);     //

#endif

