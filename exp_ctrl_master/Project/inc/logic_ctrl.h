#ifndef _LOGIC_CTRL_H
#define _LOGIC_CTRL_H

#include "main.h"

extern u16 cansend_framecnt_one[];
extern u16 cansend_framecnt_two[];
extern u16 cansend_framecnt_rise[];

// 轮询发送指令的轮询时间
#define   SEND_STARTCMD_CNT  50

// 包裹在一个模块上理论的最大运输时间  大于则异常
#define   LOGIC_TRANS_MAXTIME  10000

// 包裹结束后一段时间 前一段模块停止
#define   LOGIC_TRANS_CHUTDOWNTIME  2000  

enum {
	DIR_NONE = 0,
	FRONT_DIR = 1,
	BEHIND_DIR =2,
	LEFT_DIR =3,
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
typedef struct
{
	u8  moudle;                 //所属控制器区域
	u8  cmd;                    //启停命令
	u8  dir;                    //方向类型
	u8  type;                   //转动类型 遇到光电就停 还是一直转
}sMoudle_cmd;

//命令队列
typedef struct
{
	sMoudle_cmd cmd;          //命令队列
	u16 chutDown;             //倒计时
	u16 ctrlIndex;            //控制器站号索引
	u16 value;                //是否有效
}sModule_node;

#pragma pack ()

extern sModule_node   moduleNode[];

void logic_pkg_trans_process(void);

void logicModuleNodeInit(void);
void logicAddModuleNodeQueue(sModule_node x);
void logicDealWithModuleNodeQueue(void);



#endif