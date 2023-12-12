#include "moto.h"
#include "main.h"

//设置电机运转参数(速度,方向,加速度)
//motX: MOT1,MOT2
//speed_sel: 0~7共八档
//dir_sel: DIR_CW,DIR_CCW
//ramp_sel: 0~4

MOT_Info mot1_info, mot2_info;
sMoudleinfo moudle1_info, moudle2_info;

u16 cansend_framecnt_one = 0xFFFF;
u16 cansend_framecnt_two = 0xFFFF;

u16 ramp_delay = 0;
u16 upload_state_cnt = 0;
u16 upload_now = 0;


void Mot_msg_init(void)
{
    mot1_info.set_start_status = STOP_STATE;
    mot2_info.set_start_status = STOP_STATE;
    moudle1_info.set_start_status = STOP_STATE;
    moudle2_info.set_start_status = STOP_STATE;

//    Set_Mot_Roll_Paras(MOT1, 7, DIR_CW, 2);
//    Set_Mot_Roll_Paras(MOT2, 7, DIR_CW, 2);
}

void Set_Mot_Roll_Paras(u8 motX, u8 speed_sel, u8 dir_sel, u8 ramp_sel)
{
    u16 speed_value_t;
    u16 ramp_value_t;
    if ((speed_sel > 7) || (speed_sel == 0)) {
        speed_sel = 7;
    }
    if ((dir_sel != DIR_CCW) && (dir_sel != DIR_CW)) {
        dir_sel = DIR_CW;
    }

    if (ramp_sel > 7) {
        ramp_sel = 7;
    }

    speed_value_t = MIN_SPEED_CCR_VALUE + ((MAX_SPEED_CCR_VALUE - MIN_SPEED_CCR_VALUE) * (speed_sel) / 7);
    ramp_value_t = TEN_MS_RAMP_CCR_VALUE * ramp_sel;

    if (motX == MOT1)
    {
        mot1_info.set_speed_sel = speed_sel;
        mot1_info.set_dir_sel = dir_sel;
        mot1_info.set_ramp_sel = ramp_sel;
        mot1_info.set_speed_ccr_value = speed_value_t;
        mot1_info.ramp_ccr_value = ramp_value_t;
    }
    else if (motX == MOT2)
    {
        mot2_info.set_speed_sel = speed_sel;
        mot2_info.set_dir_sel = dir_sel;
        mot2_info.set_ramp_sel = ramp_sel;
        mot2_info.set_speed_ccr_value = speed_value_t;
        mot2_info.ramp_ccr_value = ramp_value_t;
    }
}

//设置电机运行/停止
void Mot_Set_Start_Cmd(u8 motX, u8 cmd)
{
    if (motX == MOT1)
    {
        mot1_info.set_start_status = cmd;
        if (cmd == STOP_STATE) {
            mot1_info.target_speed_ccr_value = 0;
        }
        else if (cmd == RUN_STATE) {
            mot1_info.target_speed_ccr_value = mot1_info.set_speed_ccr_value;
        }
    }
    else if (motX == MOT2)
    {
        mot2_info.set_start_status = cmd;
        if (cmd == STOP_STATE) {
            mot2_info.target_speed_ccr_value = 0;
        }
        else if (cmd == RUN_STATE) {
            mot2_info.target_speed_ccr_value = mot2_info.set_speed_ccr_value;
        }
    }
}

void Mot_Speed_Output_Handle(void)
{
    if (ramp_delay < 10)
    {
        ramp_delay++;
        return;
    }

    if (mot1_info.current_speed_ccr_value != mot1_info.target_speed_ccr_value)
    {
        if (mot1_info.ramp_ccr_value != 0)
        {
            if (ramp_delay >= 10)
            {
                if (mot1_info.current_speed_ccr_value <= mot1_info.target_speed_ccr_value)//加速
                {
                    if (mot1_info.current_speed_ccr_value < MIN_SPEED_CCR_VALUE)
                    {
                        mot1_info.current_speed_ccr_value = MIN_SPEED_CCR_VALUE;
                    }
                    else
                    {
                        mot1_info.current_speed_ccr_value += mot1_info.ramp_ccr_value;
                    }
                    if (mot1_info.current_speed_ccr_value >= mot1_info.target_speed_ccr_value)
                    {
                        mot1_info.current_speed_ccr_value = mot1_info.target_speed_ccr_value;
                    }
                }
                else//减速
                {
                    if (mot1_info.current_speed_ccr_value < MIN_SPEED_CCR_VALUE)
                    {
                        mot1_info.current_speed_ccr_value = 0;
                    }
                    else
                    {
                        mot1_info.current_speed_ccr_value -= mot1_info.ramp_ccr_value;
                    }
                    if (mot1_info.current_speed_ccr_value <= mot1_info.target_speed_ccr_value)
                    {
                        mot1_info.current_speed_ccr_value = mot1_info.target_speed_ccr_value;
                    }
                }
            }
        }
        else
        {
            mot1_info.current_speed_ccr_value = mot1_info.target_speed_ccr_value;
        }
        TIM_SetCompare1(TIM3, mot1_info.current_speed_ccr_value);
    }
    if (mot2_info.current_speed_ccr_value != mot2_info.target_speed_ccr_value)
    {
        if (mot2_info.ramp_ccr_value != 0)
        {
            if (ramp_delay >= 10)
            {
                if (mot2_info.current_speed_ccr_value < mot2_info.target_speed_ccr_value)//加速
                {
                    if (mot2_info.current_speed_ccr_value < MIN_SPEED_CCR_VALUE)
                    {
                        mot2_info.current_speed_ccr_value = MIN_SPEED_CCR_VALUE;
                    }
                    else
                    {
                        mot2_info.current_speed_ccr_value += mot2_info.ramp_ccr_value;
                    }
                    if (mot2_info.current_speed_ccr_value >= mot2_info.target_speed_ccr_value)
                    {
                        mot2_info.current_speed_ccr_value = mot2_info.target_speed_ccr_value;
                    }
                }
                else//减速
                {
                    if (mot2_info.current_speed_ccr_value <= MIN_SPEED_CCR_VALUE)
                    {
                        mot2_info.current_speed_ccr_value = 0;
                    }
                    else
                    {
                        mot2_info.current_speed_ccr_value -= mot2_info.ramp_ccr_value;
                    }
                    if (mot2_info.current_speed_ccr_value <= mot2_info.target_speed_ccr_value)
                    {
                        mot2_info.current_speed_ccr_value = mot2_info.target_speed_ccr_value;
                    }
                }
            }
        }
        else
        {
            mot2_info.current_speed_ccr_value = mot2_info.target_speed_ccr_value;
        }
        TIM_SetCompare2(TIM3, mot2_info.current_speed_ccr_value);
    }
    ramp_delay = 0;

    if (mot1_info.set_dir_sel == DIR_CW)
    {
        MOT1_DIR(Bit_RESET);//实际输出高
    }
    else
    {
        MOT1_DIR(Bit_SET);//实际输出低
    }
    if (mot2_info.set_dir_sel == DIR_CW)
    {
        MOT2_DIR(Bit_RESET);//实际输出高
    }
    else
    {
        MOT2_DIR(Bit_SET);//实际输出低
    }
}

//故障状态检测
void MOT_Error_Detection_Process(void)
{
    //检测MOT1状态
    if (input_mot1_alarm.input_state == 1)//故障或未连接
    {
        gpio_Led_Set_Mode(LED_MOT1, LED_BL);
        mot1_info.alarm_flag = 0x01;
        if (mot1_info.set_start_status == RUN_STATE)
        {
            moudle1_info.set_start_status = STOP_STATE;
            Mot_Set_Start_Cmd(MOT1, STOP_STATE);//停止
        }
    }
    else
    {
        if (mot1_info.set_start_status == RUN_STATE)//启动状态
        {
            gpio_Led_Set_Mode(LED_MOT1, LED_ON);
        }
        else if (mot1_info.set_start_status == STOP_STATE)//停止状态
        {
            gpio_Led_Set_Mode(LED_MOT1, LED_OFF);
        }
        mot1_info.alarm_flag = 0;
    }
    //检测MOT2状态
    if (input_mot2_alarm.input_state == 1)//故障或未连接
    {
        gpio_Led_Set_Mode(LED_MOT2, LED_BL);
        mot2_info.alarm_flag = 1;
        if (mot2_info.set_start_status == RUN_STATE)
        {
            moudle2_info.set_start_status = STOP_STATE;
            Mot_Set_Start_Cmd(MOT2, STOP_STATE);//停止
        }
    }
    else
    {
        if (mot2_info.set_start_status == RUN_STATE)//启动状态
        {
            gpio_Led_Set_Mode(LED_MOT2, LED_ON);
        }
        else if (mot2_info.set_start_status == STOP_STATE)//停止状态
        {
            gpio_Led_Set_Mode(LED_MOT2, LED_OFF);
        }
        mot2_info.alarm_flag = 0;
    }

    //检测电压是否正常
//    if (Get_Vcc_Volt() < 18.5)//电压过低
//    {
//        gpio_Led_Set_Mode(LED_STATUS, LED_B);
//        g_alarm_flag = 0x1;
//    }
//    else if (Get_Vcc_Volt() > 26.5)//电压过高
//    {
//        gpio_Led_Set_Mode(LED_STATUS, LED_B);
//        g_alarm_flag = 0x2;
//    }
//    else
//    {
        gpio_Led_Set_Mode(LED_STATUS, LED_BL);
        g_alarm_flag = 0;
//    }

    if (g_alarm_flag != 0)
    {
        if (mot1_info.set_start_status == RUN_STATE)
        {
            moudle1_info.set_start_status = STOP_STATE;
            Mot_Set_Start_Cmd(MOT1, STOP_STATE);//停止
        }
        if (mot2_info.set_start_status == RUN_STATE)
        {
            moudle2_info.set_start_status = STOP_STATE;
            Mot_Set_Start_Cmd(MOT2, STOP_STATE);//停止
        }
    }
    //检测温度是否正常
//    if(Get_Internal_Temperate() > 90.0)
//    {
//        Led_Set_Mode(LED_STATUS,LED_B);
//        alarm_flag |= 0x2;
//    }
//    else
//    {
//        alarm_flag &= ~0x2;
//    }
}

void Mot_upload_all_state(void)
{
    u8 buff[20] = { 0 };
    u16 sendlen = 0;
    u16 i = 0;

    
    if (upload_now == 0) {
        if (upload_state_cnt == 0) {
            return;
        }
        if (upload_state_cnt != 0) {
            upload_state_cnt--;
        }

        if (upload_state_cnt != 0) {
            return;
        }
    }
    upload_now = 0;

    for (i = 0; i < 20; i++) {
        buff[i] = 0;
    }


    if (mot1_info.set_start_status == RUN_STATE) {
        buff[0] |= 1;
    }
    if (input_in1_1.input_state == VALUE) {
        buff[0] |= (1 << 1);
    }
    if (input_in1_2.input_state == VALUE) {
        buff[0] |= (1 << 2);
    }

    if (mot1_info.alarm_flag == VALUE) {
        buff[1] |= 1;
    }
    if ((g_alarm_flag & 0x01) == 1) {
        buff[1] |= (1 << 1);
    }
    if ((g_alarm_flag & 0x02) == 1) {
        buff[1] |= (1 << 2);
    }
    if (input_in1_1.err == VALUE) {
        buff[1] |= (1 << 4);
    }
    if (input_in1_2.err == VALUE) {
        buff[1] |= (1 << 5);
    }


    if (mot2_info.set_start_status == RUN_STATE) {
        buff[2] |= 1;
    }
    if (input_in2_1.input_state == VALUE) {
        buff[2] |= (1 << 1);
    }
    if (input_in2_2.input_state == VALUE) {
        buff[2] |= (1 << 2);
    }

    if (mot2_info.alarm_flag == VALUE) {
        buff[3] |= 1;
    }
    if ((g_alarm_flag & 0x01) == 1) {
        buff[3] |= (1 << 1);
    }
    if ((g_alarm_flag & 0x02) == 1) {
        buff[3] |= (1 << 2);
    }
    if (input_in2_1.err == VALUE) {
        buff[3] |= (1 << 4);
    }
    if (input_in2_2.err == VALUE) {
        buff[3] |= (1 << 5);
    }
    buff[4] = 0;
    buff[5] = 0;
    buff[6] = 0;
    buff[7] = 0;
    sendlen = 8;

    vcan_sendmsg_two(buff, sendlen, CAN_FUNC_ID_MODULE_STATUS, 1);
    vcan_sendmsg_one(buff, sendlen, CAN_FUNC_ID_MODULE_STATUS, 1);

}

void moto_upload_moudle_status(void)
{
    //MOTO1
    moudle1_info.uploadcnt++;
    if ((moudle1_info.set_start_type == RUN_TRIGSTOP) && (moudle1_info.set_start_status == RUN_STATE)) {
        if ((input_in1_1.input_state == 1) && (moudle1_info.roll_dir_set == RUN_AHEAD_TOBACK)) {
            Mot_Set_Start_Cmd(MOT1, STOP_STATE);//停止
            moudle1_info.set_start_status = STOP_STATE;
            moudle1_info.set_start_type = CONTINUE_RUN;
        }
        if ((input_in1_2.input_state == 1) && (moudle1_info.roll_dir_set == RUN_BACK_TOAHEAD)) {
            Mot_Set_Start_Cmd(MOT1, STOP_STATE);//停止
            moudle1_info.set_start_status = STOP_STATE;
            moudle1_info.set_start_type = CONTINUE_RUN;
        }
    }


    if (moudle1_info.uploadcnt > 50) {
        moudle1_info.uploadcnt = 0;
        if (moudle1_info.set_start_status == RUN_STATE) {
            Mot_Set_Start_Cmd(MOT1, RUN_STATE);//启动
        }
        if (moudle1_info.set_start_status == STOP_STATE) {
            Mot_Set_Start_Cmd(MOT1, STOP_STATE);//停止
        }
    }

    //MOTO2
    moudle2_info.uploadcnt++;
    if ((moudle2_info.set_start_type == RUN_TRIGSTOP) && (moudle2_info.set_start_status == RUN_STATE)) {
        if ((input_in2_1.input_state == 1) && (moudle2_info.roll_dir_set == RUN_AHEAD_TOBACK)) {
            Mot_Set_Start_Cmd(MOT2, STOP_STATE);//停止
            moudle2_info.set_start_status = STOP_STATE;
            moudle2_info.set_start_type = CONTINUE_RUN;
        }
        if ((input_in2_2.input_state == 1) && (moudle2_info.roll_dir_set == RUN_BACK_TOAHEAD)) {
            Mot_Set_Start_Cmd(MOT2, STOP_STATE);//停止
            moudle2_info.set_start_status = STOP_STATE;
            moudle2_info.set_start_type = CONTINUE_RUN;
        }
    }

    if (moudle2_info.uploadcnt > 50) {
        moudle2_info.uploadcnt = 0;
        if (moudle2_info.set_start_status == RUN_STATE) {
            Mot_Set_Start_Cmd(MOT2, RUN_STATE);//启动
        }
        if (moudle2_info.set_start_status == STOP_STATE) {
            Mot_Set_Start_Cmd(MOT2, STOP_STATE);//停止
        }
    }
}

void moto_set_run_cmd(u8 motX, u8 cmd, u8 type, u8 dir)
{
    if (motX == MOT1)
    {
        moudle1_info.set_start_type = type;
        moudle1_info.set_start_status = cmd;
        moudle1_info.roll_dir_set = dir;
        if (moudle1_info.roll_dir_set == RUN_BACK_TOAHEAD) {
            Set_Mot_Roll_Paras(MOT1, mot1_info.set_speed_sel, DIR_CW, mot1_info.set_ramp_sel);
        }
        if (moudle1_info.roll_dir_set == RUN_AHEAD_TOBACK) {
            Set_Mot_Roll_Paras(MOT1, mot1_info.set_speed_sel, DIR_CCW, mot1_info.set_ramp_sel);
        }
    }
    else if (motX == MOT2)
    {
        moudle2_info.set_start_type = type;
        moudle2_info.set_start_status = cmd;
        moudle2_info.roll_dir_set = dir;

        if (moudle2_info.roll_dir_set == RUN_BACK_TOAHEAD) {
            Set_Mot_Roll_Paras(MOT2, mot2_info.set_speed_sel, DIR_CW, mot2_info.set_ramp_sel);
        }
        if (moudle2_info.roll_dir_set == RUN_AHEAD_TOBACK) {
            Set_Mot_Roll_Paras(MOT2, mot2_info.set_speed_sel, DIR_CCW, mot2_info.set_ramp_sel);
        }
    }
}

