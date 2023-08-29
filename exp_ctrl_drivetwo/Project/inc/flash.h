#ifndef _FLASH_H
#define _FLASH_H
#include "main.h"

#define  UserParaAddress           0x08032000  //每个主存储块2k 第101个存储块 
#define  MAX_MOUDLE_NUM            2

enum {
	ZONE_TYPE_ONE = 0x01,
	ZONE_TYPE_TWO = 0x02,
	ZONE_TYPE_RISE = 0x03,
};

enum {
	DIR_NONE = 0,
	FRONT_DIR = 1,
	BEHIND_DIR = 2,
	LEFT_DIR = 3,
	RIGHT_DIR = 4,
};

enum {
	RUN_DIRDEFAULT = 0,
	RUN_AHEAD_TOBACK = 1,
	RUN_BACK_TOAHEAD = 2,
	RUN_LEFT_TORIGHT = 3,
	RUN_RIGHT_TOLEFT = 4,
	RUN_AHEAD_TOLEFT = 5,
	RUN_AHEAD_TORIGHT = 6,
	RUN_BACK_TOLEFT = 7,
	RUN_BACK_TORIGHT = 8,
	RUN_LEFT_TOAHEAD = 9,
	RUN_LEFT_TOBACK = 10,
	RUN_RIGHT_TOAHEAD = 11,
	RUN_RIGHT_TOBACK = 12,
};

enum {
	RUN_DEFAULT = 0,
	RUN_CMD = 1,
	STOP_CMD = 2,
};

enum {
	RUN_TYPEDEFAULT = 0,
	CONTINUE_RUN = 1,
	RUN_TRIGSTOP = 2,
};

#pragma pack (1)

typedef struct {
	u16 moudleIndex;               //模块编号索引
	u16 funcConfig;                //功能配置
	u16 spd;
	u16 dir;
	u16 ramp;
}sFlash_moudle_para;

typedef struct {
	u8  version[4];
	u16  moudlenum;
	sFlash_moudle_para  moudlepara[MAX_MOUDLE_NUM];
}sFlash_moto_msg;

#pragma pack ()

extern sFlash_moto_msg    motolocal;
extern sFlash_moto_msg    mototemp;

extern u16  flash_upload_flag;

void flash_init(void);
void flash_upload_write_buff(void);


#endif