/***************************Copyright BestFu ***********************************
**  文    件：  IIC.c
**  功    能：  <<驱动层>> 模拟IIC驱动
**  编    译：  Keil uVision5 V5.10
**  版    本：  V2.0
**  编    写：  Seven
**  创建日期：  2014.03.21
**  修改日期：  2014.08.20
**  说    明：  模拟IIC协议
**  V2.0
    >> 所有函数重新优化，删除额外操作
    >> 优化移植接口
*******************************************************************************/
#include "BL5372_IIC.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpio_api.h"   


#include "PinNames.h"
#include "basic_types.h"
#include "diag.h"
#include <osdep_api.h>

#include "i2c_api.h"
#include "pinmap.h"
#include "ex_api.h"



/****************************************************************/
/*                              Internal Function Declare                                          */
/****************************************************************/
static inline void  I2C_Start( void );
static inline void I2C_Stop();
static inline u8 I2C_GetAck();
static inline void I2C_Ack(unsigned Ack);
static inline u8  I2C_IO_IputGet(u8 ioNum);
static u8 I2C_Read_1byte() ;
static void I2C_Write_1byte( u8 Data );


#define I2C_FAIL        0
#define I2C_SUCCESS     1


gpio_t gpio_iic_1_scl;
gpio_t gpio_iic_1_sda;

void delay_us(u32 us)
{
    u8 i = 0;
    while(us--){
        i = 50;//16;
        while(i--);
    }
        
}

                                                                  
/*******************************************************************************
**函    数： IIC_delay
**功    能： IIC时钟延时  -- 决定IIC传输速度
**参    数： void
**返    回： void
**说    明： 1bit cost double delay
**           if delay is 5us , IIC Speed is 100Kbps
*******************************************************************************/
static void IIC_delay(void) 
{
    delay_us(10);    
}



/*******************************************************************************
**函    数： IIC_InitPort
**功    能： IIC接口初始化
**参    数： void
**返    回： void
*******************************************************************************/
void IIC_InitPort(void)
{                        

    
    printf("\n BL5372 IIC Port Init");
    // Init IIC_1_SCL_PIN
      gpio_init(&gpio_iic_1_scl, IIC_1_SCL_PIN);
      gpio_dir(&gpio_iic_1_scl, PIN_OUTPUT);    // Direction: Output
      gpio_mode(&gpio_iic_1_scl, PullNone);     // No pull
      
      gpio_write(&gpio_iic_1_scl, 1);


      // Init IIC_1_SDA_PIN
      gpio_init(&gpio_iic_1_sda, IIC_1_SDA_PIN);
      gpio_dir(&gpio_iic_1_sda, PIN_OUTPUT);    // Direction: Output
      gpio_mode(&gpio_iic_1_sda, PullNone);     // No pull
    
      gpio_write(&gpio_iic_1_sda, 1);
      
      SDA_OUT();
      SCL_OUT();
      SCL1;
      SDA1;
}






/****************************************************************/
/*                                      Real Fuctions                                                    */
/****************************************************************/
/*                                             NOTE:

BL5372 最大通讯速率为100KHZ;

目前采用的通讯速率为50HZ:

delay_1us(10)


*/
extern void delay_us(u32 us);


static inline void Release_I2C(void)
{
    
}
u8 bcd2dec(u8 data)
{
    return (((data>>4)*10) + (data&0x0f));
}

u8 dec2bcd(u8 data)
{
    return (((data/10)<<4) | (data%10));
}

/********************************************************************
static inline BYTE  I2C_IO_IputGet( BYTE ioNum )
Parameters:	
       BYTE ioNum   
Returns:	
       BYTE t
Description:
        read intput
********************************************************************/
static inline u8  I2C_IO_IputGet( u8 ioNum )
{
    u8 t;   
    t = gpio_read(&gpio_iic_1_sda);
    return t;
}

/********************************************************************
static inline void  I2C_Start( void )
Parameters:	
        None
Returns:	
        None
Description:
        init i2c, send start cmd
        ____
SDA        |_______
        _______
SCL              |____

********************************************************************/
static inline void  I2C_Start( void )
{
    
    SDA1;
    
    SCL1;
    
    IIC_delay();
    
    SDA0;
    IIC_delay();
    
    SCL0;
    IIC_delay();
}

/********************************************************************
static inline void I2C_Stop()
Parameters:	
        None
Returns:	
        None
Description:
        Stop I2C Bus
                   ___________
SDA  ______|
        _________
SCL                  |________

********************************************************************/
static inline void I2C_Stop()
{
 
   SDA0;
   IIC_delay();
   SCL1;
   IIC_delay();
   SDA1;
   IIC_delay();
}

/********************************************************************
static void I2C_Write_1byte( UINT8 Data )
Parameters:	
        uchar dat
Returns:	
        None
Description:
        WRITE 8 BIT TO 24C02   
        -------------------------------------
         /            8  bit    data                              \
SDA  -------------------------------------
            __    __    __   __    __    __   __    __    
SCL  __|  |_|  |_|  |_|  |_|  |_|  |_|  |_|  |_
********************************************************************/
static void I2C_Write_1byte( u8 Data )
{
   
    u8 i;    
    for(i=0;i<8;i++)
    {   
        if(Data&0x80)
        {
            SDA1;
        }   
        else  
        {
            SDA0;
        }
            
        Data<<=1;
        
        SCL1;
        IIC_delay();
        
        SCL0;
        IIC_delay();    
    }
    
    
    
}


/********************************************************************
static BYTE I2C_Read_1byte() 
Parameters:	
        Void
Returns:	
        dat
Description:
        READ 8 BIT   
********************************************************************/
static u8 I2C_Read_1byte() 
{
   
    u8 i,dat=0;
    SDA_IN();
    
//    SCL0;//xjx.add
    
    for(i=0;i<8;i++)
    {   
       SCL1;
       IIC_delay();
       
       dat=dat<<1;
       if(Get_SDA()) 
       {
          dat |= 0x01;
       }
           
       SCL0;
       IIC_delay();
    }   
    SDA_OUT();   
    SDA1;   
    return dat;  
 }

/********************************************************************
uchar I2C_GetAck()
Parameters:	
        None
Returns:	
        None
Description:
        Slave Ack ;return 1:communicaton success;  return 0:communication fail
;      
********************************************************************/
static inline u8 I2C_GetAck()
{      
    u32 undead = 0;//10000;

    SDA_IN();       //输入
    //IIC_delay(); 
    
    SCL1; 
    IIC_delay();
    
    while(Get_SDA())
    {   
        if(++undead>10000)
        {           
            SDA_OUT();                  
            return 0;
        } 
    }
    SCL0; 
    IIC_delay(); 
    SDA_OUT();
    IIC_delay(); 
    return 1;  
}

/********************************************************************
uchar I2C_Ack()
Parameters:	
       unsigned Ack
Returns:	
        None
Description:
        Host  Ack ;if Ack=1,stop read;if Ack=0,continue read       
********************************************************************/
static inline void I2C_Ack(unsigned Ack)
{
    
    if(Ack==1) 
    {
        SDA1;
    }
    else
    {
        SDA0;
    }
             
    SCL1;   
    IIC_delay();
    
    SCL0;
    IIC_delay();
}

/********************************************************************
BYTE  API_Read_I2C(BYTE SlaveAdd, BYTE address)
Parameters:	
      BYTE address
Returns:	
        None
Description:
        Read  device      
 NOTE:before read slave,we must write slave address and slave register
********************************************************************/
u8  API_Read_I2C(u8 SlaveAdd, u8 address)
{ 


    u8 ch;
    u8  SlaveAdd_write;
    SlaveAdd_write= SlaveAdd&0xfe;//seven bit slave address +write operation
    SlaveAdd |=0x01;//seven bit slave address +read operation
    I2C_Start() ;

    I2C_Write_1byte(SlaveAdd_write);
    if( !I2C_GetAck() )
    {
        printf( "no ack after write to slave(0x%02x) address !\n", SlaveAdd );
        //goto Read_ERR;
    }    
   
    I2C_Write_1byte(address);
    if( !I2C_GetAck() )
    {
        printf( "no ack after write to slave(0x%02x) address !\n", SlaveAdd );
        //goto Read_ERR;
    }

    I2C_Start() ;
     
    I2C_Write_1byte(SlaveAdd);
    if(! I2C_GetAck() )
    {
        printf( "no ack after write to slave(0x%02x) address !\n", SlaveAdd );
        //goto Read_ERR;
    }
    
    ch= I2C_Read_1byte();
    ch=bcd2dec(ch);
    I2C_Ack(1);

    I2C_Stop();
    delay_us(10); 
    return(ch);
    
Read_ERR:
    return I2C_FAIL;         
}

/********************************************************************
BYTE API_Write_I2C( BYTE SlaveAdd, BYTE SlaveOffset, BYTE Data )
Parameters:	
      BYTE SlaveAdd, BYTE SlaveOffset, BYTE Data
Returns:	
        None
Description:
        Write slave       
********************************************************************/
u8 API_Write_I2C( u8 SlaveAdd, u8 SlaveOffset, u8 Data )
{
    SlaveAdd &= 0xfe;    //seven bit slave address +write operation
    I2C_Start();
 
    I2C_Write_1byte( SlaveAdd );
    if( !I2C_GetAck() )
    {
       printf( "no ack after write to slave(0x%02x) address !\n", SlaveAdd );
        //goto Write_ERR;
    }

 
    I2C_Write_1byte( SlaveOffset ); //slave regisister address
    if(! I2C_GetAck() )
   {
       printf( "no ack after write to slave(0x%02x) ,offset(0x%02x) !\n", SlaveAdd, SlaveOffset );
        //goto Write_ERR;
    }

   Data=dec2bcd(Data);
   I2C_Write_1byte( Data );  //data
   if( !I2C_GetAck() )
    {
        printf( "no ack after write to device (0x%02x,0x%02x,0x%02x)!\n", SlaveAdd, SlaveOffset, Data );
        //goto Write_ERR;
    }

    I2C_Stop();
    delay_us(50); 
    return I2C_SUCCESS;

 Write_ERR:
    return I2C_FAIL;    
}

/********************************************************************
BYTE  API_Read_Nbyte_I2C(BYTE SlaveAdd, BYTE address,BYTE* pReadBuf, BYTE n)
Parameters:	
       BYTE SlaveAdd, BYTE address,BYTE* pReadBuf, BYTE n
Returns:	
       None
Description:
        read/write data
********************************************************************/

u8  API_Read_Nbyte_I2C(u8 SlaveAdd, u8 address,u8* pReadBuf, u8 n)
{ 

    u8 i;
    u8  SlaveAdd_write;
    SlaveAdd_write= SlaveAdd&0xfe;
    SlaveAdd |=0x01;
    I2C_Start() ;

    I2C_Write_1byte(SlaveAdd_write);
    if(! I2C_GetAck() )
    {
        printf( "no ack after write to slave(0x%02x) address !\n", SlaveAdd );
        //goto Read_ERR;      
    }    
    
    I2C_Write_1byte(address);
    if( !I2C_GetAck() )
    {
        printf( "no ack after write to slave(0x%02x) address !\n", SlaveAdd );
        //goto Read_ERR;
    }

     I2C_Start() ;
     
    I2C_Write_1byte(SlaveAdd);
    if( !I2C_GetAck() )
    {
        printf( "no ack after write to slave(0x%02x) address !\n", SlaveAdd );
        //goto Read_ERR;
    }
    
    n -= 1;
    for( i = 0; i < n ; i++ )
    {
        *( pReadBuf + i ) = bcd2dec(I2C_Read_1byte());
        I2C_Ack(0);
    }
    *( pReadBuf + i ) = bcd2dec(I2C_Read_1byte());
    I2C_Ack(1);

    I2C_Stop();
    delay_us(10); 
    Release_I2C();
    return(1);
Read_ERR:
    return I2C_FAIL;
       
}

/********************************************************************
BYTE API_Write_Nbyte_I2C( BYTE SlaveAdd, BYTE SlaveOffset, BYTE* pWriteBuf, BYTE n )

Parameters:	
       BYTE SlaveAdd, BYTE address,BYTE* pReadBuf, BYTE n
Returns:	
       None
Description:
        read/write data
********************************************************************/

u8 API_Write_Nbyte_I2C( u8 SlaveAdd, u8 SlaveOffset, u8* pWriteBuf, u8 n )
{

    SlaveAdd &= 0xfe;

    I2C_Start();


    I2C_Write_1byte( SlaveAdd );
    if(! I2C_GetAck() )
    {
        printf( "no ack after write to slave(0x%02x) address !\n", SlaveAdd );
        //goto Write_ERR;
    }


    I2C_Write_1byte( SlaveOffset );

    if( !I2C_GetAck() )
    {
        printf( "no ack after write to slave(0x%02x) ,offset(0x%02x) !\n", SlaveAdd, SlaveOffset );
        //goto Write_ERR;
    }
    u8 i;
    for(i=0;i<n;i++)
    {

        *( pWriteBuf + i ) =dec2bcd(*( pWriteBuf + i ) );
        I2C_Write_1byte( *( pWriteBuf + i ) );                

        if(! I2C_GetAck() )
        {
            printf( "no ack after write to slave(0x%02x) address !\n", SlaveAdd );
            //goto Write_ERR;
        }
    }

    I2C_Stop();
    delay_us(10); 
    Release_I2C();
    return I2C_SUCCESS;

    Write_ERR:
    return I2C_FAIL;
    
}

/********************************* END FILE ***********************************/
