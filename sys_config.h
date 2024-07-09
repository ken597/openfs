/*
 * Copyright (c) 2023 Beijing HuaQing YuanJian Education Technology Co., Ltd
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include "cmsis_os2.h"
#include "hal_bsp_structAll.h"

// DeviceSecret fs12345678
// 设备ID
#define DEVICE_ID "6669a7e36bc31504f06e7a2f_hi3861_farm"
// MQTT客户端ID
#define MQTT_CLIENT_ID "6669a7e36bc31504f06e7a2f_hi3861_farm_0_0_2024062704"
// MQTT用户名
#define MQTT_USER_NAME "6669a7e36bc31504f06e7a2f_hi3861_farm"
// MQTT密码
#define MQTT_PASS_WORD "9f555d3c028b3f6f01ffaa970c99726d264b903b718a3d96db58f92ab546fe7c"
// 华为云平台的IP地址
#define SERVER_IP_ADDR "117.78.5.125"
// 华为云平台的IP端口号
#define SERVER_IP_PORT 1883
// 订阅 接收控制命令的主题
#define MQTT_TOPIC_SUB_COMMANDS "$oc/devices/%s/sys/commands/#"
// 发布 成功接收到控制命令后的主题
#define MQTT_TOPIC_PUB_COMMANDS_REQ "$oc/devices/%s/sys/commands/response/request_id=%s"
#define MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ "$oc/devices//sys/commands/response/request_id="
// 发布 设备属性数据的主题
#define MQTT_TOPIC_PUB_PROPERTIES "$oc/devices/%s/sys/properties/report"
#define MALLOC_MQTT_TOPIC_PUB_PROPERTIES "$oc/devices//sys/properties/report"

/*typedef struct {
    int top;  // 上边距
    int left; // 下边距
    int width; // 宽
    int hight; // 高
} margin_t;   // 边距类型
*/
typedef struct message_data {
    unsigned char fanStatus; // 风扇的状态
    unsigned char ferStatus; // 风扇的状态
    unsigned char WaterStatus; // 风扇的状态
    unsigned char SunStatus; // 风扇的状态
    unsigned char bulbStatus; // 风扇的状态
/*     unsigned short smartControl_flag;
    unsigned short Light_upper;
    unsigned short Light_lower;
    unsigned char airtemp_upper;
    unsigned char airtemp_lower;
    unsigned char airhumid_upper;
    unsigned char airhumid_lower;
    unsigned char soilhumid_upper;
    unsigned char soilhumid_lower;
    unsigned short N_lower;
    unsigned char P_lower;
    unsigned char K_lower; */
    int air_humid;          // 空气湿度值
    int air_temp;       // 空气温度值
    int soil_humid;          // 土壤湿度值
    int soil_temp;       // 土壤温度值
    float PH;          // 土壤PH值
    int CO2;       // 空气CO2浓度
    int elec;          // 土壤电导率
    int N;       // 土壤氮值
    int P;       // 土壤磷值
    int K;       // 土壤钾值
    uint32_t Light; //光照度
    hi_nv_save_sensor_threshold nvFlash;
    tn_pcf8574_io_t pcf8574_io;
} msg_data_t;

extern msg_data_t sys_msg_data; // 传感器的数据

#endif
