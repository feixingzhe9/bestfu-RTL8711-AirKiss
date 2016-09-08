/*******************************************************************
文	件：Fault.h
说	明：异常处理头文件
编	译：Keil uVision4 V4.54.0.0
版	本：v1.0
编	写：xjx
日	期：2014-08-09
********************************************************************/
#ifndef __FAULT_H_
#define __FAULT_H_
#include "freertos_service.h"

#define WATER_LEVEL_LOW_FAULT           (0)
#define TEMP_TOO_LOW_FAULT              (1)
#define TEMP_TOO_HIGH_FAULT             (2)
#define HUMIDITY_TOO_LOW_FAULT          (3)
#define HUMIDITY_TOO_HIGH_FAULT         (4)


extern void upload_fault_state(u32 fault);

#endif



/**************************Copyright BestFu 2014-05-14*************************/