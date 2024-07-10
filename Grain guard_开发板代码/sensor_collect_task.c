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
#include <stdint.h>

#include "sensor_collect_task.h"
//#include "hal_bsp_sht20.h"
#include "sensor_uart.h"
#include "hal_bsp_pcf8574.h"
#include "hal_bsp_ssd1306.h"
#include "hal_bsp_ssd1306_bmps.h"
//#include "hal_bsp_key.h"
#include "hal_bsp_mqtt.h"
#include "oled_show_log.h"
#include "sys_config.h"

#include "hi_io.h"
#include "hi_gpio.h"
#include "hi_uart.h"
#include "hi_pwm.h"
#include "stdbool.h"

/*四个传感器传输的地址码*/
enum Sensor{
    PH_Sensor = 0x01,           //Address 0x01
    CO2_Sensor = 0x02,                 //Address 0x02             
    Humidity_Temperature_Sensor = 0x04, //Address 0x04
    Light_Sensor = 0x09
};

msg_data_t sys_msg_data = {0}; // 传感器的数据

/*
margin_t bmp_number_1 = {
    .top = 16 + 8,
    .left = 8,
    .width = 16,
    .hight = 32,
}; // 数字-十位
margin_t bmp_number_2 = {
    .top = 16 + 8,
    .left = 24,
    .width = 16,
    .hight = 32,
};                                                   // 数字-个位
margin_t bmp_dian = {
    .top = 32 + 8,
    .left = 40,
    .width = 16,
    .hight = 16,
};     // 小数点
margin_t bmp_number_3 = {
    .top = 32 + 8,
    .left = 56,
    .width = 8,
    .hight = 16,
}; // 数字-小数
margin_t bmp_danwei = {
    .top = 16 + 8,
    .left = 52,
    .width = 16,
    .hight = 16,
};   // 单位
margin_t bmp_image = {
    .top = 16,
    .left = 72,
    .width = 48,
    .hight = 48,
}; // 图片
*/
#define TASK_DELAY_TIME (1000 * 1000) // us
#define TASK_WATER_DELAY_TIME 5// us
/**
#define COEFFICIENT_10 10
#define COEFFICIENT_100 100
#define COEFFICIENT_1000 1000
*/
/**
 * @brief  显示湿度页面
 * @note
 * @param  val:
 * @retval None
 */
/*
void show_humi_page(float val)
{
    SSD1306_ShowStr(OLED_TEXT16_COLUMN_0, OLED_TEXT16_LINE_0, " Smart Farm", TEXT_SIZE_16);

    int x = (val * COEFFICIENT_100) / COEFFICIENT_1000;
    SSD1306_DrawBMP(bmp_number_1.left, bmp_number_1.top,
                    bmp_number_1.width, bmp_number_1.hight, bmp_16X32_number[x]); // 显示数字的十位

    x = ((int)(val * COEFFICIENT_100)) / COEFFICIENT_100 % COEFFICIENT_10;
    SSD1306_DrawBMP(bmp_number_2.left, bmp_number_2.top,
                    bmp_number_1.width, bmp_number_1.hight, bmp_16X32_number[x]); // 显示数字的个位
    SSD1306_DrawBMP(bmp_dian.left, bmp_dian.top,
                    bmp_dian.width, bmp_dian.hight, bmp_16X16_dian);              // 显示小数点
    SSD1306_DrawBMP(bmp_danwei.left, bmp_danwei.top,
                    bmp_danwei.width, bmp_danwei.hight, bmp_16X16_baifenhao);     // 显示%符号

    x = ((int)(val * COEFFICIENT_100)) / COEFFICIENT_10 % COEFFICIENT_10;
    SSD1306_DrawBMP(bmp_number_3.left, bmp_number_3.top,
                    bmp_number_3.width, bmp_number_3.hight, bmp_8X16_number[x]); // 显示数字的小数位

    // 风扇动态显示
    if (sys_msg_data.fanStatus == 0) {
        SSD1306_DrawBMP(bmp_image.left, bmp_image.top,
                        bmp_image.width, bmp_image.hight, bmp_48X48_fan_gif[0]); // 静态显示
    } else {
        fan_gif_index++;
        if (fan_gif_index > FAN_GIF_INDEX_MAX)
            fan_gif_index = 0;
        SSD1306_DrawBMP(bmp_image.left, bmp_image.top,
                        bmp_image.width, bmp_image.hight, bmp_48X48_fan_gif[fan_gif_index]); // 动态显示
    }
}
*/
/**
 * @brief  传感器采集任务  改Task1
 * @note   用于循环执行传感器数据采集和风扇控制逻辑。
 * * 在每次循环中，采集SHT20传感器的温度和湿度值，显示湿度信息，并根据预设的湿度阈值控制风扇的开关状态。
 * @retval None


/*更改后的传感器任务*/
void pwm_drive(pwm)
{
    // init gpio of LED
    hi_gpio_init(); 
    // set the GPIO_2 multiplexing function to PWM
    hi_io_set_func(7, 5);
    hi_io_set_func(5, 5);
    // init PWM2
    hi_pwm_init(7);
    hi_pwm_init(5);
    hi_pwm_deinit(7);
    hi_pwm_deinit(5);
    hi_pwm_set_clock(PWM_CLK_XTAL);
    hi_pwm_start(5, pwm, 40000);
    hi_pwm_start(7, 0, 40000);
}
void sensor_collect_task(void)
{
    hi_u16 byte1,byte2,byte3;
    while(1)
    {
    GetPH_Data(PH_Sensor, Databuff, 0x02);
    byte1 = Databuff[3]<<8 | Databuff[4];
    byte2 = Databuff[5]<<8 | Databuff[6];
    sys_msg_data.elec = (int)byte1/10;
    sys_msg_data.PH = (float)byte2/100;

    GetNPK_Data(PH_Sensor, Databuff, 0x03);
    byte1 = Databuff[3]<<8 | Databuff[4];
    byte2 = Databuff[5]<<8 | Databuff[6];
    byte3 = Databuff[7]<<8 | Databuff[8];
    sys_msg_data.N = (int)byte1*10;
    sys_msg_data.P = (int)byte2;
    sys_msg_data.K = (int)byte3;

    GetSoilTemp_Humi_Data(PH_Sensor, Databuff, 0x02);
    byte1 = Databuff[3]<<8 | Databuff[4];
    byte2 = Databuff[5]<<8 | Databuff[6];
    sys_msg_data.soil_temp = (int)byte1/100;
    sys_msg_data.soil_humid = (int)byte2/100;

    GetCO2_Data(CO2_Sensor, Databuff, 0x01);
    byte1 = Databuff[3]<<8 | Databuff[4];
    sys_msg_data.CO2 = (int)byte1;

    GetAirTemp_Humi_Data(Humidity_Temperature_Sensor, Databuff, 0x02);
    byte1 = Databuff[3]<<8 | Databuff[4];
    byte2 = Databuff[5]<<8 | Databuff[6];
    sys_msg_data.air_temp = (int)byte1/10;
    sys_msg_data.air_humid = (int)byte2/10;

    GetLightIntens_Data(Light_Sensor, Databuff, 0x02);
    sys_msg_data.Light = Databuff[3]<<24 | Databuff[4]<<16 | Databuff[5]<<8 | Databuff[6];

    printf("electricity:%d\nPH:%.1f\n", sys_msg_data.elec, sys_msg_data.PH);
    printf("N:%d\nP:%d\nK:%d\n", sys_msg_data.N, sys_msg_data.P, sys_msg_data.K);
    printf("soil_temperature:%d\nsoil_humidity:%.d\n", sys_msg_data.soil_temp, sys_msg_data.soil_humid);
    printf("CO2:%d\n", sys_msg_data.CO2);
    printf("air_temperature:%d\nair_humidity:%d\n", sys_msg_data.air_temp, sys_msg_data.air_humid);
    printf("Light Intensity:%d\n", sys_msg_data.Light);
        // 逻辑判断
        if (sys_msg_data.nvFlash.smartControl_flag != 0) {
           // 风扇控制
            if (sys_msg_data.air_temp >= sys_msg_data.nvFlash.airtemp_upper || sys_msg_data.air_humid >= sys_msg_data.nvFlash.airhumid_upper) {
            set_fan(true); // 打开风扇
            sys_msg_data.fanStatus = 1;
            } else if (sys_msg_data.air_humid < sys_msg_data.nvFlash.airhumid_upper || sys_msg_data.air_temp < sys_msg_data.nvFlash.airtemp_upper) {
            set_fan(false); // 关闭风扇
            sys_msg_data.fanStatus = 0;
            } else {
            set_fan(sys_msg_data.fanStatus); // 保持上一状态
            }

            // 水泵控制
            if (sys_msg_data.soil_humid <= sys_msg_data.nvFlash.soilhumid_lower) {
            set_water(true); // 打开水泵
            sys_msg_data.WaterStatus = 1;
            sleep(TASK_WATER_DELAY_TIME);
            set_water(false); // 关闭水泵
            sys_msg_data.WaterStatus = 0;
            } else {
            sys_msg_data.WaterStatus = 0;
            set_water(sys_msg_data.WaterStatus); // 保持上一状态
            }

            // 遮阳控制
            if (sys_msg_data.Light >= sys_msg_data.nvFlash.Light_upper) {
            pwm_drive(50); // 打开遮阳
            sys_msg_data.SunStatus = 1;
            sleep(TASK_WATER_DELAY_TIME);//直至棚子遮蔽
            hi_pwm_stop(5);// 关闭遮阳
            sys_msg_data.SunStatus = 0;
            } else {
            sys_msg_data.SunStatus = 0;
            hi_pwm_stop(5); // 保持上一状态
            }
                 // 灯光控制
            if (sys_msg_data.Light <= sys_msg_data.nvFlash.Light_lower) {
            set_sunshade(true); // 打开灯
            sys_msg_data.bulbStatus = 1;
            } else if (sys_msg_data.Light > sys_msg_data.nvFlash.Light_lower) {
            set_sunshade(false); // 关闭灯
            sys_msg_data.bulbStatus = 0;
            } else {
            set_sunshade(sys_msg_data.bulbStatus); // 保持上一状态
            }
            // 施肥提示
            if (sys_msg_data.N <= sys_msg_data.nvFlash.N_lower || sys_msg_data.P <= sys_msg_data.nvFlash.P_lower || sys_msg_data.K <= sys_msg_data.nvFlash.K_lower) {
            set_buzzer(true); // 打开蜂鸣器提示施肥
            sys_msg_data.ferStatus = 1;
            sleep(TASK_WATER_DELAY_TIME);
            set_buzzer(false); // 关闭蜂鸣器
            sys_msg_data.ferStatus = 0;
            } else {
            sys_msg_data.ferStatus = 0;
            set_buzzer(sys_msg_data.ferStatus); // 保持上一状态
            }
        } else {
        // 手动控制模式
            set_fan(sys_msg_data.fanStatus);
            set_water(sys_msg_data.WaterStatus);
            set_sunshade(sys_msg_data.bulbStatus);
            set_buzzer(sys_msg_data.ferStatus);
           /*  if (sys_msg_data.SunStatus = 1)
            {
                pwm_drive(50);
                sleep(TASK_WATER_DELAY_TIME);//直至棚子遮蔽
                hi_pwm_stop(5);// 关闭遮阳
            } else{
                hi_pwm_stop(5);// 关闭遮阳
            } */
            }

        usleep(TASK_DELAY_TIME);
    }
}
//显示屏显示传感器数据
void show_page(void)
{
    unsigned char Array[20];
    
    while(1)
    {
        memset(Array,0,sizeof(Array));
        sprintf(Array, "%d", sys_msg_data.soil_temp);
        SSD1306_ShowStr(0, 0, "soil_temp:", TEXT_SIZE_8);
        SSD1306_ShowStr(70, 0, Array, TEXT_SIZE_8);

        memset(Array,0,sizeof(Array));
        sprintf(Array, "%d", sys_msg_data.soil_humid);
        SSD1306_ShowStr(0, 1, "soil_humid:", TEXT_SIZE_8);
        SSD1306_ShowStr(70, 1, Array, TEXT_SIZE_8);

        memset(Array,0,sizeof(Array));
        sprintf(Array, "%d", sys_msg_data.elec);
        SSD1306_ShowStr(0, 2, "elec:", TEXT_SIZE_8);
        SSD1306_ShowStr(30, 2, Array, TEXT_SIZE_8);

        memset(Array,0,sizeof(Array));
        sprintf(Array, "%.1f", sys_msg_data.PH);
        SSD1306_ShowStr(0, 3, "PH:", TEXT_SIZE_8);
        SSD1306_ShowStr(30, 3, Array, TEXT_SIZE_8);

        memset(Array,0,sizeof(Array));
        sprintf(Array, "%d", sys_msg_data.N);
        SSD1306_ShowStr(0, 4, "N:", TEXT_SIZE_8);
        SSD1306_ShowStr(30, 4, Array, TEXT_SIZE_8);

        memset(Array,0,sizeof(Array));
        sprintf(Array, "%d", sys_msg_data.P);
        SSD1306_ShowStr(0, 5, "P:", TEXT_SIZE_8);
        SSD1306_ShowStr(30, 5, Array, TEXT_SIZE_8);

        memset(Array,0,sizeof(Array));
        sprintf(Array, "%d", sys_msg_data.K);
        SSD1306_ShowStr(0, 6, "K:", TEXT_SIZE_8);
        SSD1306_ShowStr(30, 6, Array, TEXT_SIZE_8);

        usleep(1000*1000);
    }
}
