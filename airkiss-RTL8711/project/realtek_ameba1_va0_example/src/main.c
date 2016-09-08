
 //original

#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include <example_entry.h>
#include "wifi_constants.h"

#include "semphr.h"

#include "BL5372_IIC.h"


#include "fifo.h"
#include "device.h"


#include "wifi_structures.h"
#include "airkiss.h"





extern void console_init(void);
extern xTaskHandle g_client_task;  
extern xTaskHandle g_server_task;

rtw_mode_t gWifiWorkStatus = RTW_MODE_AP;
int gWifiConnectStatus = 0;//WiFi连接情况： 0：未连接到路由器       1：连接到路由器，但未连接到云    2：连接到云
unsigned char gDevIP[4] = {0};
unsigned char gDevGW[4] = {0};
unsigned char gDevMac[6] = {0};




xTaskHandle g_TimeDelayTestTask = NULL;
xTaskHandle g_task_get_temp_and_humidity = NULL;
xTaskHandle g_WaterLevelTestTask = NULL;

xTaskHandle g_auto_watering_task = NULL;
xTaskHandle g_get_temp_humidity_task = NULL;

//xSemaphoreHandle sht20_handle = NULL;

struct watering_t watering_param;            
struct watering_t *p_watering_param = &watering_param;


extern xTaskHandle manual_watering_handle;



rtw_network_info_t wifi_ap_info;




#if   1


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
        uint16_t shtc1_id;
        
        
         
        p_watering_param->watering_period = 100;
        p_watering_param->watering_interval = 100;
        p_watering_param->pwm_period = 1.0/2000;
        p_watering_param->pwm_pulsewidth = p_watering_param->pwm_period/2;
        p_watering_param->cnt = 8;
        
        printf("\n enter main function\n");


        printf("\n Version 1.25\n");
        

        
        

        // Initial a one-shout timer
        //gtimer_start_one_shout(&my_timer2, 500000, (void*)timer2_timeout_handler, NULL);
    
    
    
    
    
        
        
        
        //sht20_handle = xSemaphoreCreateMutex();//创建SHT20 温湿度 的信号量
        
        
        
                  
#if 1         
	if ( rtl_cryptoEngine_init() != 0 ) {
		DiagPrintf("crypto engine init failed\r\n");
	}
        
        
        
	/* Initialize log uart and at command service */
	console_init();	
#endif       
        
        
#if 0
	/* pre-processor of application example */
	pre_example_entry();
#endif

        
        
        
#if 0
	/* wlan intialization */
#if defined(CONFIG_WIFI_NORMAL) && defined(CONFIG_NETWORK)
	wlan_network();
#endif
#endif

#if 0
	/* Execute application example */
	example_entry();
#endif       

#define BSD_STACK_SIZE_TEST		2048        
#if 1    //xjx-test 
        
#if 0           //作为AP
        
        if(xTaskCreate(StartModeAP, "tcp_client", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 20 + PRIORITIE_OFFSET,NULL) != pdPASS)
		printf("\n\rTCP ERROR: Create StartModeAP task failed.");
        
#endif
        
#if 1           //AirKiss
        
        if(xTaskCreate(airkiss_test_task, "airkiss_test_task", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 20 + PRIORITIE_OFFSET,NULL) != pdPASS)
		printf("\n\rTCP ERROR: Create airkiss_test_task task failed.");
        
#endif        
            
        
#if 0          //作为Client，主要
        printf("\n creating TcpClientHandler_test task ");
        if(xTaskCreate(TcpClientHandler_test, "tcp_client", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 20 + PRIORITIE_OFFSET, &g_client_task) != pdPASS)
		printf("\n\rTCP ERROR: Create tcp client task failed.");
#endif 
        
#if 0           //作为Server
        if(xTaskCreate(TcpServerHandler_test, "tcp_server", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 1 + PRIORITIE_OFFSET, &g_server_task) != pdPASS)
			printf("\n\rTCP ERROR: Create tcp server task failed.");
#endif 
        
#if 0           //UDP模式 ，接收SSID和Password    
        if(xTaskCreate(Udpserverhandler_test, "Udpserverhandler_test", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 1 + PRIORITIE_OFFSET, &g_client_task) != pdPASS)
		printf("\n\rTCP ERROR: Create udp server task failed.");   
#endif   
        
        
#endif
        
#if  0           //iic获取与设置时间测试
        if(xTaskCreate(Example_BL5372_test, "Example_BL5372_test", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 2 + PRIORITIE_OFFSET, &g_TimeDelayTestTask) != pdPASS)
		printf("\n\rxTaskCreate (Example_BL5372_test task failed.) \n");

#endif        
        
#if 0           //shtc20
        if(xTaskCreate(task_get_temp_and_humidity, ((const char*)"task_get_temp_and_humidity"), 512, NULL, tskIDLE_PRIORITY + 3, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate(example_shtc20_test) failed", __FUNCTION__);
#endif

#if 0           //EEPROM
        if(xTaskCreate(example_EEPROM_test, ((const char*)"example_EEPROM_test"), 512, NULL, tskIDLE_PRIORITY + 4, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate(example_EEPROM_test) failed", __FUNCTION__);
#endif   
      
#if 0
        if(xTaskCreate(task_get_water_level, ((const char*)"task_get_water_level"), 512, NULL, tskIDLE_PRIORITY + 5, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate(task_get_water_level) failed", __FUNCTION__);
#endif
        
#if 0           
            printf("\n starting create manual_watering_task\n");
            if(xTaskCreate(manual_watering_task, ((const char*)"manual_watering_task"), 512, (void*)p_watering_param, tskIDLE_PRIORITY + 5, manual_watering_handle) != pdPASS)
                printf("\n\r%s xTaskCreate(manual_watering_task) failed", __FUNCTION__);
            printf("\n create manual_watering_task is done \n");  

#endif 
            
#if 0     
            
            //printf("\n starting create auto_watering_task\n");
            if(xTaskCreate(auto_watering_task, ((const char*)"auto_watering_task"), 512, (void*)p_watering_param, tskIDLE_PRIORITY + 6, NULL) != pdPASS)
                printf("\n\r%s xTaskCreate(auto_watering_task) failed", __FUNCTION__);
           // printf("\n create auto_watering_task is done \n");  

#endif 
            
#if 0     
            
            //printf("\n starting create fault_detection_task\n");
            if(xTaskCreate(task_fault_detection, ((const char*)"task_fault_detection"), 512, NULL, tskIDLE_PRIORITY + 7, NULL) != pdPASS)
                printf("\n\r%s xTaskCreate(fault_detection_task) failed", __FUNCTION__);
            //printf("\n create fault_detection_task is done \n");  

#endif 
    	/*Enable Schedule, Start Kernel*/
#if defined(CONFIG_KERNEL) && !TASK_SCHEDULER_DISABLED
	#ifdef PLATFORM_FREERTOS
	vTaskStartScheduler();
	#endif
#else
	RtlConsolTaskRom(NULL);
#endif
        
       
}
#endif





#if 0
    
    /*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */



void timer2_timeout_handler(uint32_t id)
{
    time2_expired = 1;
}

void main(void)
{
    // Init LED control pin
    gpio_init(&gpio_led1, GPIO_LED_PIN1);
    gpio_dir(&gpio_led1, PIN_OUTPUT);    // Direction: Output
    gpio_mode(&gpio_led1, PullNone);     // No pull

    gpio_init(&gpio_led2, GPIO_LED_PIN2);
    gpio_dir(&gpio_led2, PIN_OUTPUT);    // Direction: Output
    gpio_mode(&gpio_led2, PullNone);     // No pull

    // Initial a periodical timer
    
    
    while(1){
        if (time2_expired) {
            gpio_write(&gpio_led2, !gpio_read(&gpio_led2));
            time2_expired = 0;
            gtimer_start_one_shout(&my_timer2, 500000, (void*)timer2_timeout_handler, NULL);
        }
    }
}



#endif





