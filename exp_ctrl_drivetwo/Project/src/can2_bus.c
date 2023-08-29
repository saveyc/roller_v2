#include "stm32f10x.h"
#include "stm32f107.h"
#include "main.h"

// can2 和上下游通讯
#define  canframebuffsize_two   50
sCanFrameExt    canframequeuebuff_two[canframebuffsize_two];
sCanFrameQueue   canframequeue_two;

u8  cantwo_send_buff[CAN_TX_BUFF_SIZE];
u16 cantwo_send_len = 0;
u8  cantwo_recv_buff[CAN_RX_BUFF_SIZE];
u16 cantwo_recv_len = 0;

sCanFrameCtrl   cantwo_ctrl;


void vcan_sendmsg_two(u8* buff, u16 send_total_len, u8 type, u8 dst);


void vcanbus_initcansendqueue_two(void)
{
    sCanFrameQueue* q = NULL;
    q = &canframequeue_two;

    q->queue = canframequeuebuff_two;
    q->front = q->rear = 0;
    q->maxsize = canframebuffsize_two;
}

void vcanbus_addto_cansendqueue_two(sCanFrameExt x)
{
    sCanFrameQueue* q = NULL;
    q = &canframequeue_two;

    if ((q->rear + 1) % q->maxsize == q->front)
    {
        return;
    }

    q->rear = (q->rear + 1) % q->maxsize;

    q->queue[q->rear] = x;
}


u8 u8canbus_send_oneframe_two(sCanFrameExt sTxMsg)
{
    CanTxMsg TxMessage;

    TxMessage.ExtId = (sTxMsg.extId.mac_id & 0xFF) | ((sTxMsg.extId.func_id & 0xF) << 8) | ((sTxMsg.extId.seg_num & 0x7F) << 12) | ((sTxMsg.extId.seg_polo & 0x3) << 19) | ((sTxMsg.extId.dst_id & 0xFF) << 21);
    TxMessage.IDE = CAN_ID_EXT;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = sTxMsg.data_len;
    memcpy(TxMessage.Data, sTxMsg.data, TxMessage.DLC);

    return CAN_Transmit(CAN2, &TxMessage);
}





//can2接收数据
void vcanbus_frame_receive_two(CanRxMsg rxMsg)
{
    sCanFrameExt   frame;

    frame.extId.seg_polo = (rxMsg.ExtId >> 19) & 0x3;
    frame.extId.seg_num = (rxMsg.ExtId >> 12) & 0x7F;
    frame.extId.func_id = (rxMsg.ExtId >> 8) & 0xF;
    frame.extId.mac_id = rxMsg.ExtId & 0xFF;
    frame.extId.dst_id = rxMsg.ExtId >> 21 & 0xFF;

    frame.data_len = rxMsg.DLC;

    memcpy(frame.data, rxMsg.Data, rxMsg.DLC);

    vcanbus_addto_cansendqueue_one(frame);


    if ((frame.extId.func_id == CAN_FUNC_ID_BOOT_MODE) && (frame.extId.dst_id == localStation))
    {
//        RTC_WriteBackupRegister(RTC_BKP_DR0, 0x55);
        NVIC_SystemReset();

    }

    if (frame.extId.dst_id != localStation) {
        return;
    }

    //接收数据
    if (frame.extId.seg_polo == CAN_SEG_POLO_NONE)
    {
        memcpy(cantwo_recv_buff, rxMsg.Data, rxMsg.DLC);
        cantwo_recv_len = rxMsg.DLC;
        canbus_recv_can_msg(cantwo_recv_buff, cantwo_recv_len, frame.extId.mac_id, frame.extId.func_id);
    }
    else if (frame.extId.seg_polo == CAN_SEG_POLO_FIRST)
    {
        memcpy(cantwo_recv_buff, rxMsg.Data, rxMsg.DLC);
        cantwo_ctrl.g_SegPolo = CAN_SEG_POLO_FIRST;
        cantwo_ctrl.g_SegNum = frame.extId.seg_num;
        cantwo_ctrl.g_SegBytes = rxMsg.DLC;
    }
    else if (frame.extId.seg_polo == CAN_SEG_POLO_MIDDLE)
    {
        if ((cantwo_ctrl.g_SegPolo == CAN_SEG_POLO_FIRST)
            && (frame.extId.seg_num == (cantwo_ctrl.g_SegNum + 1))
            && ((cantwo_ctrl.g_SegBytes + rxMsg.DLC) <= CAN_RX_BUFF_SIZE))
        {
            memcpy(cantwo_recv_buff + cantwo_ctrl.g_SegBytes, rxMsg.Data, rxMsg.DLC);
            cantwo_ctrl.g_SegNum++;
            cantwo_ctrl.g_SegBytes += rxMsg.DLC;
        }
    }
    else if (frame.extId.seg_polo == CAN_SEG_POLO_FINAL)
    {
        if ((cantwo_ctrl.g_SegPolo == CAN_SEG_POLO_FIRST)
            && (frame.extId.seg_num == (cantwo_ctrl.g_SegNum + 1))
            && ((cantwo_ctrl.g_SegBytes + rxMsg.DLC) <= CAN_RX_BUFF_SIZE))
        {
            memcpy(cantwo_recv_buff + cantwo_ctrl.g_SegBytes, rxMsg.Data, rxMsg.DLC);
            cantwo_recv_len = cantwo_ctrl.g_SegBytes + rxMsg.DLC;
            canbus_recv_can_msg(cantwo_recv_buff, cantwo_recv_len, frame.extId.mac_id, frame.extId.func_id);
            cantwo_ctrl.g_SegPolo = CAN_SEG_POLO_NONE;
            cantwo_ctrl.g_SegNum = 0;
            cantwo_ctrl.g_SegBytes = 0;
        }
    }

}


//can2 将要发送的数据加入队列
void vcan_sendmsg_two(u8* buff, u16 send_total_len, u8 type, u8 dst)
{
    sCanFrameExt canTxMsg;
    u16 send_len = 0;

    canTxMsg.extId.func_id = type;
    canTxMsg.extId.mac_id = localStation;
    canTxMsg.extId.dst_id = dst;

    if (send_total_len <= CAN_PACK_DATA_LEN)//不分段发送
    {
        canTxMsg.extId.seg_polo = CAN_SEG_POLO_NONE;
        canTxMsg.extId.seg_num = 0;
        canTxMsg.data_len = send_total_len;
        memcpy(canTxMsg.data, buff, canTxMsg.data_len);
        vcanbus_addto_cansendqueue_two(canTxMsg);
    }
    else//分段发送    
    {
        canTxMsg.extId.seg_polo = CAN_SEG_POLO_FIRST;
        canTxMsg.extId.seg_num = 0;
        canTxMsg.data_len = CAN_PACK_DATA_LEN;
        memcpy(canTxMsg.data, buff, canTxMsg.data_len);
        vcanbus_addto_cansendqueue_two(canTxMsg);
        send_len += CAN_PACK_DATA_LEN;
        while (1) {
            if (send_len + CAN_PACK_DATA_LEN < send_total_len)
            {
                canTxMsg.extId.seg_polo = CAN_SEG_POLO_MIDDLE;
                canTxMsg.extId.seg_num++;
                canTxMsg.data_len = CAN_PACK_DATA_LEN;
                memcpy(canTxMsg.data, buff + send_len, canTxMsg.data_len);
                vcanbus_addto_cansendqueue_two(canTxMsg);
                send_len += CAN_PACK_DATA_LEN;
            }
            else
            {
                canTxMsg.extId.seg_polo = CAN_SEG_POLO_FINAL;
                canTxMsg.extId.seg_num++;
                canTxMsg.data_len = send_total_len - send_len;
                memcpy(canTxMsg.data, buff + send_len, canTxMsg.data_len);
                vcanbus_addto_cansendqueue_two(canTxMsg);
                break;
            }
        }
    }
}

// 发送队列数据
void vcan_sendframe_process_two(void)
{
    sCanFrameQueue* q = &canframequeue_two;
    sCanFrameExt* canTxMsg = NULL;
    u16 front = 0;
    u8  tx_mailbox = 0;

    if (q->front == q->rear)
    {
        return;
    }

    front = q->front;
    front = (front + 1) % q->maxsize;
    canTxMsg = (sCanFrameExt*)(&(q->queue[front]));
    tx_mailbox = u8canbus_send_oneframe_two(*canTxMsg);
    if (tx_mailbox != CAN_NO_MB)
    {
        q->front = front;
    }
}

// universal

void para_data_recv_process(u8* pbuf, u16 recv_len)
{
    u16 data_len = 0;
    u8  cmd = 0;

    data_len = pbuf[0] | (pbuf[1] >> 8);
    cmd = pbuf[2];

    if (data_len != recv_len) {
        return;
    }

    switch (cmd)
    {
    case SET_PARA://接收写的参数
        can_recv_set_para(&(pbuf[3]), recv_len - 3);
        can_reply_set_para();
        break;
    case RECV_READ_PARA://接收读参数命令并且回复
        //从机收到
        can_reply_read_para();
        break;
    default:
        break;
    }
}


void canbus_recv_start_cmd(u8* buf)
{
    u8 cmd = 0;
    u8 type = 0;
    u8 index = 0;
    u16 frame = 0;
    u8 dir = 0;

    index = buf[0];
    frame = (buf[1]) | (buf[2] << 8);
    cmd = buf[3];
    dir = buf[4];
    type = buf[5];

    if (buf[0] == ZONE_TYPE_ONE) {
        if (frame != cansend_framecnt_one) {
            cansend_framecnt_one = frame;
            moto_set_run_cmd(MOT1, cmd, type, dir);
        }
    }
    else if (buf[0] == ZONE_TYPE_TWO) {
        if (frame != cansend_framecnt_two) {
            cansend_framecnt_two = frame;
            moto_set_run_cmd(MOT2, cmd, type, dir);
        }
    }

}


void canbus_recv_can_msg(u8* buf, u16 len, u8 bdindex, u8 type)
{
    if (bdindex != 1) {
        return;
    }
    switch (type) {
    case CAN_FUNC_ID_PARA_DATA:
        para_data_recv_process(buf, len);
        break;
    case CAN_FUNC_ID_START_STOP_CMD:
        canbus_recv_start_cmd(buf);
        break;
    case CAN_FUNC_ID_MODULE_STATUS:
        upload_state_cnt = localStation + 1;
        break;
    default:
        break;
    }
}

void can_recv_set_para(u8* buff, u16 len)
{
    u16 i = 0;
    u16 ctrl_index = buff[0] | (buff[1] << 8);
    u16 num = buff[2] | (buff[3] << 8);
    u8  flag = 0;

    if (num > MAX_MOUDLE_NUM) {
        return;
    }

    mototemp.moudlenum = num;


    for (i = 0; i < mototemp.moudlenum; i++) {
        mototemp.moudlepara[i].moudleIndex = buff[10 * i + 4] | (buff[10 * i + 1 + 4] << 8);
        mototemp.moudlepara[i].funcConfig = buff[10 * i + 2 + 4] | (buff[10 * i + 3 + 4] << 8);
        mototemp.moudlepara[i].spd = buff[10 * i + 4 + 4] | (buff[10 * i + 5 + 4] << 8);
        mototemp.moudlepara[i].dir = buff[10 * i + 6 + 4] | (buff[10 * i + 7 + 4] << 8);
        mototemp.moudlepara[i].ramp = buff[10 * i + 8 + 4] | (buff[10 * i + 9 + 4] << 8);
    }
    for (i = 0; i < mototemp.moudlenum; i++) {
        if ((mototemp.moudlepara[i].funcConfig != 0) || (mototemp.moudlepara[i].spd != 0) ||
            (mototemp.moudlepara[i].dir != 0) || (mototemp.moudlepara[i].ramp != 0)) {
            flag = 1;
        }
    }

    if (flag == 1) {
        flash_upload_flag = VALUE;
    }

}

void can_reply_set_para(void)
{
    u8 buff[10] = { 0 };
    u16 sendlen = 0;

    sendlen = 3;
    buff[0] = sendlen & 0xFF;
    buff[1] = (sendlen >> 8) & 0xFF;
    buff[2] = RECV_SET_PARA;

    vcan_sendmsg_one(buff, sendlen, CAN_FUNC_ID_PARA_DATA, 1);
    vcan_sendmsg_two(buff, sendlen, CAN_FUNC_ID_PARA_DATA, 1);
}

void can_reply_read_para(void)
{
    u8 buff[50] = { 0 };
    u8 sendlen = 0;
    u16 i = 0;
    if (motolocal.moudlenum != 0xFFFF) {
        buff[3] = motolocal.version[0];
        buff[4] = motolocal.version[1];
        buff[5] = motolocal.version[2];
        buff[6] = motolocal.version[3];
        buff[7] = localStation;
        buff[8] = 0;
        buff[9] = motolocal.moudlenum & 0xFF;
        buff[10] = (motolocal.moudlenum >> 8) & 0xFF;
        for (i = 0; motolocal.moudlenum; i++) {
            buff[10 * i + 11] = motolocal.moudlepara[i].moudleIndex & 0xFF;
            buff[10 * i + 1 + 11] = (motolocal.moudlepara[i].moudleIndex >> 8) & 0xFF;
            buff[10 * i + 2 + 11] = (motolocal.moudlepara[i].funcConfig) & 0xFF;
            buff[10 * i + 3 + 11] = (motolocal.moudlepara[i].funcConfig >> 8) & 0xFF;
            buff[10 * i + 4 + 11] = (motolocal.moudlepara[i].spd) & 0xFF;
            buff[10 * i + 5 + 11] = (motolocal.moudlepara[i].spd >> 8) & 0xFF;
            buff[10 * i + 6 + 11] = (motolocal.moudlepara[i].dir) & 0xFF;
            buff[10 * i + 7 + 11] = (motolocal.moudlepara[i].dir >> 8) & 0xFF;
            buff[10 * i + 8 + 11] = (motolocal.moudlepara[i].ramp) & 0xFF;
            buff[10 * i + 9 + 11] = (motolocal.moudlepara[i].ramp >> 8) & 0xFF;
        }

        sendlen = 8 + 3 + 10 * motolocal.moudlenum;
        buff[0] = sendlen & 0xFF;
        buff[1] = (sendlen >> 8) & 0xFF;
        buff[2] = RECV_READ_PARA;

        vcan_sendmsg_one(buff, sendlen, CAN_FUNC_ID_PARA_DATA, 1);
        vcan_sendmsg_two(buff, sendlen, CAN_FUNC_ID_PARA_DATA, 1);
    }
    else {
        buff[3] = motolocal.version[0];
        buff[4] = motolocal.version[1];
        buff[5] = motolocal.version[2];
        buff[6] = motolocal.version[3];
        buff[7] = localStation;
        buff[8] = 0;
        buff[9] = 0;
        buff[10] = 0;

        sendlen = 11;
        buff[0] = sendlen & 0xFF;
        buff[1] = (sendlen >> 8) & 0xFF;
        buff[2] = RECV_READ_PARA;
        vcan_sendmsg_one(buff, sendlen, CAN_FUNC_ID_PARA_DATA, 1);
        vcan_sendmsg_two(buff, sendlen, CAN_FUNC_ID_PARA_DATA, 1 );
    }
}

