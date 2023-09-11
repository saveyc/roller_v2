#ifndef __CAN_BUS_H
#define __CAN_BUS_H


#define CAN_PACK_DATA_LEN     8
#define CAN_TX_BUFF_SIZE      300
#define CAN_RX_BUFF_SIZE      300

enum
{
    CAN_FUNC_ID_PARA_DATA     = 0x1,     //用户参数命令(读写)
    CAN_FUNC_ID_MODULE_STATUS = 0x2,     //模块实时状态信息(周期指令)
    CAN_FUNC_ID_START_STOP_CMD = 0x3,    //启停命令
    CAN_FUNC_ID_BOOT_MODE     = 0xF
};

enum
{
    CAN_SEG_POLO_NONE   = 0,
    CAN_SEG_POLO_FIRST  = 1,
    CAN_SEG_POLO_MIDDLE = 2,
    CAN_SEG_POLO_FINAL  = 3
};

enum {
    SET_PARA = 1,
    RECV_SET_PARA = 2,
    READ_PARA = 3,
    RECV_READ_PARA = 4, 
};

#pragma pack (1)

/*
|    28~21    |    20~19      |    18~12     |     11~8     |    7~0      |    ExtID
|  dst_id(8)  |  seg_polo(2)  |  seg_num(7)  |  func_id(4)  |  src_id(8)  |
*/

typedef struct
{
    u8 seg_polo;
    u8 seg_num;
    u8 func_id;
    u8 src_id;
    u8 dst_id;
}sCanFrameExtID;

typedef struct
{
    sCanFrameExtID extId;
    u8 data_len;
    u8 data[8];
}sCanFrameExt;

#pragma pack ()

typedef struct {
    sCanFrameExt *queue; /* 指向存储队列的数组空间 */
    u16 front, rear, len; /* 队首指针（下标），队尾指针（下标），队列长度变量 */
    u16 maxSize; /* queue数组长度 */
} sCAN_SEND_QUEUE;



extern u8   canbus_readpara_buff[];
extern u16  canbus_readpara_len;

void InitCanSendQueue(void);
void can_bus_frame_receive(CanRxMsg rxMsg);
void can_send_frame_process();


void canbus_send_user_moudlepara(u8* buf, u16 len);
void canbus_read_user_moudlepara(u8 dst);
void canbus_recv_read_moudlepara(u8* buff, u16 len);
void vcanbus_send_start_cmd(sMoudle_cmd moudle, u16 frame, u8 dst);

#endif