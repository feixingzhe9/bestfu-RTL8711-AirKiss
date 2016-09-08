/***************************Copyright BestFu 2014-05-14*************************
文	件：	Fault.c
说	明：	异常处理：包括水位异常，温湿度异常，网络异常等
编	译：	IAR
版	本：	v1.0
编	写：	xjx
日	期：	
修　改：	暂无
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "Fault.h"
#include "Water.h"

#include "SHT20.h"
#include "Kalman_Filter.h"



void upload_fault_state(u32 fault)
{    
    if(fault &(1<<WATER_LEVEL_LOW_FAULT))//低水位
    {
        
    }
    if(fault &(1<<TEMP_TOO_LOW_FAULT))//温度过低
    {
        
    }
    if(fault &(1<<TEMP_TOO_HIGH_FAULT))//温度过高
    {
        
    }
    if(fault &(1<<HUMIDITY_TOO_LOW_FAULT))//湿度过低
    {
        
    }
    if(fault &(1<<HUMIDITY_TOO_HIGH_FAULT))//湿度过高
    {
        
    }   
}





void task_fault_detection(void *param)
{
    u32 fault = 0;

    
    int temp_tmp = 0;
    u8 humidity_tmp = 0;
    
    u8 temp_min;        //温度下限
    u8 temp_max;        //温度上限
    u8 humidity_min;    //湿度下限
    u8 humidity_max;    //湿度上限
    
    u8 cnt = 0;
    u8 water_level_tmp = 0;
    
    WaterGPIOPortInit();
    vTaskDelay(100);

    
    

    
    for(;;)
    {
      
       

        
        get_temp_and_humidity(&temp_tmp,&humidity_tmp);
        
        taskENTER_CRITICAL();
        {//操作全局变量
            humidity_tmp = sht20.humidity;
            temp_tmp = sht20.temp;
            water_level_tmp = g_water_level;
            temp_min = p_temp_humidity_set->temp_min;
            temp_max = p_temp_humidity_set->temp_max;
            humidity_min = p_temp_humidity_set->humidity_min;
            humidity_max = p_temp_humidity_set->temp_max;           
        }
        taskEXIT_CRITICAL();
        
        {//打印参数
            //printf("temperature is %d, humidity is %d \n",temp_tmp,humidity_tmp);
            
            if(water_level_tmp == NO_WATER)
            {
                printf("no water \n");
            
            }
            else
            {
                printf("water full \n");
            }
        }
        
        
        
        if(NO_WATER == water_level_tmp)
        {
            //printf("no water \n");
            fault |= 1 << WATER_LEVEL_LOW_FAULT;
        }
        
        if(temp_tmp < temp_min)//温度过低
        {
            //printf("temperature is too low , %d \n",temp_tmp);
            fault |= 1 << TEMP_TOO_LOW_FAULT;
            
        }
        if(temp_tmp > temp_max)//温度过高
        {
            //printf("temperature is too high , %d \n",temp_tmp);
            fault |= 1 << TEMP_TOO_HIGH_FAULT;
            
        }
        if(humidity_tmp < humidity_min)//湿度过低
        {
            //printf("humidity is too low , %d \n",humidity_tmp);
            fault |= 1 << HUMIDITY_TOO_LOW_FAULT;
            
        }
        if(humidity_tmp > humidity_max)//湿度过高
        {
            //printf("humidity is too high , %d \n",humidity_tmp);
            fault |= 1 << HUMIDITY_TOO_HIGH_FAULT;            
        }
        
        
        if(fault)
        {
            upload_fault_state(fault);
            fault = 0;
        }
        
        vTaskDelay(950);
        
        
    }
}
















/**************************Copyright BestFu 2014-05-14*************************/