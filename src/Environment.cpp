/*
 * @Author: vegds
 * @Date: 2023-12-31 20:57:00
 * @LastEditors: vegds
 * @LastEditTime: 2024-02-20 20:19:23
 * @FilePath: \esp8266\src\Environment.cpp
 * @Description:
 *
 * Copyright (c) 2024 by 504136389, All Rights Reserved.
 */
#include "Environment.h"

#include <Wire.h>

#include "DHT20.h"

void Environment::getInfo()
{
    DHT.convert();
    int code = DHT.read(); // get code from read();
    switch (code)
    {
    case DHT20_OK:
        humid = DHT.getHumidity();
        Serial.printf("humid is %f", humid);
        templeture = DHT.getTemperature();
        Serial.printf("temp is %f", templeture);
        break;
    case DHT20_ERROR_CHECKSUM:
        Serial.print("Checksum error");
        break;
    case DHT20_ERROR_CONNECT:
        Serial.print("Connect error");
        break;
    case DHT20_MISSING_BYTES:
        Serial.print("Missing bytes");
        break;
    case DHT20_ERROR_BYTES_ALL_ZERO:
        Serial.print("All bytes read zero");
        break;
    case DHT20_ERROR_READ_TIMEOUT:
        Serial.print("Read time out");
        break;
    case DHT20_ERROR_LASTREAD:
        Serial.print("Error read too fast");
        break;
    default:
        Serial.print("Unknown error");
        break;
    }
    DHT.requestData();
}

/**
 * @brief 开始环境温度和湿度的测量
 * @param DHT DHT对象
 */
void Environment::startMeasure(int DHT_DATA_PIN, int DHT_CLOCK_PIN, DHT20 DHT)
{
    Wire.begin(DHT_DATA_PIN, DHT_CLOCK_PIN); // DHT的引脚
    Wire.setClock(400000);
    DHT.begin();
    getInfo();
}

/**
 * @brief 获取测量的湿度
 * @return 湿度
 */
float Environment::getHumid()
{
    return humid;
}

/**
 * @brief 获取测量的温度
 * @return 温度
 */
float Environment::getTempleture()
{
    return templeture;
}
