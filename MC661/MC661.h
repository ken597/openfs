#ifndef __MC661_H
#define __MC661_H	
#include "stm32f10x.h"
void Uart1_SendStr(char*SendBuf);//����1��ӡ����;
void Clear_Buffer(void);//��ջ���
void  MC661_Init(void);//M26��ʼ��
void MC661Send_StrData(char *bufferdata);
void  MC661_AliyunMQTTInit(void);
void aliyunMQTT_PUBdata(uint8_t temp,uint8_t humi);
uint8_t Mqttaliyun_Savedata(uint8_t *t_payload,uint8_t temp,uint8_t humi);

////�����Ƶ���Ԫ��,���豸���涼���Բ�ѯ��//////////////////////
#define ProductKey     "a1ZMkwhevG9"             //��ƷKEY
#define DeviceName          "mzh001"      //
#define DeviceSecret    "cc96840980ac49570acc47e2c0195b5c"  //
#define PubTopic         "/sys/a1ZMkwhevG9/mzh001/thing/event/property/post"//��������
#define SubTopic        "/sys/a1NXoxeFw4c/mzh002/thing/service/property/set"//��������
#endif  

