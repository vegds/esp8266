/*
 * @Author: vegds
 * @Date: 2024-02-07 19:17:24
 * @LastEditors: vegds
 * @LastEditTime: 2024-02-22 16:55:00
 * @FilePath: \esp8266\src\Display.cpp
 * @Description:
 *
 * Copyright (c) 2024 by 504136389, All Rights Reserved.
 */
#include "Display.h"
#include <U8g2lib.h>
/**
 * @brief 绘制进度条
 * @param u8g2 U8G2对象
 * @param progres 当前进度
 */
void Display::drawProgressBar(U8G2 u8g2, int progress)
{
    int width = u8g2.getWidth() - 2;                // 进度条宽度
    int height = 10;                                // 进度条高度
    int startX = 1;                                 // 进度条起始X坐标
    int startY = u8g2.getHeight() / 2 - height / 2; // 进度条起始Y坐标
    u8g2.drawFrame(startX, startY, width, height);  // 绘制进度条边框
    u8g2.drawUTF8(40, startY + 33, "init...");
    u8g2.drawBox(startX + 1, startY + 10, progress * (width - 2) / 100, height - 2); // 绘制进度条填充
    u8g2.drawXBMP(48, 0, 32, 32, github);
}
