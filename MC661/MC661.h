#ifndef __MC661_H
#define __MC661_H	
#include "stm32f10x.h"
void Uart1_SendStr(char*SendBuf);//串口1打印数据;
void Clear_Buffer(void);//清空缓存
void  MC661_Init(void);//M26初始化
void MC661Send_StrData(char *bufferdata);
void  MC661_AliyunMQTTInit(void);
void aliyunMQTT_PUBdata(uint8_t temp,uint8_t humi);
uint8_t Mqttaliyun_Savedata(uint8_t *t_payload,uint8_t temp,uint8_t humi);

////阿里云的三元素,在设备里面都可以查询到//////////////////////
#define ProductKey     "a1ZMkwhevG9"             //产品KEY
#define DeviceName          "mzh001"      //
#define DeviceSecret    "cc96840980ac49570acc47e2c0195b5c"  //
#define PubTopic         "/sys/a1ZMkwhevG9/mzh001/thing/event/property/post"//发布主题
#define SubTopic        "/sys/a1NXoxeFw4c/mzh002/thing/service/property/set"//订阅主题
#endif  

