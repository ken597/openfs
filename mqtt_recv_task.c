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

#include "mqtt_recv_task.h"
#include "hal_bsp_mqtt.h"
#include "hal_bsp_structAll.h"
#include "hi_nv.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cJSON.h"
#include "cmsis_os2.h"
#include "sys_config.h"

#define MQTT_RECV_TASK_DELAY_TIME (100 * 1000) // us100*1000

int get_jsonData_value(const cJSON *const object, uint8_t *value)
{
    cJSON *json_value = NULL;
    json_value = cJSON_GetObjectItem(object, "value");
    if (json_value) {
        if (!strcmp(json_value->valuestring, "ON")) {
            *value = 1;
            json_value = NULL;
            return 0; // 0为成功
        } else if (!strcmp(json_value->valuestring, "OFF")) {
            *value = 0;
            json_value = NULL;
            return 0;
        }
    }
    json_value = NULL;
    return -1; // -1为失败
}

// 解析JSON数据
uint8_t cJSON_Parse_Payload(uint8_t *payload)
{
    uint8_t ret = 0;
    if (payload == NULL) {
        printf("payload is NULL\r\n");
        return 1;
    }

    cJSON *root = cJSON_Parse((const char *)payload);
    cJSON *json_service_id = cJSON_GetObjectItem(root, "service_id");
    cJSON *json_command_name = cJSON_GetObjectItem(root, "command_name");
    if (root && json_service_id && json_command_name && (!strcmp(json_service_id->valuestring, "control"))) {
        // 接收风扇控制命令 与IOTA设备中保持一致
        if (!strcmp(json_command_name->valuestring, "fan")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            get_jsonData_value(paras, &sys_msg_data.fanStatus);
        }
        // 接收浇水控制命令 与IOTA设备中保持一致
        if (!strcmp(json_command_name->valuestring, "Water")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            get_jsonData_value(paras, &sys_msg_data.WaterStatus);
        }
        // 接收遮阳控制命令 与IOTA设备中保持一致
        if (!strcmp(json_command_name->valuestring, "Sun")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            get_jsonData_value(paras, &sys_msg_data.SunStatus);
        }
                // 接收施肥控制命令 与IOTA设备中保持一致
        if (!strcmp(json_command_name->valuestring, "fer")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            get_jsonData_value(paras, &sys_msg_data.ferStatus);
        }
                        // 接收灯光控制命令 与IOTA设备中保持一致
        if (!strcmp(json_command_name->valuestring, "bulb")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            get_jsonData_value(paras, &sys_msg_data.bulbStatus);
        }
        // 接收自动控制命令
        if (!strcmp(json_command_name->valuestring, "autoMode")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            get_jsonData_value(paras, &sys_msg_data.nvFlash.smartControl_flag);
        }

        // 接收土壤湿度上限值
        if (!strcmp(json_command_name->valuestring, "soil_humid")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "up");
            if (json_down) {
                printf("command_name: soil_humid, up: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.soilhumid_upper = json_down->valueint;
            }
            json_down = NULL;
        }
        // 接收土壤湿度下限值
        if (!strcmp(json_command_name->valuestring, "soil_humid")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "down");
            if (json_down) {
                printf("command_name: soil_humid, down: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.soilhumid_lower = json_down->valueint;
            }
            json_down = NULL;
        }
         // 接收空气湿度上限值
        if (!strcmp(json_command_name->valuestring, "air_humid")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "up");
            if (json_down) {
                printf("command_name: air_humid, up: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.airhumid_upper = json_down->valueint;
            }
            json_down = NULL;
        }
        // 接收空气湿度下限值
        if (!strcmp(json_command_name->valuestring, "air_humid")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "down");
            if (json_down) {
                printf("command_name: air_humid, down: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.airhumid_lower = json_down->valueint;
            }
            json_down = NULL;
        }
        // 接收空气温度上限值
        if (!strcmp(json_command_name->valuestring, "air_temp")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "up");
            if (json_down) {
                printf("command_name: tempurture, up: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.airtemp_upper = json_down->valueint;
            }
            json_down = NULL;
        }
        // 接收空气温度下限值
        if (!strcmp(json_command_name->valuestring, "air_temp")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "down");
            if (json_down) {
                printf("command_name: air_temp, down: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.airtemp_lower = json_down->valueint;
            }
            json_down = NULL;
        }
        // 接收光照强度上限值
        if (!strcmp(json_command_name->valuestring, "Light")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "up");
            if (json_down) {
                printf("command_name: Light, up: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.Light_upper = json_down->valueint;
            }
            json_down = NULL;
        }
        // 接收光照强度下限值
        if (!strcmp(json_command_name->valuestring, "Light")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "down");
            if (json_down) {
                printf("command_name: Light, down: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.Light_lower = json_down->valueint;
            }
            json_down = NULL;
        }
        // 接收氮下限值
        if (!strcmp(json_command_name->valuestring, "N")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "down");
            if (json_down) {
                printf("command_name: N, down: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.N_lower = json_down->valueint;
            }
            json_down = NULL;
        }
        // 接收磷下限值
        if (!strcmp(json_command_name->valuestring, "P")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "down");
            if (json_down) {
                printf("command_name: P, down: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.P_lower = json_down->valueint;
            }
            json_down = NULL;
        }
        // 接收钾下限值
        if (!strcmp(json_command_name->valuestring, "K")) {
            cJSON *paras = cJSON_GetObjectItem(root, "paras");
            cJSON *json_down = cJSON_GetObjectItem(paras, "down");
            if (json_down) {
                printf("command_name: K, down: %d.\r\n", json_down->valueint);
                sys_msg_data.nvFlash.K_lower = json_down->valueint;
            }
            json_down = NULL;
        }
    }

    cJSON_Delete(root);
    json_service_id = NULL;
    json_command_name = NULL;
    root = NULL;

    return 0;
}

// 向云端发送返回值
void send_cloud_request_code(const char *request_id, int ret_code, int request_len)
{
    char *request_topic = (char *)malloc(strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) +
                                            strlen(DEVICE_ID) + request_len + 1);
    if (request_topic != NULL) {
        memset_s(request_topic,
                 strlen(DEVICE_ID) + strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) + request_len + 1,
                 0,
                 strlen(DEVICE_ID) + strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) + request_len + 1);
        if (sprintf_s(request_topic,
                      strlen(DEVICE_ID) + strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) + request_len + 1,
                      MQTT_TOPIC_PUB_COMMANDS_REQ, DEVICE_ID, request_id) > 0) {
            if (ret_code == 0) {
                MQTTClient_pub(request_topic, "{\"result_code\":0}", strlen("{\"result_code\":0}"));
            } else if (ret_code == 1) {
                MQTTClient_pub(request_topic, "{\"result_code\":1}", strlen("{\"result_code\":1}"));
            }
        }
        free(request_topic);
        request_topic = NULL;
    }
}

/**
 * @brief MQTT接收数据的回调函数
 */
int8_t mqttClient_sub_callback(unsigned char *topic, unsigned char *payload)
{
    if ((topic == NULL) || (payload == NULL)) {
        return -1;
    } else {
        printf("topic: %s\r\n", topic);
        printf("payload: %s\r\n", payload);

        // 提取出topic中的request_id
        char request_id[50] = {0};
        int ret_code = 1; // 0为成功, 其余为失败。不带默认表示成功
        if (0 == strcpy_s(request_id, sizeof(request_id),
                          topic + strlen(DEVICE_ID) + strlen("$oc/devices//sys/commands/request_id="))) {
            printf("request_id: %s\r\n", request_id);
            // 解析JSON数据
            ret_code = cJSON_Parse_Payload(payload);
            send_cloud_request_code(request_id, ret_code, sizeof(request_id));
        }
    }
    return 0;
}

/**
 * @brief MQTT  接收消息任务
 */
void mqtt_recv_task(void)
{
    while (1) {
        MQTTClient_sub();
        usleep(MQTT_RECV_TASK_DELAY_TIME);
    }
}
