/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32_eth.h"
#include "netconf.h"
#include "main.h"
#include "udpclient.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;
u16 sec_reg = 1000;
u16 sec_flag = 0;
u8  local_station;//本机站号
u8  onems_flag = 0;

u8  mac_addr[6] = {0};

/* Private function prototypes -----------------------------------------------*/
void System_Periodic_Handle(void);


void main_init_msg(void);
void main_onems_process(void);

void read_mac_addrs(void)
{
    u32 Mac_Code;
    u32 CpuID[3];
    
    //获取CPU唯一ID
    CpuID[0] = *(u32 *)(0x1FFFF7E8);
    CpuID[1] = *(u32 *)(0x1FFFF7EC);
    CpuID[2] = *(u32 *)(0x1FFFF7F0);
    
    Mac_Code = (CpuID[0] >> 1) + (CpuID[1] >> 2) + (CpuID[2] >> 3);
    
    mac_addr[0] = 0x00;
    mac_addr[1] = 0x02;
    mac_addr[2] = (u8) ((Mac_Code >> 24) & 0xFF);
    mac_addr[3] = (u8) ((Mac_Code >> 16) & 0xFF);
    mac_addr[4] = (u8) ((Mac_Code >> 8) & 0xFF);
    mac_addr[5] = (u8) ( Mac_Code & 0xFF);
}

void scan_local_station(void)
{
    u8 tmp, i;
    u8 dip_value;

    tmp = 0;
    for(i = 0; i < 20; i++)
    {
        if( DIP1_STATE )
        {
            tmp |= 0x01;
        }
        if( DIP2_STATE )
        {
            tmp |= 0x02;
        }
        if( DIP3_STATE )
        {
            tmp |= 0x04;
        }
        if( DIP4_STATE )
        {
            tmp |= 0x08;
        }
        if( DIP5_STATE )
        {
            tmp |= 0x10;
        }
        if( DIP6_STATE )
        {
            tmp |= 0x20;
        }
        if( DIP7_STATE )
        {
            tmp |= 0x40;
        }
        if( DIP8_STATE )
        {
            tmp |= 0x80;
        }
        if( dip_value != tmp )
        {
            dip_value = tmp;
            i = 0;
        }
    }
    local_station = (~dip_value) & 0xFF;
}
//1s
void sec_process(void)
{
    if( sec_flag == 1 )
    {
        sec_flag = 0;

        if( LED_STATE )
        {
            LED_ON;
        }
        else
        {
            LED_OFF;
        }
    }
}
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
    System_Setup();
    main_init_msg();

    /* Infinite loop */
    while (1)
    {
        if(EthInitStatus == 0 )
        {
            Ethernet_Configuration();
            if( EthInitStatus != 0 )
            {
                LwIP_Init();
            }
        }
        else
        {
            LwIP_Periodic_Handle(LocalTime);
            udp_client_process();
            send_message_to_server();
        }
        sec_process();
        can_send_frame_process();
        main_onems_process();
    }
}

/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of 10ms periods to wait for.
  * @retval None
  */
void Delay(uint32_t nCount)
{
    /* Capture the current local time */
    timingdelay = LocalTime + nCount;

    /* wait until the desired delay finish */
    while(timingdelay > LocalTime)
    {
    }
}

/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
void Time_Update(void)
{
    LocalTime += SYSTEMTICK_PERIOD_MS;
    onems_flag = VALUE;
    upload_state_cnt++;

    if( sec_reg != 0 )
    {
        sec_reg--;
        if( sec_reg == 0 )
        {
            sec_reg = 1000;
            sec_flag = 1;
        }
    }
}

void main_init_msg(void)
{
    //fun.h
    fun_struct_init();
    InitSendMsgQueue();
    InitCanSendQueue();
    scan_local_station();
    read_mac_addrs();

    //data.h
    data_msg_init();
    data_pkg_list_init();
    DatainitpkgQueue();
    //flash.h
    flash_read_zone_conf();
    //logic.h
    logicModuleNodeInit();
}

void main_onems_process(void)
{
    if (onems_flag == VALUE) {
        onems_flag = INVALUE;

        //logic_ctrl.c
        logic_pkg_trans_process();
        logicDealWithModuleNodeQueue();

        //fun.h
        upload_moudle_state();
        upload_pkg_result_queue();
        //flash.h
        flash_upload();


        if (upload_state_cnt > 300) {
            upload_state_cnt = 0;
            vcanbus_send_askmodulestate_cmd();
            upload_state_flag = VALUE;
        }
    }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {}
}
#endif


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
