#include "moto.h"
#include "main.h"

//设置电机运转参数(速度,方向,加速度)
//motX: MOT1,MOT2
//speed_sel: 0~7共八档
//dir_sel: DIR_CW,DIR_CCW
//ramp_sel: 0~4

MOT_Info mot1_info, mot2_info ,mot3_info;
sMoudleinfo moudle1_info, moudle2_info;
sMoudle_cmd moudlerisecmd;

u16 canrecv_framecnt_rise = 0xFFFF;


u16 ramp_delay = 0;
u16 upload_state_cnt = 0;
u16 ramp_delay_three = 0;


u8 g_position_status = POS_INIT;//顶升机构状态
u8 g_position_set = POS_DOWN;//设置的位置
u8 g_position_find_flag = 0;
u16 g_position_find_timeout = 0;

u16 roll_test = 0;
u8  roll_flag = 0;


void Mot_msg_init(void)
{
    mot1_info.set_start_status = STOP_STATE;
    mot2_info.set_start_status = STOP_STATE;
    mot3_info.set_start_status = STOP_STATE;
    moudle1_info.set_start_status = STOP_STATE;
    moudle2_info.set_start_status = STOP_STATE;

    //Set_Mot_Roll_Paras(MOT1, 7, DIR_CW, 2);
    //Set_Mot_Roll_Paras(MOT2, 7, DIR_CW, 2);
    //Set_Mot_Roll_Paras(MOT3, 5, DIR_CW, 7);
}

void Set_Mot_Roll_Paras(u8 motX, u8 speed_sel, u8 dir_sel, u8 ramp_sel)
{
    u16 speed_value_t;
    u16 ramp_value_t;

    if ((speed_sel > 7) || (speed_sel == 0)) {
        speed_sel = 7;
    }

    if (ramp_sel > 7) {
        speed_sel = 7;
    }
    if ((dir_sel != DIR_CW) && (dir_sel != DIR_CW)) {
        dir_sel = DIR_CCW;
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
    else if (motX == MOT3)
    {
        mot3_info.set_speed_sel = speed_sel;
        mot3_info.set_dir_sel = dir_sel;
        mot3_info.set_ramp_sel = ramp_sel;
        mot3_info.set_speed_ccr_value = speed_value_t;
        mot3_info.ramp_ccr_value = ramp_value_t;
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
    else if (motX == MOT3)
    {
        mot3_info.set_start_status = cmd;
        if (cmd == STOP_STATE) {
            mot3_info.target_speed_ccr_value = 0;
        }
        else if (cmd == RUN_STATE) {
            mot3_info.target_speed_ccr_value = mot3_info.set_speed_ccr_value;
        }
    }
}

void Mot_Speed_Output_Handle(void)
{
    if (ramp_delay < RAMP_DELAY_MOTO)
    {
        ramp_delay++;
    }
    if (ramp_delay_three < RAMP_DELAY_RISE)
    {
        ramp_delay_three++;
    }

    if (ramp_delay >= RAMP_DELAY_MOTO) {
        if (mot1_info.current_speed_ccr_value != mot1_info.target_speed_ccr_value)
        {
            if (mot1_info.ramp_ccr_value != 0)
            {
                if (ramp_delay >= RAMP_DELAY_MOTO)
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
                if (ramp_delay >= RAMP_DELAY_MOTO)
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

    if (ramp_delay_three >= RAMP_DELAY_RISE) {
        if (mot3_info.current_speed_ccr_value != mot3_info.target_speed_ccr_value)
        {
            if (mot3_info.ramp_ccr_value != 0)
            {
                if (ramp_delay_three >= RAMP_DELAY_RISE)
                {
                    if (mot3_info.current_speed_ccr_value <= mot3_info.target_speed_ccr_value)//加速
                    {
                        if (mot3_info.current_speed_ccr_value < MIN_SPEED_CCR_VALUE)
                        {
                            mot3_info.current_speed_ccr_value = MIN_SPEED_CCR_VALUE;
                        }
                        else
                        {
                            mot3_info.current_speed_ccr_value += mot3_info.ramp_ccr_value;
                        }
                        if (mot3_info.current_speed_ccr_value >= mot3_info.target_speed_ccr_value)
                        {
                            mot3_info.current_speed_ccr_value = mot3_info.target_speed_ccr_value;
                        }
                    }
                    else//减速
                    {
                        if (mot3_info.current_speed_ccr_value < MIN_SPEED_CCR_VALUE)
                        {
                            mot3_info.current_speed_ccr_value = 0;
                        }
                        else
                        {
                            mot3_info.current_speed_ccr_value -= mot3_info.ramp_ccr_value;
                        }
                        if (mot3_info.current_speed_ccr_value <= mot3_info.target_speed_ccr_value)
                        {
                            mot3_info.current_speed_ccr_value = mot3_info.target_speed_ccr_value;
                        }
                    }
                }
            }
            else
            {
                mot3_info.current_speed_ccr_value = mot3_info.target_speed_ccr_value;
            }
            TIM_SetCompare1(TIM5, mot1_info.current_speed_ccr_value);
        }

        ramp_delay_three = 0;

        if (mot3_info.set_dir_sel == DIR_CW)
        {
            MOT3_DIR(Bit_RESET);//实际输出高
        }
        else
        {
            MOT3_DIR(Bit_SET);//实际输出低
        }
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

    //检测MOT3状态
    if (input_mot3_alarm.input_state == 1)//故障或未连接
    {
        gpio_Led_Set_Mode(LED_MOT3, LED_BL);
        mot3_info.alarm_flag = 1;
        Mot_Set_Start_Cmd(MOT3, STOP_STATE);//停止
    }
    else
    {
        if (mot3_info.set_start_status == RUN_STATE)//启动状态
        {
            gpio_Led_Set_Mode(LED_MOT3, LED_ON);
        }
        else if (mot3_info.set_start_status == STOP_STATE)//停止状态
        {
            gpio_Led_Set_Mode(LED_MOT3, LED_OFF);
        }
        mot3_info.alarm_flag = 0;
    }

    //检测电压是否正常
    if (Get_Vcc_Volt() < 18.5)//电压过低
    {
        gpio_Led_Set_Mode(LED_STATUS, LED_B);
        g_alarm_flag = 0x1;
    }
    else if (Get_Vcc_Volt() > 26.5)//电压过高
    {
        gpio_Led_Set_Mode(LED_STATUS, LED_B);
        g_alarm_flag = 0x2;
    }
    else
    {
        gpio_Led_Set_Mode(LED_STATUS, LED_BL);
        g_alarm_flag = 0;
    }
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

    if (upload_state_cnt == 0) {
        return;
    }
    if (upload_state_cnt != 0) {
        upload_state_cnt--;
    }

    if (upload_state_cnt != 0) {
        return;
    }


    for (i = 0; i < 20; i++) {
        buff[i] = 0;
    }


    if (mot1_info.set_start_status == RUN_STATE) {
        buff[4] |= 1;
    }
    if (mot2_info.set_start_status == RUN_STATE) {
        buff[4] |= 1;
    }
    if (input_in1_1.input_state == VALUE) {
        buff[4] |= (1 << 1);
    }
    if (input_in1_2.input_state == VALUE) {
        buff[4] |= (1 << 2);
    }
    if (input_in2_1.input_state == VALUE) {
        buff[4] |= (1 << 3);
    }
    if (input_in2_2.input_state == VALUE) {
        buff[4] |= (1 << 4);
    }

    if (input_in3_1.input_state == VALUE) {
        buff[5] |= (1 << 1);
    }
    if (input_in3_2.input_state == VALUE) {
        buff[5] |= (1 << 2);
    }

    if ((g_alarm_flag & 0x01) == 1) {
        buff[6] |= (1 << 1);
    }
    if ((g_alarm_flag & 0x02) == 1) {
        buff[6] |= (1 << 2);
    }

    if (input_in1_1.err == VALUE) {
        buff[6] |= (1 << 4);
    }
    if (input_in1_2.err == VALUE) {
        buff[6] |= (1 << 5);
    }
    if (input_in2_1.err == VALUE) {
        buff[6] |= (1 << 6);
    }
    if (input_in2_2.err == VALUE) {
        buff[6] |= (1 << 7);
    }

    if (mot1_info.alarm_flag == VALUE) {
        buff[7] |= 1 << 3;
    }
    if (mot2_info.alarm_flag == VALUE) {
        buff[7] |= 1 << 4;
    }
    if (mot3_info.alarm_flag == VALUE) {
        buff[7] |= 1 << 7;
    }
    sendlen = 8;

    vcan_sendmsg_two(buff, sendlen, CAN_FUNC_ID_MODULE_STATUS, 1);
    vcan_sendmsg_one(buff, sendlen, CAN_FUNC_ID_MODULE_STATUS, 1);
}


//上电检测初始化顶升机构位置
//顶升机构动作执行
void Init_Find_Position_Process(void)
{

    if (g_position_status == POS_INIT)//未初始化
    {
        if (B_LIMIT_SW_2_IN_STATE)//下位
        {
            g_position_status = POS_DOWN;
        }
        else if (B_LIMIT_SW_1_IN_STATE)//上位
        {
            g_position_status = POS_UP;
        }
        else
        {
            g_position_set = POS_DOWN;//初始化到下位
        }
    }
    if ((g_position_status != g_position_set) && (g_position_find_flag == 0))
    {
        if (g_position_set == POS_DOWN)
        {
            Set_Mot_Roll_Paras(MOT3, 6, DIR_CW, 7);
            Mot_Set_Start_Cmd(MOT3, RUN_STATE);
            g_position_find_flag = 1;
            g_position_find_timeout = 0;
        }
        else if (g_position_set == POS_UP)
        {
            Set_Mot_Roll_Paras(MOT3, 6, DIR_CCW, 0);
            Mot_Set_Start_Cmd(MOT3, RUN_STATE);
            g_position_find_flag = 1;
            g_position_find_timeout = 0;
        }
    }
    if (g_position_find_flag)
    {
        if (g_position_set == POS_DOWN)//下位
        {
            if (B_LIMIT_SW_2_IN_STATE)
            {
                Mot_Set_Start_Cmd(MOT3, STOP_STATE);
                g_position_status = POS_DOWN;
                g_position_find_flag = 0;
                g_position_find_timeout = 0;
            }
            else
            {
                g_position_find_timeout++;
            }
        }
        else if (g_position_set == POS_UP)//上位
        {
            if (B_LIMIT_SW_1_IN_STATE)
            {
                Mot_Set_Start_Cmd(MOT3, STOP_STATE);
                g_position_status = POS_UP;
                g_position_find_flag = 0;
                g_position_find_timeout = 0;
            }
            else
            {
                g_position_find_timeout++;
            }
        }

        if (g_position_find_timeout > 2000)
        {
            Mot_Set_Start_Cmd(MOT3, STOP_STATE);
            g_position_status = POS_ERROR;
            g_position_set = POS_ERROR;
            g_position_find_flag = 0;
        }
    }
}

//选择一个方向的滚筒运行
void Select_Direction_Run(u8 direction)
{
    if (direction == DIR_FW)//前
    {
        g_position_set = POS_UP;//前

        Set_Mot_Roll_Paras(MOT1, mot1_info.set_speed_sel, DIR_CW, mot1_info.set_ramp_sel);
        Mot_Set_Start_Cmd(MOT1, RUN_STATE);
    }
    else if (direction == DIR_BK)//后
    {
        g_position_set = POS_UP;//上
        Set_Mot_Roll_Paras(MOT1, mot1_info.set_speed_sel, DIR_CCW, mot1_info.set_ramp_sel);
        Mot_Set_Start_Cmd(MOT1, RUN_STATE);
    }
    else if (direction == DIR_LF)//左
    {
        g_position_set = POS_DOWN;//下
        Set_Mot_Roll_Paras(MOT2, mot2_info.set_speed_sel, DIR_CW, mot2_info.set_ramp_sel);
        Mot_Set_Start_Cmd(MOT2, RUN_STATE);
    }
    else if (direction == DIR_RT)//右
    {
        g_position_set = POS_DOWN;//下
        Set_Mot_Roll_Paras(MOT2, mot2_info.set_speed_sel, DIR_CCW, mot2_info.set_ramp_sel);
        Mot_Set_Start_Cmd(MOT2, RUN_STATE);
    }
}
//选择一个方向的滚筒停止
void Select_Direction_Stop(u8 direction)
{
    if (direction == DIR_FW)//前
    {
        Mot_Set_Start_Cmd(MOT1, STOP_STATE);
    }
    else if (direction == DIR_BK)//后
    {
        Mot_Set_Start_Cmd(MOT1, STOP_STATE);
    }
    else if (direction == DIR_LF)//左
    {
        Mot_Set_Start_Cmd(MOT2, STOP_STATE);
    }
    else if (direction == DIR_RT)//右
    {
        Mot_Set_Start_Cmd(MOT2, STOP_STATE);
    }
}


void moto_upload_moudle_status(void)
{
    //MOTO1
    moudle1_info.uploadcnt++;
    if ((moudle1_info.set_start_type == RUN_TRIGSTOP) && (moudle1_info.set_start_status == RUN_STATE)) {
        if (((input_in2_1.input_state == 1)  || (input_in1_2.input_state == 1) ) && (moudle1_info.roll_dir_set == DIR_BK)) {
            Select_Direction_Stop(DIR_BK);//停止
            moudle1_info.set_start_status = STOP_STATE;
            moudle1_info.set_start_type = CONTINUE_RUN;
        }
        if (((input_in1_1.input_state == 1) || (input_in2_2.input_state == 1)) && ((moudle1_info.roll_dir_set == DIR_FW))) {
            Select_Direction_Stop(DIR_FW);//停止
            moudle1_info.set_start_status = STOP_STATE;
            moudle1_info.set_start_type = CONTINUE_RUN;
        }
    }

    if (moudle1_info.uploadcnt > 5) {
        moudle1_info.uploadcnt = 0;
        if (moudle1_info.set_start_status == RUN_STATE) {
            if (moudle1_info.roll_dir_set == DIR_FW)
            {
                Select_Direction_Run(DIR_FW);
            }
            if (moudle1_info.roll_dir_set == DIR_BK)
            {
                Select_Direction_Run(DIR_BK);
            }
        }
        if (moudle1_info.set_start_status == STOP_STATE) {
            Select_Direction_Stop(DIR_FW);//停止
        }
    }

    //MOTO2
    moudle2_info.uploadcnt++; //俩对光电接在 左前 和右后
    if ((moudle2_info.set_start_type == RUN_TRIGSTOP) && (moudle2_info.set_start_status == RUN_STATE)) {
        if (((input_in2_1.input_state == 1) | (input_in1_1.input_state == 1)) && (moudle2_info.roll_dir_set == DIR_LF)) {
            Select_Direction_Stop(DIR_LF);//停止
            moudle2_info.set_start_status = STOP_STATE;
            moudle2_info.set_start_type = CONTINUE_RUN;
        }
        if (((input_in2_2.input_state == 1) || (input_in1_2.input_state == 1)) && (moudle2_info.roll_dir_set == DIR_RT)) {
            Select_Direction_Stop(DIR_RT);//停止
            moudle2_info.set_start_status = STOP_STATE;
            moudle2_info.set_start_type = CONTINUE_RUN;
        }
    }

    if (moudle2_info.uploadcnt > 5) {
        moudle2_info.uploadcnt = 0;
        if (moudle2_info.set_start_status == RUN_STATE) {
            if (moudle2_info.roll_dir_set == DIR_LF)
            {
                Select_Direction_Run(DIR_LF);
            }
            if (moudle2_info.roll_dir_set == DIR_RT)
            {
                Select_Direction_Run(DIR_RT);
            }
        }
        if (moudle2_info.set_start_status == STOP_STATE) {
            Select_Direction_Stop(DIR_LF);//停止
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
    }
    else if (motX == MOT2)
    {
        moudle2_info.set_start_type = type;
        moudle2_info.set_start_status = cmd;
        moudle2_info.roll_dir_set = dir;
    }
}

void moto_ctrl_convey_process(void)
{
    if (moudlerisecmd.value == VALUE) {
        if (moudlerisecmd.cmd == STOP_CMD) {
            moto_set_run_cmd(MOT1, STOP_STATE, CONTINUE_RUN, DIR_BK);
            moto_set_run_cmd(MOT2, STOP_STATE, CONTINUE_RUN, DIR_LF);
            moudlerisecmd.value = INVALUE;
            return;
        }

        switch (moudlerisecmd.dir) {
        case RUN_AHEAD_TOBACK:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT1, RUN_STATE, CONTINUE_RUN, DIR_BK);
                moudlerisecmd.value = INVALUE;
            }
            if (moudlerisecmd.type == RUN_TRIGSTOP) {
                moto_set_run_cmd(MOT1, RUN_STATE, RUN_TRIGSTOP, DIR_BK);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_BACK_TOAHEAD:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT1, RUN_STATE, CONTINUE_RUN, DIR_FW);
                moudlerisecmd.value = INVALUE;
            }
            if (moudlerisecmd.type == RUN_TRIGSTOP) {
                moto_set_run_cmd(MOT1, RUN_STATE, RUN_TRIGSTOP, DIR_FW);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_LEFT_TORIGHT:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT2, RUN_STATE, CONTINUE_RUN, DIR_RT);
                moudlerisecmd.value = INVALUE;
            }
            if (moudlerisecmd.type == RUN_TRIGSTOP) {
                moto_set_run_cmd(MOT2, RUN_STATE, RUN_TRIGSTOP, DIR_RT);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_RIGHT_TOLEFT:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT2, RUN_STATE, CONTINUE_RUN, DIR_LF);
                moudlerisecmd.value = INVALUE;
            }
            if (moudlerisecmd.type == RUN_TRIGSTOP) {
                moto_set_run_cmd(MOT2, RUN_STATE, RUN_TRIGSTOP, DIR_LF);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_AHEAD_TOLEFT:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT1, RUN_STATE, RUN_TRIGSTOP, DIR_BK);
                moudlerisecmd.type = RUN_TYPEDEFAULT;
            }
            if (moudle1_info.set_start_status == STOP_STATE) {
                moto_set_run_cmd(MOT2, RUN_STATE, CONTINUE_RUN, DIR_LF);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_AHEAD_TORIGHT:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT1, RUN_STATE, RUN_TRIGSTOP, DIR_BK);
                moudlerisecmd.type = RUN_TYPEDEFAULT;
            }
            if (moudle1_info.set_start_status == STOP_STATE) {
                moto_set_run_cmd(MOT2, RUN_STATE, CONTINUE_RUN, DIR_RT);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_BACK_TOLEFT:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT1, RUN_STATE, RUN_TRIGSTOP, DIR_FW);
                moudlerisecmd.type = RUN_TYPEDEFAULT;
            }
            if (moudle1_info.set_start_status == STOP_STATE) {
                moto_set_run_cmd(MOT2, RUN_STATE, CONTINUE_RUN, DIR_LF);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_BACK_TORIGHT:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT1, RUN_STATE, RUN_TRIGSTOP, DIR_FW);
                moudlerisecmd.type = RUN_TYPEDEFAULT;
            }
            if (moudle1_info.set_start_status == STOP_STATE) {
                moto_set_run_cmd(MOT2, RUN_STATE, CONTINUE_RUN, DIR_RT);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_LEFT_TOAHEAD:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT2, RUN_STATE, RUN_TRIGSTOP, DIR_RT);
                moudlerisecmd.type = RUN_TYPEDEFAULT;
            }
            if (moudle2_info.set_start_status == STOP_STATE) {
                moto_set_run_cmd(MOT1, RUN_STATE, CONTINUE_RUN, DIR_FW);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_LEFT_TOBACK:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT2, RUN_STATE, RUN_TRIGSTOP, DIR_RT);
                moudlerisecmd.type = RUN_TYPEDEFAULT;
            }
            if (moudle2_info.set_start_status == STOP_STATE) {
                moto_set_run_cmd(MOT1, RUN_STATE, CONTINUE_RUN, DIR_BK);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_RIGHT_TOAHEAD:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT2, RUN_STATE, RUN_TRIGSTOP, DIR_LF);
                moudlerisecmd.type = RUN_TYPEDEFAULT;
            }
            if (moudle2_info.set_start_status == STOP_STATE) {
                moto_set_run_cmd(MOT1, RUN_STATE, CONTINUE_RUN, DIR_FW);
                moudlerisecmd.value = INVALUE;
            }
            break;
        case RUN_RIGHT_TOBACK:
            if (moudlerisecmd.type == CONTINUE_RUN) {
                moto_set_run_cmd(MOT2, RUN_STATE, RUN_TRIGSTOP, DIR_LF);
                moudlerisecmd.type = RUN_TYPEDEFAULT;
            }
            if (moudle2_info.set_start_status == STOP_STATE) {
                moto_set_run_cmd(MOT1, RUN_STATE, CONTINUE_RUN, DIR_BK);
                moudlerisecmd.value = INVALUE;
            }
            break;
        default:
            moudlerisecmd.value = INVALUE;
            break;

        }
    }
}

void moto_roll_test_process(void)
{
    roll_test++;
    if (roll_test > 5000)
    {
        roll_test = 0;
        if (roll_flag == 0) {
            moto_set_run_cmd(MOT1, RUN_STATE, CONTINUE_RUN, DIR_FW);
            moto_set_run_cmd(MOT2, STOP_STATE, CONTINUE_RUN, DIR_LF);
        }
        if (roll_flag == 1) {
            moto_set_run_cmd(MOT1, STOP_STATE, CONTINUE_RUN, DIR_FW);
            moto_set_run_cmd(MOT2, RUN_STATE, CONTINUE_RUN, DIR_LF);

        }

        if (roll_flag == 0) {
            roll_flag = 1;
            return;
        }
        if (roll_flag == 1) {
            roll_flag = 0;
        }
    }
}




