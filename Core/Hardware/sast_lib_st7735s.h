#ifndef SAST_LIB_ST7735S_H_
#define SAST_LIB_ST7735S_H_
/**
  ******************************************************************************
  * @file           : sast_lib_st7735s.h
  * @brief          : a lcd lib for ST7735
  ******************************************************************************
  * @attention
  *
  * Change Logs:
  * Date           Author       Notes
  * 2025-07-12      zzy          the first version
  * 2025-11-17      zzy          add chinese and picture function
  *
  ******************************************************************************
  */

#include "stm32f4xx_hal.h"

/**
 * @brief 移植配置修改区
 */
// SPI模式选择：使用硬件 SPI 或 软件 SPI
#define ST7735S_USE_HARDWARE_SPI 1 // 1: 使用硬件SPI, 0: 使用软件SPI

// SPI句柄（用于硬件SPI通信）
#if ST7735S_USE_HARDWARE_SPI
#define ST7735S_SPI_HANDLE hspi1
extern SPI_HandleTypeDef ST7735S_SPI_HANDLE;
#else
#define ST7735S_LCD_SCK_Pin       GPIO_PIN_5 // SCK
#define ST7735S_LCD_SCK_GPIO_Port GPIOA

#define ST7735S_LCD_SDA_Pin       GPIO_PIN_7 // SDA
#define ST7735S_LCD_SDA_GPIO_Port GPIOA
#endif

// 是否使用RST引脚
#define ST7735S_USE_RES_PIN 1

// 是否使用BLK引脚
#define ST7735S_USE_BLK_PIN 0

// 是否使用CS引脚
#define ST7735S_USE_CS_PIN 0

// LCD 分辨率及方向设置
#define ST7735S_DIRECTION 1 // 设置屏幕方向：0/1竖屏 2/3横屏
#if ST7735S_DIRECTION == 0 || ST7735S_DIRECTION == 1
#define ST7735S_W 128
#define ST7735S_H 160
#else
#define ST7735S_W 160
#define ST7735S_H 128
#endif

// LCD 引脚定义
#if ST7735S_USE_CS_PIN
#define ST7735S_LCD_CS_Pin        LCD_CS_Pin // CS
#define ST7735S_LCD_CS_GPIO_Port  LCD_CS_GPIO_Port
#endif

#define ST7735S_LCD_DC_Pin        GPIO_PIN_0 // DC
#define ST7735S_LCD_DC_GPIO_Port  GPIOB

#if ST7735S_USE_RES_PIN
#define ST7735S_LCD_RES_Pin       GPIO_PIN_6 // RST
#define ST7735S_LCD_RES_GPIO_Port GPIOA
#endif

#if ST7735S_USE_BLK_PIN
#define ST7735S_LCD_BL_Pin        LCD_BL_Pin // BL
#define ST7735S_LCD_BL_GPIO_Port  LCD_BL_GPIO_Port
#endif

// 是否使用printf功能
#define ST7735S_USE_PRINTF 1

/**
 * @brief LCD屏幕颜色枚举
 */
typedef enum
{
  ST7735S_WHITE        = 0xFFFF,
  ST7735S_BLACK        = 0x0000,
  ST7735S_BLUE         = 0x001F,
  ST7735S_BRED         = 0XF81F,
  ST7735S_GRED         = 0XFFE0,
  ST7735S_GBLUE        = 0X07FF,
  ST7735S_RED          = 0xF800,
  ST7735S_MAGENTA      = 0xF81F,
  ST7735S_GREEN        = 0x07E0,
  ST7735S_CYAN         = 0x7FFF,
  ST7735S_YELLOW       = 0xFFE0,
  ST7735S_BROWN        = 0XBC40,//棕色
  ST7735S_BRRED        = 0XFC07,//棕红色
  ST7735S_GRAY         = 0X8430,//灰色
  ST7735S_DARKBLUE     = 0X01CF,//深蓝色
  ST7735S_LIGHTBLUE    = 0X7D7C,//浅蓝色
  ST7735S_GRAYBLUE     = 0X5458,//灰蓝色
  ST7735S_LIGHTGREEN   = 0X841F,//浅绿色
  ST7735S_LGRAY        = 0XC618,//浅灰色(PANNEL),窗体背景色
  ST7735S_LGRAYBLUE    = 0XA651,//浅灰蓝色(中间层颜色)
  ST7735S_LBBLUE       = 0X2B12,//浅棕蓝色(选择条目的反色)
} st7735s_color_t;

/**
 * @brief 覆盖模式枚举
 */
typedef enum
{
  ST7735S_NON_OVERLAY_MODE,            //非覆盖
  ST7735S_OVERLAY_MODE                 //覆盖
} st7735s_display_mode_t;

/**
 * @brief 字体大小枚举
 */
typedef enum
{
  ST7735S_SIZE_1206 = 12,
  ST7735S_SIZE_1608 = 16,
  ST7735S_SIZE_2412 = 24,
} st7735s_size_t;

/**
 * @brief 屏幕初始化函数
 */
void st7735s_init(void);

/**
 * @brief 画点函数
 * @param x 点的x坐标
 * @param y 点的y坐标
 * @param color 点的颜色
 */
void st7735s_draw_point(uint16_t x, uint16_t y, st7735s_color_t color);

/**
 * @brief 矩形填充颜色函数
 * @param x1 左上角的x坐标
 * @param y1 左上角的y坐标
 * @param x2 右下角的x坐标
 * @param y2 右下角的y坐标
 * @param color 点的颜色
 */
void st7735s_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, st7735s_color_t color);

/**
 * @brief 画线段函数
 * @param x1 端点1的x坐标
 * @param y1 端点1的y坐标
 * @param x2 端点2的x坐标
 * @param y2 端点2的y坐标
 * @param color 点的颜色
 */
void st7735s_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, st7735s_color_t color);

/**
 * @brief 单个字符函数
 * @param x 左上角的x坐标
 * @param y 左上角的y坐标
 * @param ch 要画的字符
 * @param color 字符的颜色
 * @param bgcolor 背景颜色，非覆盖模式此选项无效
 * @param size 字体大小
 * @param mode 覆盖模式
 */
void st7735s_draw_char(uint16_t x, uint16_t y, char ch, st7735s_color_t color, st7735s_color_t bgcolor, st7735s_size_t size, st7735s_display_mode_t mode);

/**
 * @brief 字符串函数
 * @param x 左上角的x坐标
 * @param y 左上角的y坐标
 * @param str 字符串指针
 * @param color 字符串的颜色
 * @param bgcolor 背景颜色，非覆盖模式此选项无效
 * @param size 字体大小
 * @param mode 覆盖模式
 */
void st7735s_draw_string(uint16_t x, uint16_t y, const char *str, st7735s_color_t color, st7735s_color_t bgcolor, st7735s_size_t size, st7735s_display_mode_t mode);

#if ST7735S_USE_PRINTF
/**
 * @brief 输出格式化字符串函数，支持不定长参数，用法同printf
 * @param x 左上角的x坐标
 * @param y 左上角的y坐标
 * @param color 字符串的颜色
 * @param bgcolor 背景颜色，非覆盖模式此选项无效
 * @param size 字体大小
 * @param mode 覆盖模式
 * @param fmt 格式化字符串指针
 */
void st7735s_printf(uint16_t x, uint16_t y, st7735s_color_t color, st7735s_color_t bgcolor, st7735s_size_t size, st7735s_display_mode_t mode, const char *fmt, ...);
#endif

/**
 * @brief 显示图像
 * @param x 左上角的x坐标
 * @param y 左上角的y坐标
 * @param length 图像的长度
 * @param width 图像的宽度
 * @param pic 图像数组指针
 */
void st7735s_show_pic(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[]);

/**
 * @brief 显示中文字符函数
 * @param x 左上角的x坐标
 * @param y 左上角的y坐标
 * @param p 中文字符取模数据指针
 * @param color 字符的颜色
 * @param bgcolor 背景颜色，非覆盖模式此选项无效
 * @param sizex 字体宽度
 * @param sizey 字体高度
 * @param mode 覆盖模式
 */
void st7735s_draw_chinese_char(uint16_t x, uint16_t y, const uint8_t *p, st7735s_color_t color, st7735s_color_t bgcolor, uint16_t sizex, uint16_t sizey, st7735s_display_mode_t mode);

/**
 * @brief 显示中文字符串函数
 * @param x 左上角的x坐标
 * @param y 左上角的y坐标
 * @param p 中文字符串取模数据指针
 * @param length 中文字符串长度（字符个数）
 * @param color 字符串的颜色
 * @param bgcolor 背景颜色，非覆盖模式此选项无效
 * @param sizex 字体宽度
 * @param sizey 字体高度
 * @param mode 覆盖模式
 */
void st7735s_draw_chinese_string(uint16_t x, uint16_t y, const uint8_t *p, uint16_t length, st7735s_color_t color, st7735s_color_t bgcolor, uint16_t sizex, uint16_t sizey, st7735s_display_mode_t mode);

/**
 * @brief 显示进度条函数
 * @param x 左上角的x坐标
 * @param y 左上角的y坐标
 * @param w 进度条宽度
 * @param h 进度条高度
 * @param percent 进度百分比 (0-100)
 * @param fg_color 前景颜色 (进度条颜色)
 * @param bg_color 背景颜色 (剩余部分颜色)
 */
void st7735s_draw_progress_bar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t percent, st7735s_color_t fg_color, st7735s_color_t bg_color);


#endif /* SAST_LIB_ST7735S_H_ */
