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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "cJSON.h"
#include "hi_nv.h"
#include "sys_config.h"

#include "hal_bsp_wifi.h"
#include "hal_bsp_mqtt.h"
//#include "hal_bsp_key.h"
#include "hal_bsp_ssd1306.h"
#include "hal_bsp_nfc.h"
#include "hal_bsp_pcf8574.h"
#include "hal_bsp_nfc_to_wifi.h" //碰一碰配网

#include "mqtt_send_task.h"
#include "mqtt_recv_task.h"
#include "sensor_collect_task.h"
#include "sensor_uart.h"
#include "oled_show_log.h"

#define TASK1_STACK_SIZE (6*1024)
#define TASK2_STACK_SIZE (6.5*1024)
#define TASK3_STACK_SIZE (4*1024)
#define TASK_INIT_DELAY 2 // s

osThreadId_t mqtt_send_task_id;      // mqtt 发布数据任务ID
osThreadId_t mqtt_recv_task_id;      // mqtt 接收数据任务ID
osThreadId_t sensor_collect_task_id; // 传感器采集任务ID
osThreadId_t show_page_id; // 显示屏展示数据ID

void SF_peripheral_init(void)
{
    // 外设的初始化
    //KEY_Init();                         // 按键初始化
    PCF8574_Init();                     // 初始化IO扩展芯片
    uart_init();
    //SHT20_Init();
    SSD1306_Init(); // OLED 显示屏初始化
    SSD1306_CLS();  // 清屏
    SSD1306_ShowStr(OLED_TEXT16_COLUMN_0, OLED_TEXT16_LINE_0, " Grain guard", TEXT_SIZE_16);
    nfc_Init();
    usleep(TASK_INIT_DELAY);
}
/* int nfc_connect_wifi_init(void)
{
    // 通过NFC芯片进行连接WiFi
    uint8_t ndefLen = 0;      // ndef包的长度
    uint8_t ndef_Header = 0;  // ndef消息开始标志位-用不到
    uint32_t result_code = 0; // 函数的返回值
    oled_consle_log("===start nfc===");

    // 读整个数据的包头部分，读出整个数据的长度
    if (result_code = NT3HReadHeaderNfc(&ndefLen, &ndef_Header) != true) {
        printf("NT3HReadHeaderNfc is failed. result_code = %d\r\n", result_code);
        return -1;
    }
    ndefLen += NDEF_HEADER_SIZE; // 加上头部字节
    if (ndefLen <= NDEF_HEADER_SIZE) {
        printf("ndefLen <= 2\r\n");
        return -1;
    }
    
    uint8_t *ndefBuff = (uint8_t *)malloc(ndefLen + 1);
    if (ndefBuff == NULL) {
        printf("ndefBuff malloc is Falied!\r\n");
        return -1;
    }
    if (result_code = get_NDEFDataPackage(ndefBuff, ndefLen) != HI_ERR_SUCCESS) {
        printf("get_NDEFDataPackage is failed. result_code = %d\r\n", result_code);
        return -1;
    }
    // 打印读出的数据
    printf("start print ndefBuff.\r\n");
    for (size_t i = 0; i < ndefLen; i++) {
        printf("0x%x ", ndefBuff[i]);
    }
    oled_consle_log("=== end nfc ===");
    sleep(TASK_INIT_DELAY);

    oled_consle_log("== start wifi==");
    // 连接WiFi
    while (NFC_configuresWiFiNetwork(ndefBuff) != WIFI_SUCCESS) {
        printf("nfc connect wifi is failed!\r\n");
        oled_consle_log("=== wifi no ===");
        sleep(TASK_INIT_DELAY);
        SSD1306_CLS(); // 清屏
    }
    printf("nfc connect wifi is SUCCESS\r\n");
    oled_consle_log("===wifi  yes===");
    sleep(TASK_INIT_DELAY);
    return 0;
} */
int mqtt_init(void)
{
    // 连接Wifi
    if (WiFi_connectHotspots("ken", "qwertyuiop") != WIFI_SUCCESS) {
        printf("[error] WiFi_connectHotspots\r\n");
    }
    // 连接MQTT服务器
    while (MQTTClient_connectServer(SERVER_IP_ADDR, SERVER_IP_PORT) != WIFI_SUCCESS) {
        printf("mqttClient_connectServer\r\n");
        oled_consle_log("==mqtt ser no==");
        sleep(TASK_INIT_DELAY);
        SSD1306_CLS(); // 清屏
    }
    printf("mqttClient_connectServer\r\n");
    oled_consle_log("==mqtt ser yes=");
    sleep(TASK_INIT_DELAY);

    // 初始化MQTT客户端
    while (MQTTClient_init(MQTT_CLIENT_ID, MQTT_USER_NAME, MQTT_PASS_WORD) != WIFI_SUCCESS) {
        printf("mqttClient_init\r\n");
        oled_consle_log("==mqtt cli no==");
        sleep(TASK_INIT_DELAY);
        SSD1306_CLS(); // 清屏
    }
    printf("mqttClient_init\r\n");
    oled_consle_log("==mqtt cli yes=");
    sleep(TASK_INIT_DELAY);

    // 订阅MQTT主题
    while (MQTTClient_subscribe(MQTT_TOPIC_SUB_COMMANDS) != 0) {
        printf("mqttClient_subscribe\r\n");
        oled_consle_log("==mqtt sub no==");
        sleep(TASK_INIT_DELAY);
        SSD1306_CLS(); // 清屏
    }
    printf("mqttClient_subscribe\r\n");
    oled_consle_log("=mqtt sub yes==");
    sleep(TASK_INIT_DELAY);
    SSD1306_CLS(); // 清屏

    return 0;
}
static void Grain_guard_demo(void)
{
    //printf("Enter Grain_guard_example()!\r\n");
    p_MQTTClient_sub_callback = &mqttClient_sub_callback;
    SF_peripheral_init();
    /* if (nfc_connect_wifi_init() == -1) {
        return ;
    } */
    mqtt_init();
    // NV值读出
    // result_code = hi_factory_nv_read(NV_ID, &sys_msg_data.nvFlash, sizeof(hi_nv_save_sensor_threshold), 0);
    // if (result_code != HI_ERR_SUCCESS) {
    //     printf("hi_factory_nv_read is Falied! result_code = %d\r\n", result_code);
    // }
    // else {
    //     printf("nv read: humi_upper=%d, humi_lower=%d autoMode=%d\r\n",
    //             sys_msg_data.nvFlash.humi_upper,
    //             sys_msg_data.nvFlash.humi_lower,
    //             sys_msg_data.nvFlash.smartControl_flag);
    // }
    //  创建线程
    osThreadAttr_t options;

    options.name = "mqtt_send_task";
    options.attr_bits = 0;
    options.cb_mem = NULL;
    options.cb_size = 0;
    options.stack_mem = NULL;//栈的大小
    options.stack_size = TASK2_STACK_SIZE;
    options.priority = osPriorityNormal;//指定线程的优先级 osPriorityNormal 
    mqtt_send_task_id = osThreadNew((osThreadFunc_t)mqtt_send_task, NULL, &options);
    if (mqtt_send_task_id != NULL) {
        printf("ID = %d, Create mqtt_send_task_id is OK!\r\n", mqtt_send_task_id);
    } 

    options.name = "mqtt_recv_task";
    options.attr_bits = 0;
    options.cb_mem = NULL;
    options.cb_size = 0;
    options.stack_mem = NULL;//栈的大小
    options.stack_size = TASK2_STACK_SIZE;
    options.priority = osPriorityNormal;//指定线程的优先级 osPriorityNormal
    mqtt_recv_task_id = osThreadNew((osThreadFunc_t)mqtt_recv_task, NULL, &options);
    if (mqtt_recv_task_id != NULL) {
        printf("ID = %d, Create mqtt_recv_task_id is OK!\r\n", mqtt_recv_task_id);
    }

    options.name = "sensor_collect_task";
    options.attr_bits = 0;
    options.cb_mem = NULL;
    options.cb_size = 0;
    options.stack_mem = NULL;//栈的大小
    options.stack_size = TASK1_STACK_SIZE;
    options.priority = osPriorityNormal;//指定线程的优先级 osPriorityNormal
    sensor_collect_task_id = osThreadNew((osThreadFunc_t)sensor_collect_task, NULL, &options);
    if (sensor_collect_task_id != NULL) {
        printf("ID = %d, Create sensor_collect_task_id is OK!\r\n", sensor_collect_task_id);
    }

    options.name = "show_page";
    options.attr_bits = 0;
    options.cb_mem = NULL;
    options.cb_size = 0;
    options.stack_mem = NULL;//栈的大小
    options.stack_size = TASK3_STACK_SIZE;
    options.priority = osPriorityBelowNormal;//指定线程的优先级 osPriorityNormal
    show_page_id = osThreadNew((osThreadFunc_t)show_page, NULL, &options);
    if (show_page_id != NULL) {
        printf("ID = %d, Create show_page_id is OK!\r\n", show_page_id);
    }
}

SYS_RUN(Grain_guard_demo);
