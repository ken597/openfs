#include "MC661.h"
#include "usart.h"	 
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "delay.h"
char *strx=0,*extstrx,*Readystrx,*Errstrx; 	//����ֵָ���ж�
char Timestr[100];//ʱ���ַ���
extern char  RxBuffer[100],RxCounter;
void Uart1_SendStr(char*SendBuf)//����2��ӡ����
{
	while(*SendBuf)
	{
	  while((USART1->SR&0X40)==0)
      {}//�ȴ�������� 
      USART1->DR = (u8) *SendBuf; 
		SendBuf++;
	}
}
void Clear_Buffer(void)//��ջ���
{
		u8 i;
    Uart1_SendStr(RxBuffer);
		for(i=0;i<RxCounter;i++)
		RxBuffer[i]=0;//����
		RxCounter=0;
		IWDG_Feed();//ι��
}
void  MC661_Init(void)
{
		printf("+++"); 
		delay_ms(500);
		printf("+++"); 
		delay_ms(500);
		printf("AT\r\n"); 
		delay_ms(500);
		printf("AT\r\n"); 
		delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
	while(strx==NULL)
		{
				Clear_Buffer();	
				printf("AT\r\n"); 
				delay_ms(500);
				strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
		}
		printf("ATE0\r\n"); //�رջ���
		delay_ms(500);
		Clear_Buffer();	
		printf("AT+CSQ\r\n"); //���CSQ
		delay_ms(500);
		/////////////////////////////////
		printf("AT+CPIN?\r\n");//���SIM���Ƿ���λ
		delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//�鿴�Ƿ񷵻�ready
	while(strx==NULL)
		{
				Clear_Buffer();
				printf("AT+CPIN?\r\n");
				delay_ms(500);
				strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//���SIM���Ƿ���λ���ȴ�����λ�������ʶ�𲻵���ʣ��Ĺ�����û������
		}
		Clear_Buffer();	
			///////////////////////////////////
		printf("AT+MQTTCLOSE=1\r\n");//CLOSE MQTT
		delay_ms(500);
		///////////////////////////////////

		/////////////////////////////////////
		printf("AT+CEREG?\r\n");//�鿴�Ƿ�ע��GPRS����
		delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"+CEREG: 0,1");//��������Ҫ��ֻ��ע��ɹ����ſ��Խ���GPRS���ݴ��䡣
		extstrx=strstr((const char*)RxBuffer,(const char*)"+CEREG: 0,5");//��������������
	while(strx==NULL&&extstrx==NULL)
		{
				Clear_Buffer();
				printf("AT+CEREG?\r\n");//�鿴�Ƿ�ע��GPRS����
				delay_ms(500);
				strx=strstr((const char*)RxBuffer,(const char*)"+CEREG: 0,1");//��������Ҫ��ֻ��ע��ɹ����ſ��Խ���GPRS���ݴ��䡣
				extstrx=strstr((const char*)RxBuffer,(const char*)"+CEREG: 0,5");//��������������
		}
		Clear_Buffer();
    printf("AT+MIPCALL=1\r\n");//�������ע���վ����ȡIP
		delay_ms(500);
		Clear_Buffer();
	}



void  MC661_AliyunMQTTInit(void)
{
	  u8 keydata[200],i,j;
	  printf("AT+MQTTAUTHCFG=1,\"%s\",\"%s\",\"%s\",0\r\n",ProductKey,DeviceName,DeviceSecret);//������Ԫ�أ��궨������޸ļ���
	  delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"+MQTTAUTHCFG:");//
		while(strx==NULL)
		{
			strx=strstr((const char*)RxBuffer,(const char*)"+MQTTAUTHCFG:");//���ص�¼������Ϣ
		}
		strx=strstr((const char*)RxBuffer,(const char*)"\"");//��ȡ��˫��������λ��
			for(i=0;;i++)
			{
				if(strx[i]==0x0D)
					break;
				keydata[i]=strx[i];
				
			}
		keydata[i]=0;//��ȡ����¼��Ϣ����һ�����а����Ƶ�¼
		Uart1_SendStr(keydata);
		Clear_Buffer();
		printf("AT+MQTTUSER=1,%s\r\n",keydata);
	  delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"OK");//
		while(strx==NULL)
		{
			strx=strstr((const char*)RxBuffer,(const char*)"OK");//
		}
		Clear_Buffer();
		printf("AT+MQTTOPEN=1,\"iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883,0,300\r\n"); 
		strx=strstr((const char*)RxBuffer,(const char*)"+MQTTOPEN: 1,1");//
	  delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"+MQTTOPEN: 1,1");//��¼�ɹ���������
		while(strx==NULL)
		{
			strx=strstr((const char*)RxBuffer,(const char*)"+MQTTOPEN: 1,1");//��¼�ɹ���������
		}
		Clear_Buffer();
}


void aliyunMQTT_PUBdata(uint8_t temp,uint8_t humi)
{
     uint8_t t_payload[200],len;
     len=Mqttaliyun_Savedata(t_payload,temp,humi);
     t_payload[len]=0;
	  // printf("AT+QMTPUB=1,\"%s\",0,0,\"%s\"\r\n",PubTopic,t_payload);//��������
   	 printf("AT+MQTTPUB=1,\"%s\",0,0,%d\r\n",PubTopic,len);
		 delay_ms(300);
	   Clear_Buffer(); 
	   printf(t_payload);
     strx=strstr((const char*)RxBuffer,(const char*)"+MQTTPUB: 1,1");//���·���״̬
  while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+MQTTPUB: 1,1");//���·���״̬
    }
	  delay_ms(500);
    Clear_Buffer(); 
}

//���ʰ�������Ҫ�ύJSON����
uint8_t Mqttaliyun_Savedata(uint8_t *t_payload,uint8_t temp,uint8_t humi)
{

    char json[]="{\"id\":\"26\",\"version\":\"1.0\",\"sys\":{\"ack\":0},\"params\":{\"CurrentTemperature\":{\"value\":%d},\"CurrentHumidity\":{\"value\":%d}},\"method\":\"thing.event.property.post\"}";	 //     char json[]="{\"datastreams\":[{\"id\":\"location\",\"datapoints\":[{\"value\":{\"lon\":%2.6f,\"lat\":%2.6f}}]}]}";
    char t_json[200];
    unsigned short json_len;
    sprintf(t_json, json, temp, humi);
    json_len = strlen(t_json)/sizeof(char);
  	memcpy(t_payload, t_json, json_len);
    return json_len;
}



