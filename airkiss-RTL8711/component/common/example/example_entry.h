#ifndef __EXAMPLE_ENTRY_H__
#define __EXAMPLE_ENTRY_H__
#include "water.h"

void example_entry(void);
void pre_example_entry(void);



void TcpClientHandler_test(void *param);
void TcpServerHandler_test(void *param);
void Udpserverhandler_test(void *param);

void TimeDelayTest(void *param);


void task_get_temp_and_humidity(void *param);
void task_get_water_level(void *param);


void cloud_link_task(void *param);
void example_EEPROM_test(void *param);
void Example_RTC_test(void *param);
void Example_BL5372_test(void *param);
void ExampleWaterTest(void *param);
void example_watering_test(void *param);

void manual_watering(struct watering_t * watering_param);
void manual_watering_task(void *param);
void auto_watering_task(void *param);

void task_fault_detection(void *param);


void StartModeAP(void *param);

void airkiss_test_task(void *param);

void connect_to_ap(void *param);


#endif //#ifndef __EXAMPLE_ENTRY_H__
