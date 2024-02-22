/*
 * @Author: vegds
 * @Date: 2024-02-16 21:03:08
 * @LastEditors: vegds
 * @LastEditTime: 2024-02-21 22:53:18
 * @FilePath: \esp8266\src\Application.cpp
 * @Description:
 *
 *
 * Copyright (c) 2024 by 504136389, All Rights Reserved.
 */

#include "Application.h"
#include <time.h>
String Application::hello(int8_t hour)
{
    if (hour > 6 && hour < 12)
    {
        helloword = "A new day!";
    }
    else if (hour >= 12 && hour <= 22)
    {
        helloword = "Stay happy.";
    }
    else
    {
        helloword = "Time to rest.";
    }
    return helloword;
}
