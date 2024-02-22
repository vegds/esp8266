/*
 * @Author: vegds
 * @Date: 2024-02-03 20:35:54
 * @LastEditors: vegds
 * @LastEditTime: 2024-02-07 19:27:40
 * @FilePath: \esp8266-main\src\Storage.cpp
 * @Description:
 *
 * Copyright (c) 2024 by 504136389, All Rights Reserved.
 */
#include "Storage.h"

#include "LittleFS.h"

/**
 * @brief 初始化文件系统
 * @return 初始化结果
 */
boolean Storage::init()
{
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS initialization failed!");
        return false;
    }
    else
    {
        return true;
    }
}
/**
 * @brief 写入文本到文件
 * @param file_name 文件名
 * @param text 要写入的字符串
 */
void Storage::write(String file_name, char text[])
{
    File dataFile = LittleFS.open(
        file_name, "w");  // 建立File对象用于向SPIFFS中的file对象写入信息
    dataFile.print(text); // 向dataFile写入字符串信息
    dataFile.close();     // 完成文件写入后关闭文件
}
/**
 * @brief 从文件读取文本
 * @param fileName 文件名
 * @return 从fileName读取的字符串
 */
String Storage::get(const char *fileName)
{
    String data = "";
    // 打开文件
    File file = LittleFS.open(fileName, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return data;
    }
    while (file.available())
    {
        data += (char)file.read();
    }
    file.close(); // 关闭文件
    return data;
}