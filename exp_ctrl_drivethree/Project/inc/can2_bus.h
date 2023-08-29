#ifndef __CAN_BUS_H
#define __CAN_BUS_H

#include "main.h"

#pragma pack (1)

#define CAN_PACK_DATA_LEN     8
#define CAN_TX_BUFF_SIZE      300
#define CAN_RX_BUFF_SIZE      300

enum
{
    CAN_FUNC_ID_PARA_DATA = 0x1,         //用户参数命令(读写)
    CAN_FUNC_ID_MODULE_STATUS = 0x2,     //模块实时状态信息(周期指令)
    CAN_FUNC_ID_START_STOP_CMD = 0x3,    //启停命令
    CAN_FUNC_ID_BOOT_MODE = 0xF,          //boot
};

enum {
    SET_PARA = 1,
    RECV_SET_PARA = 2,
    READ_PARA = 3,
    RECV_READ_PARA = 4,
};


enum
{
    CAN_SEG_POLO_NONE = 0,
    CAN_SEG_POLO_FIRST = 1,
    CAN_SEG_POLO_MIDDLE = 2,
    CAN_SEG_POLO_FINAL = 3
};


/*
|    28~21    |    20~19      |    18~12     |     11~8     |    7~0      |    ExtID
|  dst_id(8)  |  seg_polo(2)  |  seg_num(7)  |  func_id(4)  |  src_id(8)  |
*/

typedef struct
{
    u8 seg_polo;
    u8 seg_num;
    u8 func_id;
    u8 mac_id;
    u8 dst_id;
} sCanFrameExtID;



typedef struct
{
    sCanFrameExtID extId;
    u8 data_len;
    u8 data[8];
} sCanFrameExt;

typedef struct
{
    u8  g_SegPolo;
    u8  g_SegNum;
    u16 g_SegBytes;
}sCanFrameCtrl;

#pragma pack ()

typedef struct
{
    sCanFrameExt* queue;
    u16 front, rear, len;
    u16 maxsize;
}sCanFrameQueue;



#define CAN_PACK_DATA_LEN     8
#define CAN_MAX_FRAME_LEN     1024

#define CAN_SEND_DATA_LEN     100

#define CAN_RECV_PREFRAME     8

#define ALL_SLAVER            0xF



#define CAN_RX_BUFF_SIZE_TWO      1024
#define CAN_RX_CONFIG_SIZE_TWO    200


extern sCanFrameQueue   canframequeue;


//can2
void vcanbus_initcansendqueue_two(void);
void vcanbus_addto_cansendqueue_two(sCanFrameExt x);
u8 u8canbus_send_oneframe_two(sCanFrameExt sTxMsg);
void vcanbus_frame_receive_two(CanRxMsg rxMsg);
void vcan_sendmsg_two(u8* buff, u16 send_total_len, u8 type, u8 dst);
void vcan_sendframe_process_two(void);

//universal
void para_data_recv_process(u8* pbuf, u16 recv_len);
void canbus_recv_start_cmd(u8* buf);
void canbus_recv_can_msg(u8* buf, u16 len, u8 bdindex, u8 type);
void can_recv_set_para(u8* buff, u16 len);
void can_reply_set_para(void);
void can_reply_read_para(void);

#endif