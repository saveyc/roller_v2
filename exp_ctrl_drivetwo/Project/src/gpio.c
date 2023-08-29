#include "gpio.h"
#include "main.h"


sInput_Info input_in1_1;
sInput_Info input_in1_2;
sInput_Info input_in2_1;
sInput_Info input_in2_2;
sInput_Info input_x1;
sInput_Info input_x2;
sInput_Info input_mot1_alarm;
sInput_Info input_mot2_alarm;
sInput_Info input_dip_speed;
sInput_Info input_dip_dir;
sInput_Info input_dip_mod;

u16 led_blink_cnt;
u8  led_status_blk_mode;
u8  led_mot1_blk_mode;
u8  led_mot2_blk_mode;
u8  g_alarm_flag;//全局报警状态(bit0:低压报警,bit1:高压报警)(不包括电机报警)

u8  localStation = 0;



void gpio_handleInputScan()
{

    u8 dip_speed_value_t = SPEED_DIP_S1 | (SPEED_DIP_S2 << 1) | (SPEED_DIP_S3 << 2) | (SPEED_DIP_S4 << 3);
    u8 dip_dir_value_t = DIR_DIP_S1 | (DIR_DIP_S2 << 1) | (DIR_DIP_S3 << 2) | (DIR_DIP_S4 << 3);
    u8 dip_mod_value_t = MOD_DIP_S1 | (MOD_DIP_S2 << 1) | (MOD_DIP_S3 << 2) | (MOD_DIP_S4 << 3) | (MOD_DIP_S5 << 4) | (MOD_DIP_S6 << 5) | (MOD_DIP_S7 << 6) | (MOD_DIP_S8 << 7);


    // handle dip_speed single
    if (input_dip_speed.input_state != dip_speed_value_t
        && input_dip_speed.input_confirm_times == 0)
    {
        input_dip_speed.input_middle_state = dip_speed_value_t;
    }
    if (input_dip_speed.input_middle_state == dip_speed_value_t
        && input_dip_speed.input_middle_state != input_dip_speed.input_state)
    {
        input_dip_speed.input_confirm_times++;
        if (input_dip_speed.input_confirm_times > 10)
        {
            input_dip_speed.input_state = input_dip_speed.input_middle_state;
            input_dip_speed.input_confirm_times = 0;
        }
    }
    else
    {
        input_dip_speed.input_middle_state = input_dip_speed.input_state;
        input_dip_speed.input_confirm_times = 0;
    }

    // handle dip_dir single
    if (input_dip_dir.input_state != dip_dir_value_t
        && input_dip_dir.input_confirm_times == 0)
    {
        input_dip_dir.input_middle_state = dip_dir_value_t;
    }
    if (input_dip_dir.input_middle_state == dip_dir_value_t
        && input_dip_dir.input_middle_state != input_dip_dir.input_state)
    {
        input_dip_dir.input_confirm_times++;
        if (input_dip_dir.input_confirm_times > 10)
        {
            input_dip_dir.input_state = input_dip_dir.input_middle_state;
            input_dip_dir.input_confirm_times = 0;
        }
    }
    else
    {
        input_dip_dir.input_middle_state = input_dip_dir.input_state;
        input_dip_dir.input_confirm_times = 0;
    }

    // handle dip_mod single
    if (input_dip_mod.input_state != dip_mod_value_t
        && input_dip_mod.input_confirm_times == 0)
    {
        input_dip_mod.input_middle_state = dip_mod_value_t;
    }
    if (input_dip_mod.input_middle_state == dip_mod_value_t
        && input_dip_mod.input_middle_state != input_dip_mod.input_state)
    {
        input_dip_mod.input_confirm_times++;
        if (input_dip_mod.input_confirm_times > 10)
        {
            input_dip_mod.input_state = input_dip_mod.input_middle_state;
            input_dip_mod.input_confirm_times = 0;
        }
    }
    else
    {
        input_dip_mod.input_middle_state = input_dip_mod.input_state;
        input_dip_mod.input_confirm_times = 0;
    }

    localStation = input_dip_mod.input_state;



    // handle IN1-1 single
    input_in1_1.err = INVALUE;
    if (input_in1_1.input_state != IN1_1_STATE
        && input_in1_1.input_confirm_times == 0)
    {
        input_in1_1.input_middle_state = IN1_1_STATE;
    }
    if (input_in1_1.input_middle_state == IN1_1_STATE
        && input_in1_1.input_middle_state != input_in1_1.input_state)
    {
        input_in1_1.input_confirm_times++;
        if (input_in1_1.input_confirm_times > 10)
        {
            input_in1_1.input_state = input_in1_1.input_middle_state;
            input_in1_1.input_confirm_times = 0;
            if (input_in1_1.input_state == 1)
            {
                input_in1_1.input_trig_mode = INPUT_TRIG_UP;
                input_in1_1.level = HIGH_LEVEL;
            }
            else
            {
                input_in1_1.input_trig_mode = INPUT_TRIG_DOWN;
                input_in1_1.level = LOW_LEVEL;
                input_in1_1.cnt = 0;
            }
        }
    }
    else
    {
        input_in1_1.input_middle_state = input_in1_1.input_state;
        input_in1_1.input_confirm_times = 0;
    }
    if (input_in1_1.level == HIGH_LEVEL) {
        input_in1_1.cnt++;
        if (input_in1_1.cnt >= MAX_ERR_CNT) {
            input_in1_1.cnt = MAX_ERR_CNT;
            input_in1_1.err = VALUE;
        }
    }

    // handle IN1-2 single
    input_in1_2.err = INVALUE;
    if (input_in1_2.input_state != IN1_2_STATE
        && input_in1_2.input_confirm_times == 0)
    {
        input_in1_2.input_middle_state = IN1_2_STATE;
    }
    if (input_in1_2.input_middle_state == IN1_2_STATE
        && input_in1_2.input_middle_state != input_in1_2.input_state)
    {
        input_in1_2.input_confirm_times++;
        if (input_in1_2.input_confirm_times > 10)
        {
            input_in1_2.input_state = input_in1_2.input_middle_state;
            input_in1_2.input_confirm_times = 0;
            if (input_in1_2.input_state == 1)
            {
                input_in1_2.input_trig_mode = INPUT_TRIG_UP;
                input_in1_2.level = HIGH_LEVEL;
            }
            else
            {
                input_in1_2.input_trig_mode = INPUT_TRIG_DOWN;
                input_in1_2.level = LOW_LEVEL;
                input_in1_2.cnt = 0;
            }
        }
    }
    else
    {
        input_in1_2.input_middle_state = input_in1_2.input_state;
        input_in1_2.input_confirm_times = 0;
    }
    if (input_in1_2.level == HIGH_LEVEL) {
        input_in1_2.cnt++;
        if (input_in1_2.cnt >= MAX_ERR_CNT) {
            input_in1_2.cnt = MAX_ERR_CNT;
            input_in1_2.err = VALUE;
        }
    }

    // handle IN2-1 single
    input_in2_1.err = INVALUE;
    if (input_in2_1.input_state != IN2_1_STATE
        && input_in2_1.input_confirm_times == 0)
    {
        input_in2_1.input_middle_state = IN2_1_STATE;
    }
    if (input_in2_1.input_middle_state == IN2_1_STATE
        && input_in2_1.input_middle_state != input_in2_1.input_state)
    {
        input_in2_1.input_confirm_times++;
        if (input_in2_1.input_confirm_times > 10)
        {
            input_in2_1.input_state = input_in2_1.input_middle_state;
            input_in2_1.input_confirm_times = 0;
            if (input_in2_1.input_state == 1)
            {
                input_in2_1.input_trig_mode = INPUT_TRIG_UP;
                input_in2_1.level = HIGH_LEVEL;
            }
            else
            {
                input_in2_1.input_trig_mode = INPUT_TRIG_DOWN;
                input_in2_1.level = LOW_LEVEL;
                input_in2_1.cnt = 0;
            }
        }
    }
    else
    {
        input_in2_1.input_middle_state = input_in2_1.input_state;
        input_in2_1.input_confirm_times = 0;
    }
    if (input_in2_1.level == HIGH_LEVEL) {
        input_in2_1.cnt++;
        if (input_in2_1.cnt >= MAX_ERR_CNT) {
            input_in2_1.cnt = MAX_ERR_CNT;
            input_in2_1.err = VALUE;
        }
    }

    // handle IN2-2 single
    input_in2_2.err = INVALUE;
    if (input_in2_2.input_state != IN2_2_STATE
        && input_in2_2.input_confirm_times == 0)
    {
        input_in2_2.input_middle_state = IN2_2_STATE;
    }
    if (input_in2_2.input_middle_state == IN2_2_STATE
        && input_in2_2.input_middle_state != input_in2_2.input_state)
    {
        input_in2_2.input_confirm_times++;
        if (input_in2_2.input_confirm_times > 10)
        {
            input_in2_2.input_state = input_in2_2.input_middle_state;
            input_in2_2.input_confirm_times = 0;
            if (input_in2_2.input_state == 1)
            {
                input_in2_2.input_trig_mode = INPUT_TRIG_UP;
                input_in2_2.level = HIGH_LEVEL;
            }
            else
            {
                input_in2_2.input_trig_mode = INPUT_TRIG_DOWN;
                input_in2_2.level = LOW_LEVEL;
                input_in2_2.cnt = 0;
            }
        }
    }
    else
    {
        input_in2_2.input_middle_state = input_in2_2.input_state;
        input_in2_2.input_confirm_times = 0;
    }
    if (input_in2_2.level == HIGH_LEVEL) {
        input_in2_2.cnt++;
        if (input_in2_2.cnt >= MAX_ERR_CNT) {
            input_in2_2.cnt = MAX_ERR_CNT;
            input_in2_2.err = VALUE;
        }
    }


    // handle INX1 single
    if (input_x1.input_state != X1_IN_STATE
        && input_x1.input_confirm_times == 0)
    {
        input_x1.input_middle_state = X1_IN_STATE;
    }
    if (input_x1.input_middle_state == X1_IN_STATE
        && input_x1.input_middle_state != input_x1.input_state)
    {
        input_x1.input_confirm_times++;
        if (input_x1.input_confirm_times > 10)
        {
            input_x1.input_state = input_x1.input_middle_state;
            input_x1.input_confirm_times = 0;
            if (input_x1.input_state == 1)
            {
                input_x1.input_trig_mode = INPUT_TRIG_UP;
            }
            else
            {
                input_x1.input_trig_mode = INPUT_TRIG_DOWN;
            }
        }
    }
    else
    {
        input_x1.input_middle_state = input_x1.input_state;
        input_x1.input_confirm_times = 0;
    }

    // handle INX2 single
    if (input_x2.input_state != X2_IN_STATE
        && input_x2.input_confirm_times == 0)
    {
        input_x2.input_middle_state = X2_IN_STATE;
    }
    if (input_x2.input_middle_state == X2_IN_STATE
        && input_x2.input_middle_state != input_x2.input_state)
    {
        input_x2.input_confirm_times++;
        if (input_x2.input_confirm_times > 10)
        {
            input_x2.input_state = input_x2.input_middle_state;
            input_x2.input_confirm_times = 0;
            if (input_x2.input_state == 1)
            {
                input_x2.input_trig_mode = INPUT_TRIG_UP;
            }
            else
            {
                input_x2.input_trig_mode = INPUT_TRIG_DOWN;
            }
        }
    }
    else
    {
        input_x2.input_middle_state = input_x2.input_state;
        input_x2.input_confirm_times = 0;
    }


    // handle MOT1_ALARM single
    if (input_mot1_alarm.input_state != MOT1_ALARM_IN
        && input_mot1_alarm.input_confirm_times == 0)
    {
        input_mot1_alarm.input_middle_state = MOT1_ALARM_IN;
    }
    if (input_mot1_alarm.input_middle_state == MOT1_ALARM_IN
        && input_mot1_alarm.input_middle_state != input_mot1_alarm.input_state)
    {
        input_mot1_alarm.input_confirm_times++;
        if (input_mot1_alarm.input_confirm_times > 100)
        {
            input_mot1_alarm.input_state = input_mot1_alarm.input_middle_state;
            input_mot1_alarm.input_confirm_times = 0;
            if (input_mot1_alarm.input_state == 1)
            {
                input_mot1_alarm.input_trig_mode = INPUT_TRIG_UP;
                input_mot1_alarm.level = HIGH_LEVEL;
            }
            else
            {
                input_mot1_alarm.input_trig_mode = INPUT_TRIG_DOWN;
                input_mot1_alarm.level = LOW_LEVEL;
            }
        }
    }
    else
    {
        input_mot1_alarm.input_middle_state = input_mot1_alarm.input_state;
        input_mot1_alarm.input_confirm_times = 0;
    }

    // handle MOT2_ALARM single
    if (input_mot2_alarm.input_state != MOT2_ALARM_IN
        && input_mot2_alarm.input_confirm_times == 0)
    {
        input_mot2_alarm.input_middle_state = MOT2_ALARM_IN;
    }
    if (input_mot2_alarm.input_middle_state == MOT2_ALARM_IN
        && input_mot2_alarm.input_middle_state != input_mot2_alarm.input_state)
    {
        input_mot2_alarm.input_confirm_times++;
        if (input_mot2_alarm.input_confirm_times > 100)
        {
            input_mot2_alarm.input_state = input_mot2_alarm.input_middle_state;
            input_mot2_alarm.input_confirm_times = 0;
            if (input_mot2_alarm.input_state == 1)
            {
                input_mot2_alarm.input_trig_mode = INPUT_TRIG_UP;
                input_mot2_alarm.level = HIGH_LEVEL;
            }
            else
            {
                input_mot2_alarm.input_trig_mode = INPUT_TRIG_DOWN;
                input_mot2_alarm.level = LOW_LEVEL;
            }
        }
    }
    else
    {
        input_mot2_alarm.input_middle_state = input_mot2_alarm.input_state;
        input_mot2_alarm.input_confirm_times = 0;
    }
}

//设置Led闪烁模式
void gpio_Led_Set_Mode(u8 led, u8 mode)
{
    if (led == LED_STATUS)
    {
        switch (mode)
        {
        case LED_OFF:
            LED_STATUS_OFF;
            led_status_blk_mode = LED_OFF;
            break;
        case LED_ON:
            LED_STATUS_ON;
            led_status_blk_mode = LED_ON;
            break;
        case LED_B:
            led_status_blk_mode = LED_B;
            break;
        case LED_BL:
            led_status_blk_mode = LED_BL;
            break;
        default:
            break;
        }
    }
    else if (led == LED_MOT1)
    {
        switch (mode)
        {
        case LED_OFF:
            LED_MOT1_OFF;
            led_mot1_blk_mode = LED_OFF;
            break;
        case LED_ON:
            LED_MOT1_ON;
            led_mot1_blk_mode = LED_ON;
            break;
        case LED_B:
            led_mot1_blk_mode = LED_B;
            break;
        case LED_BL:
            led_mot1_blk_mode = LED_BL;
            break;
        default:
            break;
        }
    }
    else if (led == LED_MOT2)
    {
        switch (mode)
        {
        case LED_OFF:
            LED_MOT2_OFF;
            led_mot2_blk_mode = LED_OFF;
            break;
        case LED_ON:
            LED_MOT2_ON;
            led_mot2_blk_mode = LED_ON;
            break;
        case LED_B:
            led_mot2_blk_mode = LED_B;
            break;
        case LED_BL:
            led_mot2_blk_mode = LED_BL;
            break;
        default:
            break;
        }
    }
}
//Led闪烁控制
void gpio_Led_Blink_Handle(void)
{
    led_blink_cnt++;

    if (led_blink_cnt < 500)
    {
        if ((led_status_blk_mode == LED_B) || (led_status_blk_mode == LED_BL))
        {
            LED_STATUS_ON;
        }
        if ((led_mot1_blk_mode == LED_B) || (led_mot1_blk_mode == LED_BL))
        {
            LED_MOT1_ON;
        }
        if ((led_mot2_blk_mode == LED_B) || (led_mot2_blk_mode == LED_BL))
        {
            LED_MOT2_ON;
        }
    }
    else if (led_blink_cnt >= 500 && led_blink_cnt < 1000)
    {
        if ((led_status_blk_mode == LED_B) || (led_status_blk_mode == LED_BL))
        {
            LED_STATUS_OFF;
        }
        if ((led_mot1_blk_mode == LED_B) || (led_mot1_blk_mode == LED_BL))
        {
            LED_MOT1_OFF;
        }
        if ((led_mot2_blk_mode == LED_B) || (led_mot2_blk_mode == LED_BL))
        {
            LED_MOT2_OFF;
        }
    }
    else if (led_blink_cnt >= 1000 && led_blink_cnt < 1500)
    {
        if (led_status_blk_mode == LED_B)
        {
            LED_STATUS_ON;
        }
        if (led_mot1_blk_mode == LED_B)
        {
            LED_MOT1_ON;
        }
        if (led_mot2_blk_mode == LED_B)
        {
            LED_MOT2_ON;
        }
    }
    else if (led_blink_cnt >= 1500 && led_blink_cnt < 2000)
    {
        if (led_status_blk_mode == LED_B)
        {
            LED_STATUS_OFF;
        }
        if (led_mot1_blk_mode == LED_B)
        {
            LED_MOT1_OFF;
        }
        if (led_mot2_blk_mode == LED_B)
        {
            LED_MOT2_OFF;
        }
    }
    if (led_blink_cnt >= 2000)
    {
        led_blink_cnt = 0;
    }
}