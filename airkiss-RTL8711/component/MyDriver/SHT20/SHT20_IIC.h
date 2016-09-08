/*****************************************************************************
	文件： IIC.h
	说明： I2C通信驱动
	编译： Keil uVision4 V4.54.0.0
	版本： v1.0
	编写： Joey
	日期： 2013.12.16    
*****************************************************************************/
#ifndef _IIC_H
#define _IIC_H

//#include "device.h"
#include "gpio_api.h"   // mbed

#define SHT_DEVICE_ADDR     //0x64      //BL5372  ADDR               //0x46    // BH1750  ADDR =0      : 0100 011x

/************************IIC Inteface STM32L151C8T6 I/O***********************/

/*******************************************************************************/

#define SHT_SCL_PIN       PC_5         
#define SHT_SDA_PIN       PC_4//PC_4//




#define SHT_SCL1        gpio_write(&gpio_sht_scl, 1)

#define SHT_SCL0        gpio_write(&gpio_sht_scl, 0)

#define SHT_SDA1        gpio_write(&gpio_sht_sda, 1)

#define SHT_SDA0        gpio_write(&gpio_sht_sda, 0)

#define SHT_SDA_IN()    {gpio_dir(&gpio_sht_sda, PIN_INPUT);gpio_mode(&gpio_sht_sda, PullUp);}
                    
                    
#define SHT_SDA_OUT()   gpio_dir(&gpio_sht_sda, PIN_OUTPUT)
#define SHT_SCL_OUT()   gpio_dir(&gpio_sht_scl, PIN_OUTPUT)
                    


#define SHT_Get_SDA()   gpio_read(&gpio_sht_sda)

#endif


