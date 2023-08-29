#include "stm32f10x.h"
#include "stm32f107.h"
#include "main.h"


//CAN1  和下游通讯

#define  canframebuffsize_one   50
sCanFrameExt    canframequeuebuff_one[canframebuffsize_one];
sCanFrameQueue   canframequeue_one;

void vcan_sendmsg_one(u8* buff, u16 send_total_len, u8 type, u8 dst);


u8  canone_send_buff[CAN_TX_BUFF_SIZE];
u16 canone_send_len = 0;
u8  canone_recv_buff[CAN_RX_BUFF_SIZE];
u16 canone_recv_len = 0;

sCanFrameCtrl   canone_ctrl;


void vcanbus_initcansendqueue_one(void)
{
    sCanFrameQueue* q = NULL;
    q = &canframequeue_one;

    q->queue = canframequeuebuff_one;
    q->front = q->rear = 0;
    q->maxsize = canframebuffsize_one;
}

void vcanbus_addto_cansendqueue_one(sCanFrameExt x)
{
    sCanFrameQueue* q = NULL;
    q = &canframequeue_one;

    if ((q->rear + 1) % q->maxsize == q->front)
    {
        return;
    }

    q->rear = (q->rear + 1) % q->maxsize;

    q->queue[q->rear] = x;
}


u8 u8canbus_send_oneframe_one(sCanFrameExt sTxMsg)
{
    CanTxMsg TxMessage;

    TxMessage.ExtId = (sTxMsg.extId.mac_id & 0xFF) | ((sTxMsg.extId.func_id & 0xF) << 8) | ((sTxMsg.extId.seg_num & 0x7F) << 12) | ((sTxMsg.extId.seg_polo & 0x3) << 19) | ((sTxMsg.extId.dst_id & 0xFF) << 21);
    TxMessage.IDE = CAN_ID_EXT;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = sTxMsg.data_len;
    memcpy(TxMessage.Data, sTxMsg.data, TxMessage.DLC);

    return CAN_Transmit(CAN1, &TxMessage);
}


void vcanbus_framereceive_one(CanRxMsg rxMsg)
{
    u16 i =0;
    sCanFrameExt   frame;

    frame.extId.seg_polo = (rxMsg.ExtId >> 19) & 0x3;
    frame.extId.seg_num = (rxMsg.ExtId >> 12) & 0x7F;
    frame.extId.func_id = (rxMsg.ExtId >> 8) & 0xF;
    frame.extId.mac_id = rxMsg.ExtId & 0xFF;
    frame.extId.dst_id = rxMsg.ExtId >> 21 & 0xFF;



    frame.data_len = rxMsg.DLC;

    for(i=0;i<frame.data_len;i++){
        frame.data[i] = rxMsg.Data[i];
    }

    vcanbus_addto_cansendqueue_two(frame);

    if ((frame.extId.func_id == CAN_FUNC_ID_BOOT_MODE) && (frame.extId.dst_id == localStation))
    {
        //        RTC_WriteBackupRegister(RTC_BKP_DR0, 0x55);
        //        NVIC_SystemReset();

    }

    //if (frame.extId.dst_id != localStation) {
    //    if (frame.extId.mac_id > localStation) {
    //        vcanbus_addto_cansendqueue_two(frame);
    //    }
    //    return;
    //}

    if (frame.extId.dst_id != localStation) {
        return;
    }

    //接收数据
    if (frame.extId.seg_polo == CAN_SEG_POLO_NONE)
    {
        memcpy(canone_recv_buff, rxMsg.Data, rxMsg.DLC);
        canone_recv_len = rxMsg.DLC;
        canbus_recv_can_msg(canone_recv_buff, canone_recv_len, frame.extId.mac_id, frame.extId.func_id);
    }
    else if (frame.extId.seg_polo == CAN_SEG_POLO_FIRST)
    {
        memcpy(canone_recv_buff, rxMsg.Data, rxMsg.DLC);
        canone_ctrl.g_SegPolo = CAN_SEG_POLO_FIRST;
        canone_ctrl.g_SegNum = frame.extId.seg_num;
        canone_ctrl.g_SegBytes = rxMsg.DLC;
    }
    else if (frame.extId.seg_polo == CAN_SEG_POLO_MIDDLE)
    {
        if ((canone_ctrl.g_SegPolo == CAN_SEG_POLO_FIRST)
            && (frame.extId.seg_num == (canone_ctrl.g_SegNum + 1))
            && ((canone_ctrl.g_SegBytes + rxMsg.DLC) <= CAN_RX_BUFF_SIZE))
        {
            memcpy(canone_recv_buff + canone_ctrl.g_SegBytes, rxMsg.Data, rxMsg.DLC);
            canone_ctrl.g_SegNum++;
            canone_ctrl.g_SegBytes += rxMsg.DLC;
        }
    }
    else if (frame.extId.seg_polo == CAN_SEG_POLO_FINAL)
    {
        if ((canone_ctrl.g_SegPolo == CAN_SEG_POLO_FIRST)
            && (frame.extId.seg_num == (canone_ctrl.g_SegNum + 1))
            && ((canone_ctrl.g_SegBytes + rxMsg.DLC) <= CAN_RX_BUFF_SIZE))
        {
            memcpy(canone_recv_buff + canone_ctrl.g_SegBytes, rxMsg.Data, rxMsg.DLC);
            canone_recv_len = canone_ctrl.g_SegBytes + rxMsg.DLC;
            canbus_recv_can_msg(canone_recv_buff, canone_recv_len, frame.extId.mac_id, frame.extId.func_id);
            canone_ctrl.g_SegPolo = CAN_SEG_POLO_NONE;
            canone_ctrl.g_SegNum = 0;
            canone_ctrl.g_SegBytes = 0;
        }
    }
}


void vcan_sendmsg_one(u8* buff, u16 send_total_len, u8 type, u8 dst)
{
    sCanFrameExt canTxMsg;
    u8 send_len = 0;

    canTxMsg.extId.func_id = type;
    canTxMsg.extId.mac_id = localStation;
    canTxMsg.extId.dst_id = dst;

    if (send_total_len <= CAN_PACK_DATA_LEN)   //不需分段传输
    {
        canTxMsg.extId.seg_polo = CAN_SEG_POLO_NONE;
        canTxMsg.extId.seg_num = 0;
        canTxMsg.data_len = send_total_len;
        memcpy(canTxMsg.data, buff, canTxMsg.data_len);
        vcanbus_addto_cansendqueue_one(canTxMsg);
    }  
    else    //需要分段传输
    {                        
        canTxMsg.extId.seg_polo = CAN_SEG_POLO_FIRST;
        canTxMsg.extId.seg_num = 0;
        canTxMsg.data_len = CAN_PACK_DATA_LEN;
        memcpy(canTxMsg.data, buff, canTxMsg.data_len);
        vcanbus_addto_cansendqueue_one(canTxMsg);
        send_len += CAN_PACK_DATA_LEN;
        while (1) {
            if (send_len + CAN_PACK_DATA_LEN < send_total_len)
            {
                canTxMsg.extId.seg_polo = CAN_SEG_POLO_MIDDLE;
                canTxMsg.extId.seg_num++;
                canTxMsg.data_len = CAN_PACK_DATA_LEN;
                memcpy(canTxMsg.data, buff + send_len, canTxMsg.data_len);
                vcanbus_addto_cansendqueue_one(canTxMsg);
                send_len += CAN_PACK_DATA_LEN;
            }
            else
            {
                canTxMsg.extId.seg_polo = CAN_SEG_POLO_FINAL;
                canTxMsg.extId.seg_num++;
                canTxMsg.data_len = send_total_len - send_len;
                memcpy(canTxMsg.data, buff + send_len, canTxMsg.data_len);
                vcanbus_addto_cansendqueue_one(canTxMsg);
                break;
            }
        }
    }
}

void vcan_sendframe_process_one(void)
{
    sCanFrameQueue* q = &canframequeue_one;
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
    tx_mailbox = u8canbus_send_oneframe_one(*canTxMsg);
    if (tx_mailbox != CAN_NO_MB)
    {
        q->front = front;
    }
}
