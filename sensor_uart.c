#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "hi_io.h"
#include "hi_gpio.h"
#include "hi_uart.h"
#include "stdbool.h"

#include "hal_bsp_ssd1306.h"
#include "hal_bsp_pcf8574.h"

#include "sensor_uart.h"


/*
学习要点总结：

*/
hi_u8 Databuff[12] = {0};
hi_u8 sendbuff[12] = {0};

#define TASK_DELAY_TIME (1000 * 1000)    //  任务执行周期1000ms

/*引脚宏定义*/
#define TXD1 HI_GPIO_IDX_6
#define RXD1 HI_GPIO_IDX_5
#define TXD2 HI_GPIO_IDX_11
#define RXD2 HI_GPIO_IDX_12

#define TASK_STACK_SIZE (1024 * 5)

//直接计算法CRC校验
unsigned short do_crc(unsigned char *ptr, int len)
{
    unsigned int i;
    unsigned short crc = 0xFFFF;
    
    while(len--)
    {
        crc ^= *ptr++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
    }
    
    return crc;
}

/**
 * @brief  串口初始化
 * @note   
 * @retval None
 */
void uart_init(void)
{
    uint32_t ret = 0,ret1 = 0;
    // 初始化串口
    /*串口2，用于与其他外设通信*/   
    hi_io_set_func(HI_IO_NAME_GPIO_11, HI_IO_FUNC_GPIO_11_UART2_TXD);
    hi_io_set_func(HI_IO_NAME_GPIO_12, HI_IO_FUNC_GPIO_12_UART2_RXD);

    hi_uart_attribute uart2_param = {
        .baud_rate = 9600,          //波特率9600
        .data_bits = 8,             //8位数据位
        .stop_bits = 1,             //1位停止位
        .parity = 0,                //不校验
    };
    
    ret1 = hi_uart_init(HI_UART_IDX_2, &uart2_param, NULL);

    if (ret1 != HI_ERR_SUCCESS) {
        printf("hi uart2 init is faild.\r\n");
    }
}

void uart1_send_buff(unsigned char *str, unsigned short len)
{
    hi_u32 ret = 0;
    ret = hi_uart_write(HI_UART_IDX_1, (uint8_t *)str, len);
    if (ret == HI_ERR_FAILURE)
    {
        printf("uart send buff is failed.\r\n");
    }
}

void uart2_send_buff(unsigned char *str, unsigned short len)
{
    hi_u32 ret = 0;
    ret = hi_uart_write(HI_UART_IDX_2, (uint8_t *)str, len);
    if (ret == HI_ERR_FAILURE)
    {
        printf("uart send buff is failed.\r\n");
    }
}

void GetAirTemp_Humi_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize)
{
    for(uint8_t i=0; i<12; i++)
    {
        //databuff[i] = 0x00;
        *(databuff+i) = 0x00;
    }
    hi_u8 retval;
    //uart请求传感器数据
    if(Sensor_Addr == 0x04)
    {
        sendbuff[0] = Sensor_Addr;
        sendbuff[1] = 0x03;
        sendbuff[2] = 0x00;
        sendbuff[3] = 0x00;
        sendbuff[4] = 0x00;
        sendbuff[5] = datasize;

        //CRC校验数据帧
        hi_u16 crc=do_crc(sendbuff, 6);
        sendbuff[6] = crc;
        sendbuff[7] = crc>>8;
        
        uart2_send_buff(sendbuff, 8);

        //uart返回传感器数据，传入Databuff数组中

        retval = hi_uart_read_timeout(HI_UART_IDX_2, databuff, 9, 500);
        if(retval == HI_ERR_FAILURE)
        {
            printf("Get Sensor Data Failed");
        }
    }
}

void GetSoilTemp_Humi_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize)
{
    for(uint8_t i=0; i<12; i++)
    {
        //databuff[i] = 0x00;
        *(databuff+i) = 0x00;
    }
    hi_u8 retval;
    //uart请求传感器数据
    if(Sensor_Addr == 0x01)
    {
        sendbuff[0] = Sensor_Addr;
        sendbuff[1] = 0x03;
        sendbuff[2] = 0x00;
        sendbuff[3] = 0x06;
        sendbuff[4] = 0x00;
        sendbuff[5] = datasize;

        //CRC校验数据帧
        hi_u16 crc=do_crc(sendbuff, 6);
        sendbuff[6] = crc;
        sendbuff[7] = crc>>8;
        
        uart2_send_buff(sendbuff, 8);

        //uart返回传感器数据，传入Databuff数组中

        retval = hi_uart_read_timeout(HI_UART_IDX_2, databuff, 9, 500);
        if(retval == HI_ERR_FAILURE)
        {
            printf("Get Sensor Data Failed");
        }
    }
}

void GetPH_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize)
{
    for(uint8_t i=0; i<12; i++)
    {
        //databuff[i] = 0x00;
        *(databuff+i) = 0x00;
    }
    hi_u8 retval;
    //uart请求传感器数据
    if(Sensor_Addr == 0x01)
    {
        sendbuff[0] = Sensor_Addr; //传感器地址
        sendbuff[1] = 0x03;        //代表查询功能
        sendbuff[2] = 0x00;        //起始地址高位
        sendbuff[3] = 0x08;        //起始地址低位
        sendbuff[4] = 0x00;        //数据长度高位
        sendbuff[5] = datasize;    //数据长度低位

        //CRC校验数据帧
        hi_u16 crc=do_crc(sendbuff, 6);
        sendbuff[6] = crc;
        sendbuff[7] = crc>>8;
        
        uart2_send_buff(sendbuff, 8);//进行数据查询

        //uart返回传感器数据，传入databuff数组中
        retval = hi_uart_read_timeout(HI_UART_IDX_2, databuff, 9, 500);
        if(retval == HI_ERR_FAILURE)
        {
            printf("Get Sensor Data Failed");
        }
    }

}

void GetNPK_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize)
{
    for(uint8_t i=0; i<12; i++)
    {
        //databuff[i] = 0x00;
        *(databuff+i) = 0x00;
    }
    hi_u8 retval;
    //uart请求传感器数据
    if(Sensor_Addr == 0x01)
    {
        sendbuff[0] = Sensor_Addr; //传感器地址
        sendbuff[1] = 0x03;        //代表查询功能
        sendbuff[2] = 0x00;        //起始地址高位
        sendbuff[3] = 0x1E;        //起始地址低位
        sendbuff[4] = 0x00;        //数据长度高位
        sendbuff[5] = datasize;    //数据长度低位

        //CRC校验数据帧
        hi_u16 crc=do_crc(sendbuff, 6);
        sendbuff[6] = crc;
        sendbuff[7] = crc>>8;
        
        uart2_send_buff(sendbuff, 8);//进行数据查询

        //uart返回传感器数据，传入databuff数组中
        retval = hi_uart_read_timeout(HI_UART_IDX_2, databuff, 11, 500);
        if(retval == HI_ERR_FAILURE)
        {
            printf("Get Sensor Data Failed");
        }
    }
}

void GetCO2_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize)
{
    for(uint8_t i=0; i<12; i++)
    {
        //databuff[i] = 0x00;
        *(databuff+i) = 0x00;
    }
    hi_u8 retval;
    //uart请求传感器数据
    if(Sensor_Addr == 0x02)
    {
        sendbuff[0] = Sensor_Addr; //传感器地址
        sendbuff[1] = 0x03;        //代表查询功能
        sendbuff[2] = 0x00;        //起始地址高位
        sendbuff[3] = 0x04;        //起始地址低位
        sendbuff[4] = 0x00;        //数据长度高位
        sendbuff[5] = datasize;    //数据长度低位

        //CRC校验数据帧
        hi_u16 crc=do_crc(sendbuff, 6);
        sendbuff[6] = crc;
        sendbuff[7] = crc>>8;
        
        uart2_send_buff(sendbuff, 8);//进行数据查询

        //uart返回传感器数据，传入databuff数组中
        retval = hi_uart_read_timeout(HI_UART_IDX_2, databuff, 7, 500);
        if(retval == HI_ERR_FAILURE)
        {
            printf("Get Sensor Data Failed");
        }
    }
}

void GetLightIntens_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize)//校验码64  29
{
    for(uint8_t i=0; i<12; i++)
    {
        //databuff[i] = 0x00;
        *(databuff+i) = 0x00;
    }
    hi_u8 retval;
    //uart请求传感器数据
    if(Sensor_Addr == 0x09)
    {
        sendbuff[0] = Sensor_Addr; //传感器地址
        sendbuff[1] = 0x03;        //代表查询功能
        sendbuff[2] = 0x00;        //起始地址高位
        sendbuff[3] = 0x02;        //起始地址低位
        sendbuff[4] = 0x00;        //数据长度高位
        sendbuff[5] = datasize;    //数据长度低位

        //CRC校验数据帧
        hi_u16 crc=do_crc(sendbuff, 6);
        sendbuff[6] = crc;
        sendbuff[7] = crc>>8;
        
        uart2_send_buff(sendbuff, 8);//进行数据查询

        //uart返回传感器数据，传入databuff数组中
        retval = hi_uart_read_timeout(HI_UART_IDX_2, databuff, 9, 500);
        if(retval == HI_ERR_FAILURE)
        {
            printf("Get Sensor Data Failed");
        }
    }
}

