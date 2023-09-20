#ifndef _DATA_H
#define _DATA_H

#include "list.h"
#include "main.h"

#define   BELT_ZONE_NUM                    200

#define   ZONE_NUM                         3

#define   MAX_PKG_NUM                      20

#define   VALUE                            1
#define   INVALUE                          0

#define   FINAL_CAR                       0xFFFF
#define   START_CAR                       0xFFFE


#define   PKGQUEUELEN                     30


enum {
	ZONE_TYPE_ONE = 0x01,
	ZONE_TYPE_TWO = 0x02,
	ZONE_TYPE_RISE = 0x03,
};


enum {
	TRANS_ALLOW = 0,                      //允许分拣
	TRANS_FORBID = 1,                     //禁止分拣
	TRANS_ABORT = 2,                      //取消分拣
	TRANS_SUCCESS = 3,                    //成功分拣
};

#pragma pack (1)

//配置
//区域节点
typedef struct {
	u16  zoneIndex;                        //区域编号  编号与站号 无必然联系， 默认0xFFFF 无效;
	u16  ctrlIndex;                        //控制器站号 
	u16  beltMoudleIndex;                  //所属从机区域  第一区域(0x01) 第二区域(0x02) 顶升区域（0x03)
	u16  front_zone;                       //前方区域          规定前方区域  
	u16  rear_zone;                        //后方区域
	u16  left_zone;                        //左方区域          规定左方区域（电滚筒正转 注意安装位置）   
	u16  right_zone;                       //右方区域  
}sData_zone_node;

//区域配置
typedef struct {
	u16   flashcnt;                               //更新flash
	u32   confid;                                 //配置的id
	u16   pkgtotalnum;                            //总共有多少包数据
	u16   pkgcurnum;                              //当前是第几包数据
	u16   pkgcal;                                 //数据包统计
	u16   zonetotalnum;                           //总共有多少数据
	u16   zonecurnum;                             //当前包数据有多少
	sData_zone_node  zoneNode[BELT_ZONE_NUM];     //区域节点信息
}sData_zone_config;

//包裹相关信息


//配置各个区域参数
typedef struct {
	u16 zoneIndex;                        //模块1(0x01) 模块2(0x02)  顶升模块(0xA0)(顶升模块不需要配置) 0xFF为默认值
	u16 FuncSeclet;                       //功能配置
	u16 speed;                            //速度挡位 4档	
	u16 dir;                              //方向  默认cw正向
	u16 ramp;                             //加减速度挡位
}sData_belt_node;
//每一段区域的配置
typedef struct {
	u8  version[4];                        //模块版本号低字节
	u16 slaverIndex;                       //控制器站号
	u16 slaver_zonenum;                    //该从机有效的的区域数量
	sData_belt_node  belt_node[ZONE_NUM];  //从机区域参数
}sData_belt_msg;


//实时信息
typedef struct {
	u16 zoneIndex;                     //区域编号
	u16 zoneState;                     // v1
									   // 运行状态 bit0运行状态   Bit1区域光电1触发  bit2 区域光电2触发 
	                                   // bit3 无货 bit4有货 bit5 正在出货 bit6等待接货  bit7货物成功传输到位 bit8 顶升区域光电2-0触发(对应电滚筒1）
	                                   // bit9 顶升区域光电2-1触发（电滚筒1）bit10 顶升区域光电2-2触发（电滚筒2）bit11 顶升区域光电2-3触发（电滚筒2）
	                                   // bit12 顶升上限光电触发 bit13 顶升下限光电触发
									   // v2 20230803
	                                   // 运行状态 bit0运行状态   Bit1区域光电1触发 (顶升区域光电1-0触发,对应电滚筒1）  bit2 区域光电2触发 (bit9 顶升区域光电1-1触发电滚筒1）
									   // bit3 顶升区域光电2-1触发（电滚筒2）bit4 顶升区域光电2-2触发（电滚筒2）
									   // bit5 无货 bit6有货 bit7 正在出货 bit8等待接货  bit9 顶升上限光电触发 bit10 顶升下限光电触发
	u16 zoneAlarm;                     // 报警状态 Bit0  顶升模块电机1 电机故障/未连接  bit1 欠压 bit 2 过压 bit3过热
	                                   // bit4 区域光电1故障 顶升区域光电1-0故障(对应电滚筒1）  Bit5区域光电2故障  顶升区域光电1-1故障(对应电滚筒1）
									   // bit6 顶升区域光电2-0故障(对应电滚筒2）  Bit7区域光电2故障  顶升区域光电2-1故障(对应电滚筒2）
									   // bit8 该模块未连接  Bit9  区域接货超时  Bit10  区域出货超时 Bit11 顶升模块电机2/未连接  Bit12 顶升模块电机故障3/未连接  	                                  
	u32 zonePkg;                       // 包裹编号 预留  默认0
	u16 transsuccess;                  // 成功分拣
}sData_RealtimeState;

typedef struct {                                 //区域配置标识符
	u16 ctrlindex;
	u16 moudlenum;
	sData_RealtimeState  node[ZONE_NUM];
}sData_Realtime_statenode;

typedef struct {
	sData_Realtime_statenode  state[BELT_ZONE_NUM];
}sbelt_Moudle_state;


//包裹运输节点
typedef struct {
	u32   pkgid;
	u16   result;
}sPkgResultNode;

#pragma pack ()

//包裹信息
typedef struct {
	struct xLIST_ITEM* index;
	u32 pkgId;                              //货物编号
	u16 totalzoneNum;                       //包裹需要途径的区域数量
	u16 zoneindex[BELT_ZONE_NUM];           //包裹途径的具体路径
	u16 lastZonenum;                        //包裹上一次所属区域索引
	u16 curZonenum;                         //当前包裹所属区域索引
	u16 curZoneIndex;                       //当前的区域编号
	u16 curZoneStatus;                      //包裹在当前区域的状态
	u16 curZoneCnt;                         //包裹处于当前区域的时间
	u16 curZonedir;                         //包裹在当前区域的传输方向
	u16 curZoneruntype;                     //当前区域包裹运行类型
	u16 curZoneArrive;                      //到达当前区域
	u16 nextZoneIndex;                      //欲前往的下一段区域的编号
	u16 nextZoneStatus;                     //包裹在欲前往的下一段区域的状态
	u16 nextZoneCnt;                        //欲前往的下一段区域的接货时间
	u16 nextZonedir;                        //包裹欲前往的下一个区域的传输方向
	u16 nextZoneruntype;                    //前往的下一个区域的包裹运行类型
//	u16 nextZonelastpkgstat;                //包裹欲前往的下一个区域的之前的包裹状态
	u16 nextZonecurpkgstat;                 //包裹欲前往的下一个区域的当前的包裹状态
	u16 nextZonepkgstatchange;              //包裹欲前往的下一个区域的当前的包裹状态变化
	u16 nextZonechangenum;                  //包裹欲前往的下一个区域的当前的包裹状态变化次数
	u16 theThirdZone;                       //下一个区域的下一个区域
    u16 allowState;                         //禁止运输、允许运送、取消运输
	u16 sendcmdcnt;                         //给从板卡发送启停命令计时
}sData_pkg_node;

typedef struct {
	sPkgResultNode* queue;     //结果队列节点
	u16 front, rear, len;     
	u16 maxsize;               
}sPkgResultQueue;

//区域配置信息
extern sData_zone_config     zoneConfig;
extern sData_zone_config     zonetmp;
//滚筒配置参数
extern sData_belt_msg        beltMoudlepara;
//滚筒模块实时参数
extern sbelt_Moudle_state    beltMoudlestate;


extern sData_pkg_node        pgkNodeDateItem[];
extern struct xLIST_ITEM     pkgNodeListItem[];


//包裹结果队列
extern sPkgResultNode        pkgresultnode[];
extern sPkgResultQueue       pkgresultqueue;

void data_msg_init(void);
void data_pkg_list_init(void);
void data_addto_pkg_list(sData_pkg_node x);
sData_zone_node* data_find_zone_moudlestate(u16 index);
sData_RealtimeState* data_find_ctrl_status(u16 ctrlindex, u16 moudleindex);
u8 data_find_nearzone_direction(sData_zone_node node, u16 nextzone);
u8 data_config_moudle_rundir(u8 predir, u8 nextdir);

void DatainitpkgQueue(void);
void DataaddPkgQueue(sPkgResultNode x);
sPkgResultNode* DatagetmsgfromQueue(void);


#endif