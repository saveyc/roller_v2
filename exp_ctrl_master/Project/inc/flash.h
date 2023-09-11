#ifndef _FLASH_H
#define _FLASH_H
#include "main.h"

#define  UserParaAddressone           0x08032000  //每个主存储块2k 第101个存储块 
#define  UserParaAddresstwo           0x08032800  //每个主存储块2k 第101个存储块 
#define  MAX_MOUDLE_NUM               2


void flash_read_zone_conf(void);
void flash_upload(void);


#endif