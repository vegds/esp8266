#define BLINKER_PRINT Serial
#define BLINKER_MIOT_LIGHT
#define BLINKER_WIFI
#include <Blinker.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include "DHT20.h"
#include <Ticker.h>
#include <LittleFS.h> //! 用于保存key

String file_name = "/auth.txt"; // 被读取的文件位置和名称

DHT20 DHT;
uint8_t SERIAL_DATA_PIN = D4;
uint8_t SERIAL_CLOCK_PIN = D3;
float humid = 0;
float temp = 0;
char *weatherText ="unknow";

// 心知天气HTTP请求所需信息
String apikey = "Sq39pxuSJkyEM4XoT";   // 私钥

String url = "v3/weather/now.json?key="+apikey+"location=Hezhou&language=zh-Hans&unit=c";

//* 点灯科技key
char auth[13] = "000000000000";

//* 时间
unsigned long startTime;
unsigned long previousMillis = 0;
unsigned long interval = 10;
time_t times;

uint8_t progress = 4; //! 初始时进度条的值

//*继电器引脚和电平
const int relayPin = D6;
bool relayState = HIGH;

//*按钮引脚
const int buttonPin = D7;

//* NeoPixel配置
#define PIN 15
#define NUMPIXELS 17
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//*blinker定义组件/
BlinkerRGB Color("Color");
BlinkerSlider Slider1("bright");
BlinkerButton mode("power");

uint32_t colors[] = {pixels.Color(255, 0, 0), pixels.Color(0, 255, 0), pixels.Color(0, 0, 255)};

//*默认rgb
int LED_R = 0, LED_G = 0, LED_B = 0;
char *hex_color = "#000000";

int page = 0;
bool change = false;

//* wifimanger配置
WiFiManager wifiManager1;
WiFiManagerParameter custom_auth("auth", "点灯科技key", auth, 13);
bool WIFI_Status;

//* SSD1306屏幕
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, D1, D2, /* reset=*/U8X8_PIN_NONE);

void initDHT20()
{
  Wire.begin(SERIAL_DATA_PIN,SERIAL_CLOCK_PIN);
  DHT.begin();
}
void getDHT20()
{
  int code = DHT.read();
  if (code == DHT20_OK)
  {
    humid = DHT.getHumidity();
    Serial.print(humid);
    temp = DHT.getTemperature();
    Serial.print(temp);
  }
  else
  {
    switch (code)
    {
    case DHT20_OK:
      Serial.print("OK");
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
  }
}

void parseInfo(WiFiClient client)
{
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(7) + 70;
  StaticJsonDocument<capacity> doc;
  // 解析JSON数据
  deserializeJson(doc, client);
  // 获取需要的数据
  // *status = doc["status"];
  // *location = doc["results"][0]["location"]["name"];
  // *weatherText = doc["results"][0]["now"]["text"];
  // temp_get = doc["results"][0]["now"]["temperature"];
}
// 向心知天气服务器服务器请求信息并对信息进行解析
void getWeather()
{
  // WiFiClientSecure client;
  //   if (!client.connect("api.seniverse.com", 443)) {
  //     Serial.println("Connection failed!");
  //     return;
  //   }
  //   String url = String("GET ") + url + " HTTP/1.1\r\n" +
  //                "Host: " + "api.seniverse.com" + "\r\n" +
  //                "Connection: close\r\n\r\n";
  //   client.print(url);
  //  const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 50;
  //   DynamicJsonDocument jsonBuffer(bufferSize);

  //   DeserializationError error = deserializeJson(jsonBuffer, client);

  //   if (error) {
  //     Serial.print("JSON parsing failed: ");
  //     Serial.println(error.c_str());
  //     return;
  //   }

  //   JsonObject root = jsonBuffer.as<JsonObject>();

  //   const char weatherText = root["results"][0]["now"]["text"];
  //   const char temp_get = root["results"][0]["now"]["temperature"];

  //   Serial.print("Weather: ");
  //   Serial.println(weatherText);
  //   Serial.print("Temperature: ");
  //   Serial.println(temp_get);
  }
//* 初始化ws2812
void initws2812()
{
  pixels.begin();
  pixels.show();
}

char *rgbToHex(uint8_t r, uint8_t g, uint8_t b)
{
  char *hex = new char[7];
  sprintf(hex, "#%02X%02X%02X", r, g, b);
  return hex;
}

//* 设置灯的颜色
void SetColor(int R, int G, int B)
{
  change = true;
  hex_color = rgbToHex(R, G, B);
  for (uint16_t i = 0; i < NUMPIXELS; i++) // 把灯条变色
  {
    pixels.setPixelColor(i, R, G, B);
  }
  pixels.show(); // 送出显示
}

//* 设置灯的亮度
void SetBright(int bright)
{
  pixels.setBrightness(bright * 84 + 1);
  pixels.show();
}

void connect()
{
  // 自动连接WiFi。以下语句的参数是连接ESP8266时的WiFi名称
  wifiManager1.setEnableConfigPortal(false);
  wifiManager1.addParameter(&custom_auth);
  // wifiManager1.resetSettings();
  wifiManager1.autoConnect();
}

// 灯的模式回调
void mode_callback(const String &state)
{
  relayState = !relayState;
  // 更新继电器状态
  digitalWrite(relayPin, relayState);
  // 打印继电器状态
  Serial.print("Relay state: ");
  Serial.println(relayState);
}

//* APP RGB颜色设置回调
void rgb1_callback(uint8_t r_value, uint8_t g_value, uint8_t b_value, uint8_t bright)
{
  // digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  BLINKER_LOG("R value: ", r_value);
  BLINKER_LOG("G value: ", g_value);
  BLINKER_LOG("B value: ", b_value);
  SetColor(r_value, g_value, b_value);
}

void slider_callback(int32_t bright)
{
  BLINKER_LOG("bright value: ", bright);
  SetBright(bright);
}

void drawProgressBar(int progress)
{
  int width = u8g2.getWidth() - 2;                // 进度条宽度
  int height = 10;                                // 进度条高度
  int startX = 1;                                 // 进度条起始X坐标
  int startY = u8g2.getHeight() / 2 - height / 2; // 进度条起始Y坐标
  u8g2.drawFrame(startX, startY, width, height);  // 绘制进度条边框
  u8g2.drawUTF8(40, startY + 22, "正在启动");
  u8g2.drawBox(startX + 1, startY + 1, progress * (width - 2) / 100, height - 2); // 绘制进度条填充
  u8g2.drawUTF8(30, startY - 12, "github:vegds");
  u8g2.sendBuffer();
  delay(1000);
}

//* 生成彩虹颜色循环
uint32_t Wheel(::byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void initLFS()
{
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS initialization failed!");
    return;
  }
}

void writeLFS(char text[])
{
  File dataFile = LittleFS.open(file_name, "w"); // 建立File对象用于向SPIFFS中的file对象写入信息
  dataFile.print(text);                          // 向dataFile写入字符串信息
  dataFile.close();                              // 完成文件写入后关闭文件
}

// get string from LFS
String getLFS(const char *fileName)
{
  String data = "";
  //*打开文件
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
  file.close(); //! 关闭文件
  return data;
}
// 启动系统时
void startSystem()
{
  Serial.begin(115200); // 初始化串口
  initws2812();         // 初始化ws2812
  initLFS();            // 初始化LFS
  initDHT20();
  startTime = millis(); // 记录开机时间
  u8g2.begin();
  // u8g2.enableUTF8Print(); // enable UTF8 support for the Arduino print() function
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, OUTPUT);          // 设置继电器为输出模式
  digitalWrite(relayPin, relayState); // 初始继电器状态为关闭
#if defined(BLINKER_PRINT)
  BLINKER_DEBUG.stream(BLINKER_PRINT);
#endif
  strcpy(auth, getLFS(file_name.c_str()).c_str());
  Serial.print(auth);
  Blinker.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str()); // 初始化blinker
}

void allcallback()
{
  mode.attach(mode_callback);
  Slider1.attach(slider_callback);
  Color.attach(rgb1_callback);
}

void setup()
{
  startSystem();
  u8g2.enableUTF8Print();                  // 启用utf-8
  u8g2.setFont(u8g2_font_wqy14_t_gb2312a); // 设置oled的字体
  connect();                               // 连接WiFi
  allcallback();
  setTime(times);
  // ticker.attach(5, getDHT20);
}

void rainbow()
{
  uint16_t i, j;
  for (j = 0; j < 256 * 5; j++)
  { // 5个完整的彩虹循环
    for (i = 0; i < pixels.numPixels(); i++)
    {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
  }
  pixels.show();
}

void saveConfigCallback()
{
  strcpy(auth, custom_auth.getValue());
  writeLFS(auth); // 保存auth
  page = 2;
}

void loop()
{
  Blinker.run();
  bool isrun = false;
  uint8_t wlan = WiFi.status();
  int buttonState = digitalRead(buttonPin); // 读取按钮的状态
  if (buttonState == HIGH)
  { // 如果按钮被按下
    // wifiManager1.resetSettings();
  }
  uint32_t now = millis();
  if (now - DHT.lastRead() > 5000)
  {
    getDHT20();
    //  new request
    DHT.requestData();
    getWeather();
  }
  if (progress < 100)
  {
    if (wlan != WL_CONNECTED)
    {
      progress += 6;
      page = 1;
    }
    else
    {
      progress = 103;
      page = 2;
    }
  }
  if (progress == 100)
  {
    page = 3;
    progress += 6;
  }
  switch (page)
  {
  case 1:
    u8g2.firstPage();
    do
    {
      drawProgressBar(progress); // 显示进度条
    } while (u8g2.nextPage());
    break;
  case 3:
    do
    {
      u8g2.clearBuffer();
      u8g2.sendBuffer();
      page = 4;
      break;
    } while (u8g2.nextPage());
    break;
  case 4:
    if (!isrun)
    {
      wifiManager1.setSaveConfigCallback(saveConfigCallback);
      wifiManager1.startConfigPortal("vegds", "12345678");
      isrun = true;
    }
    break;

  case 2:
    do
    {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 14);
      u8g2.printf("%.1f%%RH %.1f%℃", humid, temp);
      u8g2.drawHLine(0, 48, 128);
      u8g2.setCursor(0, 64);
      u8g2.drawUTF8(0, 26,weatherText);
      time_t rawtime;
      struct tm *info;
      char buffer[80];
      time(&rawtime);
      info = localtime(&rawtime);
      strftime(buffer, 80, "%Y-%m-%d  %H:%M:%S", info);
      u8g2.printf("%s\n", buffer);
      // delete[] buffer;
    } while (u8g2.nextPage());
  }
}
