#include "MC661.h"
#include "usart.h"	 
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "delay.h"
char *strx=0,*extstrx,*Readystrx,*Errstrx; 	//返回值指针判断
char Timestr[100];//时间字符串
extern char  RxBuffer[100],RxCounter;
void Uart1_SendStr(char*SendBuf)//串口2打印数据
{
	while(*SendBuf)
	{
	  while((USART1->SR&0X40)==0)
      {}//等待发送完成 
      USART1->DR = (u8) *SendBuf; 
		SendBuf++;
	}
}
void Clear_Buffer(void)//清空缓存
{
		u8 i;
    Uart1_SendStr(RxBuffer);
		for(i=0;i<RxCounter;i++)
		RxBuffer[i]=0;//缓存
		RxCounter=0;
		IWDG_Feed();//喂狗
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
		strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
	while(strx==NULL)
		{
				Clear_Buffer();	
				printf("AT\r\n"); 
				delay_ms(500);
				strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
		}
		printf("ATE0\r\n"); //关闭回显
		delay_ms(500);
		Clear_Buffer();	
		printf("AT+CSQ\r\n"); //检查CSQ
		delay_ms(500);
		/////////////////////////////////
		printf("AT+CPIN?\r\n");//检查SIM卡是否在位
		delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//查看是否返回ready
	while(strx==NULL)
		{
				Clear_Buffer();
				printf("AT+CPIN?\r\n");
				delay_ms(500);
				strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//检查SIM卡是否在位，等待卡在位，如果卡识别不到，剩余的工作就没法做了
		}
		Clear_Buffer();	
			///////////////////////////////////
		printf("AT+MQTTCLOSE=1\r\n");//CLOSE MQTT
		delay_ms(500);
		///////////////////////////////////

		/////////////////////////////////////
		printf("AT+CEREG?\r\n");//查看是否注册GPRS网络
		delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"+CEREG: 0,1");//，这里重要，只有注册成功，才可以进行GPRS数据传输。
		extstrx=strstr((const char*)RxBuffer,(const char*)"+CEREG: 0,5");//返回正常，漫游
	while(strx==NULL&&extstrx==NULL)
		{
				Clear_Buffer();
				printf("AT+CEREG?\r\n");//查看是否注册GPRS网络
				delay_ms(500);
				strx=strstr((const char*)RxBuffer,(const char*)"+CEREG: 0,1");//，这里重要，只有注册成功，才可以进行GPRS数据传输。
				extstrx=strstr((const char*)RxBuffer,(const char*)"+CEREG: 0,5");//返回正常，漫游
		}
		Clear_Buffer();
    printf("AT+MIPCALL=1\r\n");//激活场景，注册基站，获取IP
		delay_ms(500);
		Clear_Buffer();
	}



void  MC661_AliyunMQTTInit(void)
{
	  u8 keydata[200],i,j;
	  printf("AT+MQTTAUTHCFG=1,\"%s\",\"%s\",\"%s\",0\r\n",ProductKey,DeviceName,DeviceSecret);//配置三元素，宏定义代码修改即可
	  delay_ms(500);
		strx=strstr((const char*)RxBuffer,(const char*)"+MQTTAUTHCFG:");//
		while(strx==NULL)
		{
			strx=strstr((const char*)RxBuffer,(const char*)"+MQTTAUTHCFG:");//返回登录密码信息
		}
		strx=strstr((const char*)RxBuffer,(const char*)"\"");//获取到双引号所在位置
			for(i=0;;i++)
			{
				if(strx[i]==0x0D)
					break;
				keydata[i]=strx[i];
				
			}
		keydata[i]=0;//获取到登录信息，下一步进行阿里云登录
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
		strx=strstr((const char*)RxBuffer,(const char*)"+MQTTOPEN: 1,1");//登录成功返回在线
		while(strx==NULL)
		{
			strx=strstr((const char*)RxBuffer,(const char*)"+MQTTOPEN: 1,1");//登录成功返回在线
		}
		Clear_Buffer();
}


void aliyunMQTT_PUBdata(uint8_t temp,uint8_t humi)
{
     uint8_t t_payload[200],len;
     len=Mqttaliyun_Savedata(t_payload,temp,humi);
     t_payload[len]=0;
	  // printf("AT+QMTPUB=1,\"%s\",0,0,\"%s\"\r\n",PubTopic,t_payload);//发布主题
   	 printf("AT+MQTTPUB=1,\"%s\",0,0,%d\r\n",PubTopic,len);
		 delay_ms(300);
	   Clear_Buffer(); 
	   printf(t_payload);
     strx=strstr((const char*)RxBuffer,(const char*)"+MQTTPUB: 1,1");//看下返回状态
  while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+MQTTPUB: 1,1");//看下返回状态
    }
	  delay_ms(500);
    Clear_Buffer(); 
}

//访问阿里云需要提交JSON数据
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



