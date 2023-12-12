#ifndef _MOTO_H
#define _MOTO_H

#include "main.h"


#define  B_LIMIT_SW_1_IN_STATE   IN3_1_STATE   //接近开关1输入
#define  B_LIMIT_SW_2_IN_STATE   IN3_2_STATE   //接近开关2输入
 
#define  MAX_SPEED_CCR_VALUE     7660  //10V(对应最大转速)
#define  MIN_SPEED_CCR_VALUE     1838  //2.4V(对应最小转速)
#define  TEN_MS_RAMP_CCR_VALUE   145   //0.19V(加减速坡度开启时每10ms增减的值)

#define  RAMP_DELAY_MOTO         5
#define  RAMP_DELAY_RISE         3

#define  MAX_ERR_CNT             5000


#define  RUN_STATE               1
#define  STOP_STATE              2
#define  RUN_STATE_DEFAULT       0 

enum
{
    MOT1 = 1,
    MOT2 = 2,
    MOT3 = 3,
};

enum
{
    DIR_CW = 0,
    DIR_CCW = 1
};


enum
{
    POS_INIT = 0,
    POS_UP = 1,
    POS_DOWN = 2,
    POS_ERROR = 3
};

enum
{
    DIR_FW = 0,
    DIR_BK = 1,
    DIR_LF = 2,
    DIR_RT = 3
};

typedef struct {
    u16 value;                    //是否有效
    u16 set_start_status;         //设置的启停状态(0:停止 1:启动)
    u16 set_speed_sel;            //设置的速度档位(0~7)(3.96V~10V)
    u16 set_dir_sel;              //设置的方向(0:CW 1:CCW)
    u16 set_ramp_sel;             //设置的加减速档位(0~4)(0:no ramp)
    u16 current_speed_ccr_value;  //当前速度输出占空比值
    u16 target_speed_ccr_value;   //目标速度输出占空比值(需要输出的速度(包括停止))
    u16 set_speed_ccr_value;      //设置的速度输出占空比值(运行时的速度)
    u16 ramp_ccr_value;           //加减速每10ms增减的占空比值
    u16 alarm_flag;               //电机报警状态
    u16 opposite_dir;             //设置方向的反方向
}MOT_Info;

typedef struct {
    u16 set_start_status;         //设置的启停状态
    u16 set_start_type;           //设置的启停类型   遇到光电立马停止 还是继续运行
    u16 roll_dir_set;             //旋转方向
    u16 uploadcnt;
}sMoudleinfo;

typedef struct
{
    u8  value;                  //是否有效
    u8  cmd;                    //启停命令
    u8  dir;                    //方向类型
    u8  type;                   //转动类型 遇到光电就停 还是一直转
}sMoudle_cmd;

extern MOT_Info mot1_info, mot2_info, mot3_info;
extern sMoudleinfo moudle1_info, moudle2_info;
extern sMoudle_cmd moudlerisecmd;

extern u16 canrecv_framecnt_rise;

extern u8 g_position_status;//顶升机构状态
extern u8 g_position_set;//设置的位置

extern u16 upload_state_cnt;
extern u16 upload_now;


void Mot_msg_init(void);
void Set_Mot_Roll_Paras(u8 motX, u8 speed_sel, u8 dir_sel, u8 ramp_sel);
void Mot_Set_Start_Cmd(u8 motX, u8 cmd);
void Mot_Speed_Output_Handle(void);
void MOT_Error_Detection_Process(void);
void Mot_upload_all_state(void);


void Init_Find_Position_Process(void);
void moto_upload_moudle_status(void);
void moto_set_run_cmd(u8 motX, u8 cmd, u8 type, u8 dir);
void moto_ctrl_convey_process(void);
void moto_roll_test_process(void);

#endif