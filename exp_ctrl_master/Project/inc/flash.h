#ifndef _FLASH_H
#define _FLASH_H
#include "main.h"

#define  UserParaAddressone           0x08032000  //ÿ�����洢��2k ��101���洢�� 
#define  UserParaAddresstwo           0x08032800  //ÿ�����洢��2k ��101���洢�� 
#define  MAX_MOUDLE_NUM               2


void flash_read_zone_conf(void);
void flash_upload(void);


#endif