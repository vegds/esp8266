/*
 * @Author: vegds
 * @Date: 2024-02-16 21:03:19
 * @LastEditors: vegds
 * @LastEditTime: 2024-02-21 23:13:02
 * @FilePath: \esp8266\src\Application.h
 * @Description:
 *
 * Copyright (c) 2024 by 504136389, All Rights Reserved.
 */

#include <WString.h>
#include <ctime>
class Application
{
  private:
  public:
    String hello(int8_t hour);
    String helloword;
};
static const unsigned char PROGMEM version[] = {"1.0.0"};