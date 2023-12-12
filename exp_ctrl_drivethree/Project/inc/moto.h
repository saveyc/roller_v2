#ifndef _MOTO_H
#define _MOTO_H

#include "main.h"


#define  B_LIMIT_SW_1_IN_STATE   IN3_1_STATE   //�ӽ�����1����
#define  B_LIMIT_SW_2_IN_STATE   IN3_2_STATE   //�ӽ�����2����
 
#define  MAX_SPEED_CCR_VALUE     7660  //10V(��Ӧ���ת��)
#define  MIN_SPEED_CCR_VALUE     1838  //2.4V(��Ӧ��Сת��)
#define  TEN_MS_RAMP_CCR_VALUE   145   //0.19V(�Ӽ����¶ȿ���ʱÿ10ms������ֵ)

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
    u16 value;                    //�Ƿ���Ч
    u16 set_start_status;         //���õ���ͣ״̬(0:ֹͣ 1:����)
    u16 set_speed_sel;            //���õ��ٶȵ�λ(0~7)(3.96V~10V)
    u16 set_dir_sel;              //���õķ���(0:CW 1:CCW)
    u16 set_ramp_sel;             //���õļӼ��ٵ�λ(0~4)(0:no ramp)
    u16 current_speed_ccr_value;  //��ǰ�ٶ����ռ�ձ�ֵ
    u16 target_speed_ccr_value;   //Ŀ���ٶ����ռ�ձ�ֵ(��Ҫ������ٶ�(����ֹͣ))
    u16 set_speed_ccr_value;      //���õ��ٶ����ռ�ձ�ֵ(����ʱ���ٶ�)
    u16 ramp_ccr_value;           //�Ӽ���ÿ10ms������ռ�ձ�ֵ
    u16 alarm_flag;               //�������״̬
    u16 opposite_dir;             //���÷���ķ�����
}MOT_Info;

typedef struct {
    u16 set_start_status;         //���õ���ͣ״̬
    u16 set_start_type;           //���õ���ͣ����   �����������ֹͣ ���Ǽ�������
    u16 roll_dir_set;             //��ת����
    u16 uploadcnt;
}sMoudleinfo;

typedef struct
{
    u8  value;                  //�Ƿ���Ч
    u8  cmd;                    //��ͣ����
    u8  dir;                    //��������
    u8  type;                   //ת������ ��������ͣ ����һֱת
}sMoudle_cmd;

extern MOT_Info mot1_info, mot2_info, mot3_info;
extern sMoudleinfo moudle1_info, moudle2_info;
extern sMoudle_cmd moudlerisecmd;

extern u16 canrecv_framecnt_rise;

extern u8 g_position_status;//��������״̬
extern u8 g_position_set;//���õ�λ��

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