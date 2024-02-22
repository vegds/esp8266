/*
 * @Author: vegds
 * @Date: 2024-02-16 21:28:36
 * @LastEditors: vegds
 * @LastEditTime: 2024-02-19 14:42:07
 * @FilePath: \esp8266\src\Light.h
 * @Description:
 *
 * Copyright (c) 2024 by 504136389, All Rights Reserved.
 */
#include <Adafruit_NeoPixel.h>
class Light
{
  private:
    /* data */ Adafruit_NeoPixel ws2812;

  public:
    void SetBright(int bright);
    int NUMPIXELS;
    // void getColor();
    void initws2812(Adafruit_NeoPixel pixels);
    void SetColor(int R, int G, int B);
};
