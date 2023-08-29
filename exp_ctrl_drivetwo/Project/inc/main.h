/**
******************************************************************************
* @file    main.h
* @author  MCD Application Team
* @version V1.0.0
* @date    11/20/2009
* @brief   This file contains all the functions prototypes for the main.c
*          file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f107.h"
#include "can2_bus.h"
#include "can1_bus.h"
#include "string.h"
#include "flash.h"
#include "gpio.h"
#include "moto.h"


/* Exported function prototypes ----------------------------------------------*/
void Time_Update(void);
void Delay(uint32_t nCount);
u16 Get_Adc(u8 ch);


#define  VALUE              1
#define  INVALUE            0
//#define  NULL              (void*)0

#define	 LED_MOT1_ON		GPIO_ResetBits(GPIOC,GPIO_Pin_2)
#define	 LED_MOT1_OFF		GPIO_SetBits(GPIOC,GPIO_Pin_2)
#define	 LED_MOT2_ON		GPIO_ResetBits(GPIOC,GPIO_Pin_3)
#define	 LED_MOT2_OFF		GPIO_SetBits(GPIOC,GPIO_Pin_3)
#define	 LED_STATUS_ON		GPIO_ResetBits(GPIOA,GPIO_Pin_12)
#define	 LED_STATUS_OFF		GPIO_SetBits(GPIOA,GPIO_Pin_12)

#define  IN1_1_STATE            ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0))&0x1)//一区光电1
#define  IN2_1_STATE            ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1))&0x1)//二区光电1
#define  IN1_2_STATE            ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4))&0x1)//一区光电2
#define  IN2_2_STATE            ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5))&0x1)//二区光电2

#define  X1_IN_STATE            ((GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10))&0x1)//一区通用输入
#define  X2_IN_STATE            ((GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11))&0x1)//二区通用输入
#define  X1_OUT(BitVal)         GPIO_WriteBit(GPIOA,GPIO_Pin_4,BitVal)//一区通用输出
#define  X2_OUT(BitVal)         GPIO_WriteBit(GPIOA,GPIO_Pin_5,BitVal)//二区通用输出

#define  MOT1_ALARM_IN          GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)//无连接默认上拉,无故障输入低,故障输入高12V
#define  MOT2_ALARM_IN          GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)//无连接默认上拉,无故障输入低,故障输入高12V
#define  MOT1_DIR(BitVal)       GPIO_WriteBit(GPIOB,GPIO_Pin_0,BitVal)//默认输出高12V(CW),输出反向逻辑
#define  MOT2_DIR(BitVal)       GPIO_WriteBit(GPIOB,GPIO_Pin_1,BitVal)//默认输出高12V(CW),输出反向逻辑



//#define  MOD_DIP_S1             ((~GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7))&0x1)
//#define  MOD_DIP_S3             ((~GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6))&0x1)
//#define  MOD_DIP_S5             ((~GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12))&0x1)
//#define  MOD_DIP_S7             ((~GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11))&0x1)
//#define  MOD_DIP_S8             ((~GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10))&0x1)
//#define  MOD_DIP_S6             ((~GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9))&0x1)
//#define  MOD_DIP_S4             ((~GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8))&0x1)
//#define  MOD_DIP_S2             ((~GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13))&0x1)

#define  MOD_DIP_S1             ((~GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7))&0x1)
#define  MOD_DIP_S2             ((~GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6))&0x1)
#define  MOD_DIP_S3             ((~GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12))&0x1)
#define  MOD_DIP_S4             ((~GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11))&0x1)
#define  MOD_DIP_S5             ((~GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10))&0x1)
#define  MOD_DIP_S6             ((~GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9))&0x1)
#define  MOD_DIP_S7             ((~GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8))&0x1)
#define  MOD_DIP_S8             ((~GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4))&0x1)

#define  SPEED_DIP_S1           ((GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5))&0x1)
#define  SPEED_DIP_S2           ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11))&0x1)
#define  SPEED_DIP_S3           ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_10))&0x1)
#define  SPEED_DIP_S4           ((GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2))&0x1)

#define  DIR_DIP_S1             ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9))&0x1)
#define  DIR_DIP_S2             ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8))&0x1)
#define  DIR_DIP_S3             ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7))&0x1)
#define  DIR_DIP_S4             ((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6))&0x1)

float Get_Vcc_Volt(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

