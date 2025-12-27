/**
  ******************************************************************************
  * @file           : sast_lib_st7735s.c
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

#include "sast_lib_st7735s.h"
#include "sast_lib_st7735s_font.h" // 导入字模数据
#if ST7735S_USE_PRINTF
#include <stdarg.h> // 用于处理可变参数函数
#endif
#include <stdio.h>
#include <stdlib.h>


#define GPIO_FAST_SETPIN(port, pin)  (port)->BSRR = (pin)
#define GPIO_FAST_RSTPIN(port, pin)  (port)->BSRR = ((pin) << 16U)

/**
 * @brief 私有函数，spi底层实现
 * @param data 要发送的一个字节
 */
static void st7735s_write(uint8_t data)
{
#if ST7735S_USE_HARDWARE_SPI
  HAL_SPI_Transmit(&ST7735S_SPI_HANDLE, &data, 1, HAL_MAX_DELAY);
#else
  // 软件SPI模拟
  for (uint8_t i = 0; i < 8; i++)
  {
    GPIO_FAST_SETPIN(ST7735S_LCD_SCK_GPIO_Port, ST7735S_LCD_SCK_Pin);
    if (data & 0x80)
    {
      GPIO_FAST_SETPIN(ST7735S_LCD_SDA_GPIO_Port, ST7735S_LCD_SDA_Pin);
    }
    else
    {
      GPIO_FAST_RSTPIN(ST7735S_LCD_SDA_GPIO_Port, ST7735S_LCD_SDA_Pin);
    }
    data <<= 1;
    GPIO_FAST_SETPIN(ST7735S_LCD_SCK_GPIO_Port, ST7735S_LCD_SCK_Pin);
  }
#endif
}

/**
 * @brief 私有函数，发送一个字节命令
 * @param cmd 要发送的命令
 */
static void st7735s_write_command(uint8_t cmd)
{
  GPIO_FAST_RSTPIN(ST7735S_LCD_DC_GPIO_Port, ST7735S_LCD_DC_Pin); // DC = 0
#if ST7735S_USE_CS_PIN
  GPIO_FAST_RSTPIN(ST7735S_LCD_CS_GPIO_Port, ST7735S_LCD_CS_Pin); // CS = 0
#endif
  st7735s_write(cmd);
#if ST7735S_USE_CS_PIN
  GPIO_FAST_SETPIN(ST7735S_LCD_CS_GPIO_Port, ST7735S_LCD_CS_Pin); // CS = 1
#endif
}

/**
 * @brief 私有函数，写一个字节数据
 * @param data 要写的一个字节
 */
static void st7735s_write_data(uint8_t data)
{
  GPIO_FAST_SETPIN(ST7735S_LCD_DC_GPIO_Port, ST7735S_LCD_DC_Pin);   // DC = 1
#if ST7735S_USE_CS_PIN
  GPIO_FAST_RSTPIN(ST7735S_LCD_CS_GPIO_Port, ST7735S_LCD_CS_Pin); // CS = 0
#endif
  st7735s_write(data);
#if ST7735S_USE_CS_PIN
  GPIO_FAST_SETPIN(ST7735S_LCD_CS_GPIO_Port, ST7735S_LCD_CS_Pin); // CS = 1
#endif
}

/**
 * @brief 私有函数，写两个字节数据
 * @param data 要写的两个字节
 */
static void st7735s_write_data16(uint16_t data)
{
  st7735s_write_data(data >> 8);
  st7735s_write_data(data & 0xFF);
}

/**
 * @brief 私有函数，复位模块
 */
#if ST7735S_USE_RES_PIN
static void st7735s_reset(void)
{
  GPIO_FAST_RSTPIN(ST7735S_LCD_RES_GPIO_Port, ST7735S_LCD_RES_Pin);
  HAL_Delay(120);
  GPIO_FAST_SETPIN(ST7735S_LCD_RES_GPIO_Port, ST7735S_LCD_RES_Pin);
  HAL_Delay(120);
}
#endif

/**
 * @brief 私有函数，设置绘图区域
 */
static void st7735s_set_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  st7735s_write_command(0x2a);//列地址设置
  st7735s_write_data16(x1);
  st7735s_write_data16(x2);
  st7735s_write_command(0x2b);//行地址设置
  st7735s_write_data16(y1);
  st7735s_write_data16(y2);
  st7735s_write_command(0x2c);//储存器写
}

/**
 * @brief 画点函数
 * @param x 点的x坐标
 * @param y 点的y坐标
 * @param color 点的颜色
 */
void st7735s_draw_point(uint16_t x, uint16_t y, st7735s_color_t color)
{
  st7735s_set_address(x, y, x, y);
  st7735s_write_data16(color);
}

/**
 * @brief 矩形填充颜色函数
 * @param x1 左上角的x坐标
 * @param y1 左上角的y坐标
 * @param x2 右下角的x坐标
 * @param y2 右下角的y坐标
 * @param color 点的颜色
 */
void st7735s_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, st7735s_color_t color)
{
  st7735s_set_address(x1, y1, x2, y2);
  for (uint16_t i = y1; i <= y2; i++)
  {
    for (uint16_t j = x1; j <= x2; j++)
    {
      st7735s_write_data16(color);
    }
  }
}

/**
 * @brief 画线段函数
 * @param x1 端点1的x坐标
 * @param y1 端点1的y坐标
 * @param x2 端点2的x坐标
 * @param y2 端点2的y坐标
 * @param color 点的颜色
 */
void st7735s_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, st7735s_color_t color)
{
  int16_t dx = abs(x2 - x1), dy = abs(y2 - y1);
  int16_t sx = (x1 < x2) ? 1 : -1;
  int16_t sy = (y1 < y2) ? 1 : -1;
  int16_t err = dx - dy;

  while(1)
  {
    st7735s_draw_point(x1, y1, color);

    if (x1 == x2 && y1 == y2)
      break;

    int16_t e2 = 2 * err;
    if (e2 > -dy)
    {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y1 += sy;
    }
  }
}

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
void st7735s_draw_char(uint16_t x, uint16_t y, char ch, st7735s_color_t color, st7735s_color_t bgcolor, st7735s_size_t size, st7735s_display_mode_t mode)
{
  uint8_t temp; // 用于存放字模数据
  uint8_t sizex = size/2; // 字符宽度
  uint16_t TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * size;//一个字符所占字节大小

  uint16_t i; // 循环变量
  uint16_t t; // 循环变量

  uint16_t x0 = x; // 光标的x坐标
  uint16_t y0 = y; // 光标的y坐标

  char _ch = ch - ' ';    //得到偏移后的值

  st7735s_set_address(x, y, x + sizex - 1, y + size - 1);  //设置光标位置

  for(i = 0; i < TypefaceNum; i++) // 遍历单个字符对应的所有字节
  {
    if(size == 12)temp = ascii_1206[_ch][i];		     //调用6x12字体
    else if(size == 16)temp = ascii_1608[_ch][i];		 //调用8x16字体
    else if(size == 24)temp = ascii_2412[_ch][i];		 //调用12x24字体
    else return;
    for(t = 0; t < 8; t++)
    {
      if (mode == ST7735S_NON_OVERLAY_MODE) //非叠加模式
      {
        if(temp & (0x01 << t))st7735s_write_data16(color);
        else st7735s_write_data16(bgcolor);
        x0++;
        if((x0 - x) == sizex)
        {
          x0 = x;
          break;
        }
      }
      else//叠加模式
      {
        if(temp & (0x01 << t))st7735s_draw_point(x0, y0, color);//画一个点
        x0++;
        if((x0 - x) == sizex)
        {
          x0 = x;
          y0++;
          break;
        }
      }
    }
  }
}

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
void st7735s_draw_string(uint16_t x, uint16_t y, const char *str, st7735s_color_t color, st7735s_color_t bgcolor, st7735s_size_t size, st7735s_display_mode_t mode)
{
  uint8_t width = 0;

  switch (size)
  {
  case 12:
    width = 6;
    break;
  case 16:
    width = 8;
    break;
  case 24:
    width = 12;
    break;
  default:
    return;
  }

  while (*str)
  {
    if (x + width > ST7735S_W)
    {
      x = 0;
      y += size;
    }
    if (y + size > ST7735S_H)
    {
      break;
    }

    st7735s_draw_char(x, y, *str, color, bgcolor, size, mode);
    x += width;
    str++;
  }
}

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
void st7735s_printf(uint16_t x, uint16_t y, st7735s_color_t color, st7735s_color_t bgcolor, st7735s_size_t size, st7735s_display_mode_t mode, const char *fmt, ...)
{
  char mBuffer[512] = {0};                             // 开辟一个缓存, 并把里面的数据置0
  va_list ap;                                          // 新建一个可变参数列表
  va_start(ap, fmt);                                   // 列表指向第一个可变参数
  vsnprintf(mBuffer, 512, fmt, ap);                    // 把所有参数，按格式，输出到缓存; 参数2用于限制发送的最大字节数，如果达到上限，则只发送上限值-1; 最后1字节自动置'\0'
  va_end(ap);                                          // 清空可变参数列表

  st7735s_draw_string(x, y, mBuffer, color, bgcolor, size, mode);
}
#endif

/**
 * @brief 显示图像
 * @param x 左上角的x坐标
 * @param y 左上角的y坐标
 * @param length 图像的长度
 * @param width 图像的宽度
 * @param pic 图像数组指针
 */
void st7735s_show_pic(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[])
{
	uint16_t i,j;
	uint32_t k=0;
	st7735s_set_address(x,y,x+length-1,y+width-1);
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
			st7735s_write_data(pic[k*2]);
			st7735s_write_data(pic[k*2+1]);
			k++;
		}
	}
}

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
void st7735s_draw_chinese_char(uint16_t x, uint16_t y, const uint8_t *p, st7735s_color_t color, st7735s_color_t bgcolor, uint16_t sizex, uint16_t sizey, st7735s_display_mode_t mode)
{
  uint8_t temp; // 用于存放字模数据
  const uint8_t *p_data = p; // 字模数据指针

  uint16_t TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey; // 一个字符所占字节大小

  uint16_t i; // 循环变量
  uint16_t t; // 循环变量

  uint16_t x0 = x; // 光标的x坐标
  uint16_t y0 = y; // 光标的y坐标

  st7735s_set_address(x, y, x + sizex - 1, y + sizey - 1);  // 设置光标位置

  for(i = 0; i < TypefaceNum; i++) // 遍历单个字符对应的所有字节
  {
    temp = *(p_data + i); // 取出字模数据
    for(t = 0; t < 8; t++)
    {
      if (mode == ST7735S_NON_OVERLAY_MODE) //非叠加模式
      {
        if(temp & (0x01 << t))st7735s_write_data16(color);
        else st7735s_write_data16(bgcolor);
        x0++;
        if((x0 - x) == sizex)
        {
          x0 = x;
          break;
        }
      }
      else//叠加模式
      {
        if(temp & (0x01 << t))st7735s_draw_point(x0, y0, color);//画一个点
        x0++;
        if((x0 - x) == sizex)
        {
          x0 = x;
          y0++;
          break;
        }
      }
    }
  }
}

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
void st7735s_draw_chinese_string(uint16_t x, uint16_t y, const uint8_t *p, uint16_t length, st7735s_color_t color, st7735s_color_t bgcolor, uint16_t sizex, uint16_t sizey, st7735s_display_mode_t mode)
{
  const uint8_t *p_data = p;
  uint16_t i; // 循环变量
  uint16_t x0 = x; // 光标的x坐标
  uint16_t y0 = y; // 光标的y坐标
  uint16_t TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey; // 一个字符所占字节大小

  for(i = 0; i < length; i++) // 遍历每一个中文字符
  {
    if (x0 + sizex > ST7735S_W)
    {
      x0 = 0;
      y0 += sizey;
    }
    if (y0 + sizey > ST7735S_H)
    {
      break;
    }

    st7735s_draw_chinese_char(x0, y0, p_data, color, bgcolor, sizex, sizey, mode);

    x0 += sizex; // 移动到下一个字符位置
    p_data += TypefaceNum; // 移动到下一个中文字符字模数据起始位置
  }
}

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
void st7735s_draw_progress_bar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t percent, st7735s_color_t fg_color, st7735s_color_t bg_color)
{
  if (percent > 100) percent = 100;

  uint16_t fill_w = (w * percent) / 100;

  // 绘制已填充部分
  if (fill_w > 0)
  {
    st7735s_fill_rect(x, y, x + fill_w - 1, y + h - 1, fg_color);
  }

  // 绘制未填充部分
  if (fill_w < w)
  {
    st7735s_fill_rect(x + fill_w, y, x + w - 1, y + h - 1, bg_color);
  }
}

/**
 * @brief 屏幕初始化函数
 */
void st7735s_init(void)
{
#if ST7735S_USE_RES_PIN
  st7735s_reset();
#endif

  st7735s_write_command(0x11); // Sleep out
  HAL_Delay(120);

  //------------------------------------ST7735S Frame Rate-----------------------------------------//
  st7735s_write_command(0xB1);
  st7735s_write_data(0x05);
  st7735s_write_data(0x3C);
  st7735s_write_data(0x3C);
  st7735s_write_command(0xB2);
  st7735s_write_data(0x05);
  st7735s_write_data(0x3C);
  st7735s_write_data(0x3C);
  st7735s_write_command(0xB3);
  st7735s_write_data(0x05);
  st7735s_write_data(0x3C);
  st7735s_write_data(0x3C);
  st7735s_write_data(0x05);
  st7735s_write_data(0x3C);
  st7735s_write_data(0x3C);
  //------------------------------------End ST7735S Frame Rate---------------------------------//
  st7735s_write_command(0xB4); //Dot inversion
  st7735s_write_data(0x03);
  //------------------------------------ST7735S Power Sequence---------------------------------//
  st7735s_write_command(0xC0);
  st7735s_write_data(0x28);
  st7735s_write_data(0x08);
  st7735s_write_data(0x04);
  st7735s_write_command(0xC1);
  st7735s_write_data(0XC0);
  st7735s_write_command(0xC2);
  st7735s_write_data(0x0D);
  st7735s_write_data(0x00);
  st7735s_write_command(0xC3);
  st7735s_write_data(0x8D);
  st7735s_write_data(0x2A);
  st7735s_write_command(0xC4);
  st7735s_write_data(0x8D);
  st7735s_write_data(0xEE);
  //---------------------------------End ST7735S Power Sequence-------------------------------------//
  st7735s_write_command(0xC5); //VCOM
  st7735s_write_data(0x1A);
  st7735s_write_command(0x36); //MX, MY, RGB mode
  if(ST7735S_DIRECTION==0)st7735s_write_data(0x00);
  else if(ST7735S_DIRECTION==1)st7735s_write_data(0xC0);
  else if(ST7735S_DIRECTION==2)st7735s_write_data(0x70);
  else st7735s_write_data(0xA0);
  //------------------------------------ST7735S Gamma Sequence---------------------------------//
  st7735s_write_command(0xE0);
  st7735s_write_data(0x04);
  st7735s_write_data(0x22);
  st7735s_write_data(0x07);
  st7735s_write_data(0x0A);
  st7735s_write_data(0x2E);
  st7735s_write_data(0x30);
  st7735s_write_data(0x25);
  st7735s_write_data(0x2A);
  st7735s_write_data(0x28);
  st7735s_write_data(0x26);
  st7735s_write_data(0x2E);
  st7735s_write_data(0x3A);
  st7735s_write_data(0x00);
  st7735s_write_data(0x01);
  st7735s_write_data(0x03);
  st7735s_write_data(0x13);
  st7735s_write_command(0xE1);
  st7735s_write_data(0x04);
  st7735s_write_data(0x16);
  st7735s_write_data(0x06);
  st7735s_write_data(0x0D);
  st7735s_write_data(0x2D);
  st7735s_write_data(0x26);
  st7735s_write_data(0x23);
  st7735s_write_data(0x27);
  st7735s_write_data(0x27);
  st7735s_write_data(0x25);
  st7735s_write_data(0x2D);
  st7735s_write_data(0x3B);
  st7735s_write_data(0x00);
  st7735s_write_data(0x01);
  st7735s_write_data(0x04);
  st7735s_write_data(0x13);
  //------------------------------------End ST7735S Gamma Sequence-----------------------------//
  st7735s_write_command(0x3A); //65k mode
  st7735s_write_data(0x05);
  st7735s_write_command(0x29); //Display on
}
