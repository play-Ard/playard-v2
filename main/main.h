/*
 * main.h
 *
 *  Created on: 8 Apr 2023
 *      Author: erhan
 */

#ifndef MAIN_H_
#define MAIN_H_

static const char *TAG = "example";

#define T_DISPLAY_S3_LCD_PIXEL_CLOCK_HZ     (10 * 1000 * 1000)
#define T_DISPLAY_S3_LCD_BK_LIGHT_ON_LEVEL  1
#define T_DISPLAY_S3_LCD_BK_LIGHT_OFF_LEVEL 0
#define T_DISPLAY_S3_PIN_NUM_DATA0          39
#define T_DISPLAY_S3_PIN_NUM_DATA1          40
#define T_DISPLAY_S3_PIN_NUM_DATA2          41
#define T_DISPLAY_S3_PIN_NUM_DATA3          42
#define T_DISPLAY_S3_PIN_NUM_DATA4          45
#define T_DISPLAY_S3_PIN_NUM_DATA5          46
#define T_DISPLAY_S3_PIN_NUM_DATA6          47
#define T_DISPLAY_S3_PIN_NUM_DATA7          48
#define T_DISPLAY_S3_PIN_NUM_PCLK           8
#define T_DISPLAY_S3_PIN_NUM_CS             6
#define T_DISPLAY_S3_PIN_NUM_DC             7
#define T_DISPLAY_S3_PIN_NUM_RST            5
#define T_DISPLAY_S3_PIN_NUM_BK_LIGHT       38

#define T_DISPLAY_S3_PIN_RD          	    9

#define T_DISPLAY_S3_PIN_PWR				15
// The pixel number in horizontal and vertical
#define T_DISPLAY_S3_LCD_H_RES              320
#define T_DISPLAY_S3_LCD_V_RES              170
// Bit number used to represent command and parameter
#define T_DISPLAY_S3_LCD_CMD_BITS           8
#define T_DISPLAY_S3_LCD_PARAM_BITS         8

#define T_DISPLAY_S3_LVGL_TICK_PERIOD_MS    2

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2


#endif /* MAIN_H_ */
