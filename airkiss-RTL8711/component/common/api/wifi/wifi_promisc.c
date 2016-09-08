#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "tcpip.h"
#include "wifi/wifi_conf.h"

#ifndef CONFIG_WLAN
#define CONFIG_WLAN 1
#endif

#if CONFIG_WLAN
#include <platform/platform_stdlib.h>


#include "airkiss.h"

struct eth_frame {
	struct eth_frame *prev;
	struct eth_frame *next;
	unsigned char da[6];
	unsigned char sa[6];
	unsigned int len;
	unsigned char type;
	signed char rssi;
};

#if CONFIG_INIC_CMD_RSP
#if defined(__IAR_SYSTEMS_ICC__)
#pragma pack(1)
#endif
struct inic_eth_frame {
	unsigned char da[6];
	unsigned char sa[6];
	unsigned int len;
	unsigned char type;
};
#if defined(__IAR_SYSTEMS_ICC__)
#pragma pack()
#endif

static struct inic_eth_frame *inic_frame, *inic_frame_tail = NULL;
static int inic_frame_cnt = 0;
#define MAX_INIC_FRAME_NUM 50 //maximum packets for each channel
extern void inic_c2h_msg(const char *atcmd, char status, char *msg, u16 msg_len);
#endif

struct eth_buffer {
	struct eth_frame *head;
	struct eth_frame *tail;
};

static struct eth_buffer eth_buffer;

#ifdef CONFIG_PROMISC
#define MAX_PACKET_FILTER_INFO 5
#define FILTER_ID_INIT_VALUE 10
rtw_packet_filter_info_t paff_array[MAX_PACKET_FILTER_INFO]={0, 0, 0, 0, 0};
static u8 packet_filter_enable_num = 0;

void promisc_init_packet_filter()
{
	int i = 0;
	for(i=0; i<MAX_PACKET_FILTER_INFO; i++){
		paff_array[i].filter_id = FILTER_ID_INIT_VALUE;
		paff_array[i].enable = 0;
		paff_array[i].patt.mask_size = 0;
		paff_array[i].rule = RTW_POSITIVE_MATCHING;
		paff_array[i].patt.mask = NULL;
		paff_array[i].patt.pattern = NULL;
	}
	packet_filter_enable_num = 0;
}

int promisc_add_packet_filter(u8 filter_id, rtw_packet_filter_pattern_t *patt, rtw_packet_filter_rule_e rule)
{
	int i = 0;
	while(i < MAX_PACKET_FILTER_INFO){
		if(paff_array[i].filter_id == FILTER_ID_INIT_VALUE){
			break;
		}
		i++;	
	}

	if(i == MAX_PACKET_FILTER_INFO)
		return -1;

	paff_array[i].filter_id = filter_id;

	paff_array[i].patt.offset= patt->offset;
	paff_array[i].patt.mask_size = patt->mask_size;
	paff_array[i].patt.mask = pvPortMalloc(patt->mask_size);
	memcpy(paff_array[i].patt.mask, patt->mask, patt->mask_size);
	paff_array[i].patt.pattern= pvPortMalloc(patt->mask_size);
	memcpy(paff_array[i].patt.pattern, patt->pattern, patt->mask_size);

	paff_array[i].rule = rule;
	paff_array[i].enable = 0;

	return 0;
}

int promisc_enable_packet_filter(u8 filter_id)
{
	int i = 0;
	while(i < MAX_PACKET_FILTER_INFO){
		if(paff_array[i].filter_id == filter_id)
			break;
		i++;
	}

	if(i == MAX_PACKET_FILTER_INFO)
		return -1;

	paff_array[i].enable = 1;
	packet_filter_enable_num++;
	return 0;
}

int promisc_disable_packet_filter(u8 filter_id)
{
	int i = 0;
	while(i < MAX_PACKET_FILTER_INFO){
		if(paff_array[i].filter_id == filter_id)
			break;
		i++;
	}

	if(i == MAX_PACKET_FILTER_INFO)
		return -1;

	paff_array[i].enable = 0;
	packet_filter_enable_num--;
	return 0;
}

int promisc_remove_packet_filter(u8 filter_id)
{
	int i = 0;
	while(i < MAX_PACKET_FILTER_INFO){
		if(paff_array[i].filter_id == filter_id)
			break;
		i++;
	}

	if(i == MAX_PACKET_FILTER_INFO)
		return -1;

	paff_array[i].filter_id = FILTER_ID_INIT_VALUE;
	paff_array[i].enable = 0;
	paff_array[i].patt.mask_size = 0;
	paff_array[i].rule = 0;
	if(paff_array[i].patt.mask){
		vPortFree(paff_array[i].patt.mask);
		paff_array[i].patt.mask = NULL;
	}
	
	if(paff_array[i].patt.pattern){
		vPortFree(paff_array[i].patt.pattern);
		paff_array[i].patt.pattern = NULL;
	}
	return 0;
}
#endif

/*	Make callback simple to prevent latency to wlan rx when promiscuous mode */
static void promisc_callback(unsigned char *buf, unsigned int len, void* userdata)
{
	struct eth_frame *frame = (struct eth_frame *) pvPortMalloc(sizeof(struct eth_frame));
	
	if(frame) {
		frame->prev = NULL;
		frame->next = NULL;
		memcpy(frame->da, buf, 6);
		memcpy(frame->sa, buf+6, 6);
		frame->len = len;
		frame->rssi = ((ieee80211_frame_info_t *)userdata)->rssi;
		taskENTER_CRITICAL();

		if(eth_buffer.tail) {
			eth_buffer.tail->next = frame;
			frame->prev = eth_buffer.tail;
			eth_buffer.tail = frame;
		}
		else {
			eth_buffer.head = frame;
			eth_buffer.tail = frame;
		}

		taskEXIT_CRITICAL();
	}
}

struct eth_frame* retrieve_frame(void)
{
	struct eth_frame *frame = NULL;

	taskENTER_CRITICAL();

	if(eth_buffer.head) {
		frame = eth_buffer.head;

		if(eth_buffer.head->next) {
			eth_buffer.head = eth_buffer.head->next;
			eth_buffer.head->prev = NULL;
		}
		else {
			eth_buffer.head = NULL;
			eth_buffer.tail = NULL;
		}
	}

	taskEXIT_CRITICAL();

	return frame;
}

static void promisc_test(int duration, unsigned char len_used)
{
	int ch;
	unsigned int start_time;
	struct eth_frame *frame;
	eth_buffer.head = NULL;
	eth_buffer.tail = NULL;

	wifi_enter_promisc_mode();
	wifi_set_promisc(RTW_PROMISC_ENABLE, promisc_callback, len_used);

	for(ch = 1; ch <= 13; ch ++) {
		if(wifi_set_channel(ch) == 0)
			printf("\n\n\rSwitch to channel(%d)", ch);

		start_time = xTaskGetTickCount();

		while(1) {
			unsigned int current_time = xTaskGetTickCount();

			if((current_time - start_time) < (duration * configTICK_RATE_HZ)) {
				frame = retrieve_frame();

				if(frame) {
					int i;
					printf("\n\rDA:");
					for(i = 0; i < 6; i ++)
						printf(" %02x", frame->da[i]);
					printf(", SA:");
					for(i = 0; i < 6; i ++)
						printf(" %02x", frame->sa[i]);
					printf(", len=%d", frame->len);
					printf(", RSSI=%d", frame->rssi);
#if CONFIG_INIC_CMD_RSP
					if(inic_frame_tail){
						if(inic_frame_cnt < MAX_INIC_FRAME_NUM){
							memcpy(inic_frame_tail->da, frame->da, 6);
							memcpy(inic_frame_tail->sa, frame->sa, 6);
							inic_frame_tail->len = frame->len;
							inic_frame_tail++;
							inic_frame_cnt++;
						}
					}
#endif	
					vPortFree((void *) frame);
				}
				else
					vTaskDelay(1);	//delay 1 tick
			}
			else
				break;	
		}
#if CONFIG_INIC_CMD_RSP
		if(inic_frame){
			inic_c2h_msg("ATWM", RTW_SUCCESS, (char *)inic_frame, sizeof(struct inic_eth_frame)*inic_frame_cnt);
			memset(inic_frame, '\0', sizeof(struct inic_eth_frame)*MAX_INIC_FRAME_NUM);
				inic_frame_tail = inic_frame;
				inic_frame_cnt = 0;
			rtw_msleep_os(10);
		}
#endif
	}

	wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 0);

	while((frame = retrieve_frame()) != NULL)
		vPortFree((void *) frame);
}

static void promisc_callback_all(unsigned char *buf, unsigned int len, void* userdata)
{
	struct eth_frame *frame = (struct eth_frame *) pvPortMalloc(sizeof(struct eth_frame));
	
	if(frame) {
		frame->prev = NULL;
		frame->next = NULL;
		memcpy(frame->da, buf+4, 6);
		memcpy(frame->sa, buf+10, 6);
		frame->len = len;
		frame->type = *buf;
		frame->rssi = ((ieee80211_frame_info_t *)userdata)->rssi;

		taskENTER_CRITICAL();

		if(eth_buffer.tail) {
			eth_buffer.tail->next = frame;
			frame->prev = eth_buffer.tail;
			eth_buffer.tail = frame;
		}
		else {
			eth_buffer.head = frame;
			eth_buffer.tail = frame;
		}

		taskEXIT_CRITICAL();
	}
}
static void promisc_test_all(int duration, unsigned char len_used)
{
	int ch;
	unsigned int start_time;
	struct eth_frame *frame;
	eth_buffer.head = NULL;
	eth_buffer.tail = NULL;

	wifi_enter_promisc_mode();
	wifi_set_promisc(RTW_PROMISC_ENABLE_2, promisc_callback_all, len_used);

	for(ch = 1; ch <= 13; ch ++) {
		if(wifi_set_channel(ch) == 0)
			printf("\n\n\rSwitch to channel(%d)", ch);

		start_time = xTaskGetTickCount();

		while(1) {
			unsigned int current_time = xTaskGetTickCount();

			if((current_time - start_time) < (duration * configTICK_RATE_HZ)) {
				frame = retrieve_frame();

				if(frame) {
					int i;
					printf("\n\rTYPE: 0x%x, ", frame->type);
					printf("DA:");
					for(i = 0; i < 6; i ++)
						printf(" %02x", frame->da[i]);
					printf(", SA:");
					for(i = 0; i < 6; i ++)
						printf(" %02x", frame->sa[i]);
					printf(", len=%d", frame->len);
					printf(", RSSI=%d", frame->rssi);
#if CONFIG_INIC_CMD_RSP
					if(inic_frame_tail){
						if(inic_frame_cnt < MAX_INIC_FRAME_NUM){
							memcpy(inic_frame_tail->da, frame->da, 6);
							memcpy(inic_frame_tail->sa, frame->sa, 6);
							inic_frame_tail->len = frame->len;
							inic_frame_tail->type = frame->type;
							inic_frame_tail++;
							inic_frame_cnt++;
						}
					}
#endif	
					vPortFree((void *) frame);
				}
				else
					vTaskDelay(1);	//delay 1 tick
			}
			else
				break;	
		}
#if CONFIG_INIC_CMD_RSP
		if(inic_frame){
			inic_c2h_msg("ATWM", RTW_SUCCESS, (char *)inic_frame, sizeof(struct inic_eth_frame)*inic_frame_cnt);
			memset(inic_frame, '\0', sizeof(struct inic_eth_frame)*MAX_INIC_FRAME_NUM);
				inic_frame_tail = inic_frame;
				inic_frame_cnt = 0;
			rtw_msleep_os(10);
		}
#endif
	}

	wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 0);

	while((frame = retrieve_frame()) != NULL)
		vPortFree((void *) frame);
}

void cmd_promisc(int argc, char **argv)
{
	int duration;
#if CONFIG_INIC_CMD_RSP
	inic_frame_tail = inic_frame = pvPortMalloc(sizeof(struct inic_eth_frame)*MAX_INIC_FRAME_NUM);
	if(inic_frame == NULL){
		inic_c2h_msg("ATWM", RTW_BUFFER_UNAVAILABLE_TEMPORARY, NULL, 0);
		return;
	}
#endif
	#ifdef CONFIG_PROMISC
	wifi_init_packet_filter();
	#endif
	if((argc == 2) && ((duration = atoi(argv[1])) > 0))
		//promisc_test(duration, 0);
		promisc_test_all(duration, 0);
	else if((argc == 3) && ((duration = atoi(argv[1])) > 0) && (strcmp(argv[2], "with_len") == 0))
		promisc_test(duration, 1);
	else
		printf("\n\rUsage: %s DURATION_SECONDS [with_len]", argv[0]);
#if CONFIG_INIC_CMD_RSP
	if(inic_frame)
		vPortFree(inic_frame);
	inic_frame_tail = NULL;
	inic_frame_cnt = 0;
#endif
}






airkiss_context_t ak_context;
extern void creat_task__connect_to_ap(void);

airkiss_config_t ak_cfg = 
{

        (airkiss_memset_fn)&memset,
	(airkiss_memcpy_fn)&memcpy,
	(airkiss_memcmp_fn)&memcmp,
	(airkiss_printf_fn)&printf
};

airkiss_result_t airkiss_result;
extern u8 wifi_change_channel;
extern u8 wifi_channel;
extern rtw_network_info_t wifi_ap_info;

extern void connect_ap(void *arg);

void promisc_test_callback(u8 *buf ,u16 len)
{
    u8 ret = 0;
    ret = airkiss_recv(&ak_context, buf, len); 
    
    if(ret == AIRKISS_STATUS_CONTINUE)//解码正常，无需特殊处理，继续调用airkiss_recv()直到解码成功
    {
        //printf("receive is OK \n");
    }
    else if(ret == AIRKISS_STATUS_CHANNEL_LOCKED)// wifi信道已经锁定，上层应该立即停止切换信道 
    {
        wifi_change_channel = FALSE;
        printf("channel is locked \n");
        printf("wifi channel is %d \n",wifi_channel);
    }
    else if(ret == AIRKISS_STATUS_COMPLETE)//解码成功，可以调用airkiss_get_result()取得结果 
    {
        
        u8 ret;
        //u8 buf[256];
        
        ret = airkiss_get_result(&ak_context,&airkiss_result);
        if(ret == 0)
        {
            memcpy(&wifi_ap_info.ssid.val,airkiss_result.ssid,airkiss_result.ssid_length);
            wifi_ap_info.password = (unsigned char *)airkiss_result.pwd;
            wifi_ap_info.ssid.len = airkiss_result.ssid_length;
            wifi_ap_info.password_len = airkiss_result.pwd_length;
            
            printf("ssid is %s, password is %s, ssid length is %d, password length is %d; \n",wifi_ap_info.ssid.val,wifi_ap_info.password,\
                                                                                            wifi_ap_info.ssid.len,wifi_ap_info.password_len);
            
            //printf("ssid is %s, password is %s, ssid length is %d, password length is %d; \n",airkiss_result.ssid,airkiss_result.pwd, \
                                                                                    airkiss_result.ssid_length,airkiss_result.pwd_length);
            wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 0/*len_used*/);
            wifi_change_channel = 3;
            creat_task__connect_to_ap();
        }
    }
    //printf("airkiss result is %d \n",ret);
}


#include "timer_api.h"

#define TIMER_PERIOD            (100*1000)
gtimer_t my_timer0;

u8 wifi_channel = 1;
u8 wifi_change_channel = TRUE;




extern airkiss_context_t ak_context;


void timer0_callback(uint32_t id)
{   
    
    if(wifi_change_channel == TRUE)
    {
        if(wifi_channel < 13)
        {
            wifi_channel++;
        }
        else
        {
            wifi_channel = 1;
        }
        wifi_set_channel(wifi_channel);
        airkiss_change_channel(&ak_context);
    }
    else
    {
        //printf("wifi channel is %d \n",wifi_channel);
    }
    
    if(wifi_change_channel == 3)//完成AirKiss，关闭定时器
    {
        gtimer_stop(&my_timer0);
    }   
}




void airkiss_test_task(void *param)
{
    int ret = 0;
    
    const char key[] = {0,1,2,3,4,5,6,7,8,9,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    
    //vTaskDelay(1000);
        
        
    const char * AirKissVersion = airkiss_version();       
    printf("\n airkiss version is %s \n",AirKissVersion);
    
    ret = airkiss_init(&ak_context,&ak_cfg);
    printf("\n airkiss init result is %d \n",ret);
    
    
//#if AIRKISS_ENABLE_CRYPT

    //ret = airkiss_set_key(&ak_context, key, sizeof(key));
    //printf("set key ret is %d",ret);

//#endif
    
    wifi_disconnect();
    wifi_on(RTW_MODE_STA);
    wifi_enter_promisc_mode();
    wifi_set_promisc(RTW_PROMISC_ENABLE_2, promisc_test_callback, 0/*len_used*/);
    
    gtimer_init(&my_timer0, TIMER0);
    gtimer_start_periodical(&my_timer0, TIMER_PERIOD, (void*)timer0_callback, NULL);
    
    while(1)
    {
        vTaskDelay(50);
    }
}
#endif	//#if CONFIG_WLAN
