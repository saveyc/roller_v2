#ifndef _CAN1_BUS_H
#define _CAN1_BUS_H

#include "main.h"

extern  sCanFrameCtrl       canctrl_one;

void vcanbus_initcansendqueue_one(void);
void vcanbus_addto_cansendqueue_one(sCanFrameExt x);
u8 u8canbus_send_oneframe_one(sCanFrameExt sTxMsg);
void vcanbus_framereceive_one(CanRxMsg rxMsg);
void vcan_sendmsg_one(u8* buff, u16 send_total_len, u8 type, u8 dst);
void vcan_sendframe_process_one(void);
#endif 