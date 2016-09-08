

#if 1
/****************************************************************/


#include "BL5372_IIC.h"
#include "BL5372.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpio_api.h"   
#include "PinNames.h"



#define BL5372_TIME_SPACE         sizeof(struct time_t)

extern void IIC_InitPort(void);
struct time_t current_time;
struct time_t *p_current_time = &current_time;
void Example_BL5372_test(void *param)
{

    struct time_t time_set = {58,20,16,5,26,8,16};//秒分时周日月年
    u8 i;
    
    IIC_InitPort();
    vTaskDelay(100);
//    API_Write_Nbyte_I2C(BL5372_DEVICE_ADDR,0,(u8*)&time_set,BL5372_TIME_SPACE);
    while(1)
    {
    
        taskENTER_CRITICAL();
        API_Read_Nbyte_I2C(BL5372_DEVICE_ADDR, 0,p_current_time, BL5372_TIME_SPACE);
        taskEXIT_CRITICAL();
        
        printf("20%d- %d- %d  xingqi %d   %d : %d : %d \n",p_current_time->year,p_current_time->month,p_current_time->day,\
                                                           p_current_time->week,p_current_time->hour,p_current_time->min,p_current_time->sec);

        vTaskDelay(990);

    }
    
}

#endif