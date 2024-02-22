#define BLINKER_PRINT Serial
#define BLINKER_MIOT_LIGHT
#define BLINKER_WIFI
#include "Application.h"
#include "DHT20.h"
#include "Display.h"
#include "Environment.h"
#include "Light.h"
#include "Storage.h"
#include "Tool.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <Blinker.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <Ticker.h>
#include <U8g2lib.h>
#include <WiFiManager.h>
#define buttonPin D7 // 按钮引脚
#define DHT_DATA_PIN D4
#define DHT_CLOCK_PIN D3
#define relayPin D6 // 继电器引脚
#define PIN 15      // OLED的信号引脚
#define DOT D8

String file_name = "/auth.txt"; // 被读取的文件位置和名称
Storage storage;
Environment env;
Display display;
DHT20 DHT;
Application app;

char auth[13] = "000000000000"; // 点灯科技key

unsigned long startTime;
unsigned long previousMillis = 0;
//  unsigned long interval = 10;
uint8_t progress = 4; // 初始时进度条的值

bool relayState = HIGH; // 继电器电平

bool first = true;

Light light;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(light.NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// uint32_t colors[] = {pixels.Color(255, 0, 0), pixels.Color(0, 255, 0),
// pixels.Color(0, 0, 255)};

int LED_R = 0, LED_G = 0, LED_B = 0; // 开机时oled默认颜色

// char *hex_color = "#000000";

WiFiManager wifiManager; // wifimanger配置

WiFiManagerParameter custom_auth("auth", "点灯科技key", auth, 13); // 密码框控件
WiFiUDP udp;
NTPClient timeClient(udp, "ntp.ntsc.ac.cn",60*60*8,60000);

uint8_t wlan_status;

int now_page = 0;

bool update = false;

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, D1, D2, U8X8_PIN_NONE); // SSD1306屏幕

// 灯的模式回调
void mode_callback(const String &state)
{
    relayState = !relayState;
    // 更新继电器状态
    digitalWrite(relayPin, relayState);

    Serial.print("Relay state: ");
    Serial.println(relayState); // 打印继电器状态
}

/**
 * @brief
 * @param r_value 组件BlinkerRGB Color上R的值
 * @param g_value 组件BlinkerRGB Color上G的值
 * @param b_value 组件BlinkerRGB Color上B的值
 * @param bright 组件BlinkerRGB Color上bright的值
 */
void rgb_callback(uint8_t r_value, uint8_t g_value, uint8_t b_value, uint8_t bright)
{
    BLINKER_LOG("R value: ", r_value);                  // 打印R的值
    BLINKER_LOG("G value: ", g_value);                  // 打印G的值
    BLINKER_LOG("B value: ", b_value);                  // 打印B的值
    (new Light())->SetColor(r_value, g_value, b_value); // 设置灯的颜色
}

void slider_callback(int32_t bright)
{
    BLINKER_LOG("bright value: ", bright);
    (new Light())->SetBright(bright); // 设置灯的亮度
};

BlinkerRGB Color("Color", rgb_callback);
BlinkerSlider Slider("bright", slider_callback);
BlinkerButton mode("power", mode_callback);
BlinkerNumber Temp("temp");
BlinkerNumber Humid("humid");
BlinkerText run("run");

void Connect_wifi()
{
    wifiManager.setEnableConfigPortal(false);
    wifiManager.addParameter(&custom_auth); // 添加控件
    // wifiManager.resetSettings(); //重置WiFi
    wifiManager.autoConnect();
}

void dataRead(const String &data)
{
    if (data != NULL)
    {
        now_page = 1;
    }
    else
    {
        Blinker.print("ERROR");
    }
}

void heartbeat()
{
    Temp.print(env.getTempleture()); // 发送templeture到APP
    Humid.print(env.getHumid());     // 发送humid到APP
}

/**
 * @brief 回调函数
 * @param data 数据
 */

void configData()
{
    Blinker.dataStorage("Temp", env.getTempleture());
    Blinker.dataStorage("Humid", env.getHumid());
}

void setup()
{
    Serial.begin(115200);                               // 初始化串口
    light.initws2812(pixels);                           // 初始化ws2812
    storage.init();                                     // 初始化storage
    env.startMeasure(DHT_DATA_PIN, DHT_CLOCK_PIN, DHT); // 开始测量环境温度
    u8g2.begin();
    u8g2.enableUTF8Print();             // enable UTF8 suppot for the Arduino print() function
    pinMode(relayPin, OUTPUT);          // 设置继电器为输出模式
    digitalWrite(relayPin, relayState); // 初始继电器状态为关闭
    strcpy(auth, storage.get(file_name.c_str()).c_str());
    Blinker.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str()); // 初始化blinker
    Blinker.attachHeartbeat(heartbeat);                           // 注册心跳包
    Blinker.attachData(dataRead); // 注册回调函数，当有设备收到APP发来的数据时会调用对应的回调函数
    Blinker.attachDataStorage(configData, 60, 4); // 注册历史数据回调
    u8g2.setFont(u8g2_font_courB18_tf);           // 设置oled的字体
    Connect_wifi();                               // 连接WiFi
    startTime = millis();                         // 记录开机时间
    timeClient.update();
}

/**
 * @brief wifi密码保存回调
 */
void saveConfigCallback()
{
    strcpy(auth, custom_auth.getValue()); // 复制密码框里的字符串给auth
    storage.write(file_name, auth);       // 保存auth到文件中
    wifiManager.reboot();                 // 重启
}

void loop()
{
    time_t nowtime = millis();
    if (nowtime - previousMillis >= 5000)
    {
        env.getInfo();
        timeClient.update();
        now_page = ((now_page == 2) ? 3 : 2);
        previousMillis = nowtime;
    }
    u8g2.firstPage();
    Blinker.run();

    wlan_status = WiFi.status();                       // 获取WiFi状态
    if (progress < 100 && wlan_status != WL_CONNECTED) // 如果初始化未完成且未联网
    {
        progress = +3;
        now_page = 1;
    }
    else if (progress == 100 && wlan_status != WL_CONNECTED) // 如果初始化完成但未联网
    {
        first = false;
        u8g2.setFont(u8g2_font_8x13_tf);
        u8g2.drawUTF8(0, 64, "Check Internet...");
        u8g2.sendBuffer();
        delay(2000);
        wifiManager.setSaveConfigCallback(saveConfigCallback);
        wifiManager.startConfigPortal("vegds", "12345678");
    }
    else if (wlan_status == WL_CONNECTED && first)
    {
        now_page = 2;
        first = false;
    }
    do
    {
        switch (now_page)
        {
        case 1:
            display.drawProgressBar(u8g2, progress);
            break;
        case 2: {
            u8g2.setFont(u8g2_font_courB18_tf); // 设置oled的字体
            u8g2.clearBuffer();
            u8g2.drawXBMP(0, 0, 18, 18, temp_icon);
            u8g2.drawXBMP(0, 18, 18, 18, humid_icon);
            char tempStr[8];
            sprintf(tempStr, "%.1f°C", env.getTempleture()); // 格式化文本
            u8g2.drawUTF8(18, 18, tempStr);                   // 显示温度
            u8g2.setCursor(18, 36);                           // 移动光标
            u8g2.printf("%.1f%%RH", env.getHumid());         // 显示湿度
            u8g2.drawHLine(0, 48, 128);                      // 划直线
            u8g2.setFont(u8g2_font_9x15_tf);                 // 设置oled的字体
            u8g2.setCursor(0, 64);
            u8g2.printf("ver:%s", version);

            break;
        }
        case 3: {
            u8g2.setFont(u8g2_font_9x15_tf); // 设置oled的字体
            time_t uptime = nowtime - startTime;
            int hours = uptime / 3600000;
            int minutes = (uptime / 60000) % 60;
            char timer[16];
            // u8g2.drawStr(0, 15 * 1, "IP:");
            // u8g2.drawStr(0, 15 * 2, WiFi.localIP().toString().c_str());
            sprintf(timer, "%ih%im", hours, minutes);
            u8g2.drawStr(0, 15 * 1, "Runtime:");
            u8g2.drawStr(8 * 9, 15 * 1, timer);
            u8g2.drawStr(0, 15 * 2, app.hello(timeClient.getHours()).c_str());
            run.print(timer);
            u8g2.drawXBMP(128 - 32, 32, 32, 32, tiyi);
            break;
        }
        default:
            u8g2.sendBuffer();
            break;
        }
    } while (u8g2.nextPage());
}
