#ifndef SENSOR_UART_H
#define SENSOR_UART_H

#include "hi_io.h"
#include "hi_gpio.h"
#include "hi_uart.h"
#include "stdbool.h"

extern hi_u8 Databuff[12];
extern hi_u8 sendbuff[12];

void uart_init(void);
void GetAirTemp_Humi_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize);
void GetSoilTemp_Humi_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize);
void GetPH_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize);
void GetNPK_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize);
void GetCO2_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize);
void GetLightIntens_Data(hi_u8 Sensor_Addr, hi_u8* databuff, hi_u8 datasize);


#endif