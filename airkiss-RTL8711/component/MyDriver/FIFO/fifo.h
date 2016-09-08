/***************************Copyright BestFu 2014-05-14*************************
文   件：  Fifo.h
说   明：  通用队列处理头文件
编   译：  Keil uVision4 V4.54.0.0
版   本：  v1.0
编   写：  Unarty
日   期：  2013-09-03
修  改:     暂无
*******************************************************************************/
#ifndef __FIFO_H
#define __FIFO_H
#include "freertos_service.h"


/***********************************宏定义************************************/
/*容错宏*/
#define ERRR(conf, ret)      do              \
                            {                \
                                if (conf)    \
                                {            \
                                    ret;     \
                                }            \
                            } while(0) 
                            
#define MIN(a, b)           ((a) > (b) ? (b) : (a))

/*******************************定义数据结构**********************************/
struct fifo_data
{
    u8   *data;
    u32   size;
    u32   front;
    u32   rear;
};

/*********************************函数声明*************************************/
u8 fifo_Init(struct fifo_data *head, u8 *buf, u32 len); //队列初始化
void fifo_Rst(struct fifo_data *head);                        //清空队列
u32 fifo_validSize(struct fifo_data *head);     //数据可用空间大小
u8 fifo_empty(struct fifo_data *head);        //队空判断
u8 fifo_puts(struct fifo_data *head, u8 *data, u32 len);
u8 fifo_gets(struct fifo_data *head, u8 *data, u32 len);
u8 fifo_putc(struct fifo_data *head, u8 data);
u8 fifo_getc(struct fifo_data *head, u8 *data);
u32 fifo_find(struct fifo_data *head, const u8 *data, u32 len);
u8 fifo_cmp(const struct fifo_data *fifo, u32 seat, const u8 *cmp, u32 cmpsize);
                            
#endif //queue.h end
/**************************Copyright BestFu 2014-05-14*************************/
