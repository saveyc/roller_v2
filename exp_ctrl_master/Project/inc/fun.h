#ifndef _FUN_H
#define _FUN_H

/********命令定义**********/
#define		MSG_NULL_TYPE			                     0x0000
/**********主动发送命令*******/
#define		SEND_MSG_BD2PC_BDONLINE_TYPE                  0x1001        //控制器上线信息
#define     SEDN_MSG_BD2PC_MOTOMOUDLE_STATUS_TYPE         0x1008        //模块状态信息

/**********主动发送的命令的回复******/
#define		SEND_MSG_BD2PC_BDONLINE_TYPE_ACK              0x9001

/**********接收命令**********/
#define     RECV_MSG_PC2BD_MOTOPARA_CONFIG_TYPE           0x1002        //电滚筒配置信息
#define     RECV_MSG_PC2BD_MOTOPARA_ACQUIRE_TYPE          0x1003        //读取电滚筒配置信息
#define     RECV_MSG_PC2BD_ZONE_CONFIG_TYPE               0x1004        //区域配置
#define     RECV_MSG_PC2BD_PKG_TRANS_TYPE                 0x1005        //包裹运输指令
#define     RECV_MSG_PC2BD_PKG_ALLOW_FORBID_TYPE          0x1007        //包裹是否允许运输指令
#define     RECV_MSG_PC2BD_MOTO_RUN_STOP_CMD_TYPE         0x1009        //启停电滚筒指令

#define     RECV_MSG_BOOT_CMD_TYPE                        0x1F01        //进入BOOT
/**********回复接收命令******/
#define		REPLY_RECV_MSG_PC2BD_MOTOPARA_CONFIG_TYPE     0x9002
#define     REPLY_RECV_MSG_PC2BD_MOTOPARA_ACQUIRE_TYPE    0x9003
#define     REPLY_RECV_MSG_PC2BD_ZONE_CONFIG_TYPE         0x9004
#define     REPLY_RECV_MSG_PC2BD_PKG_TRANS_TYPE           0x9005    
#define     REPLY_RECV_MSG_PC2BD_PKG_ALLOW_FORBID_TYPE    0x9007   
#define     REPLY_RECV_MSG_PC2BD_MOTO_RUN_STOP_CMD_TYPE   0x9009        

/**********回复接收命令******/

#pragma pack (1) 
typedef struct {
    u8  MSG_TAG[2];
    u32 MSG_ID;
    u16 MSG_LENGTH;
    u8  MSG_CRC;
    u16 MSG_TYPE;
}MSG_HEAD_DATA;

typedef struct {
    u16 sendcnt;                         //发送上线命令计时
    u16 checkcnt;                        //自检计数
    u8  version[4];                      //版本号
    u32 confid;                          //区域配置标识符
}sFun_Online_ctrl;

#pragma pack () 

typedef struct {
        u16 *queue; /* 指向存储队列的数组空间 */
        u16 front, rear, len; /* 队首指针（下标），队尾指针（下标），队列长度变量 */
        u16 maxSize; /* queue数组长度 */
}MSG_SEND_QUEUE;

extern sFun_Online_ctrl    funOnlineCtrl;

extern u8 upload_motostate_buff[];
extern u16 zonestate_index;
extern u16 upload_state_flag;
extern u16 upload_state_cnt;
extern u16 upload_pkg_cnt;


void InitSendMsgQueue(void);
void AddSendMsgToQueue(u16 msg);
u16 GetSendMsgFromQueue(void);
void recv_message_from_server(u8 *point,u16 *len);
void send_message_to_server(void);
void upload_moudle_state(void);

void fun_struct_init(void);

#endif
