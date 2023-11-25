#define BLINKER_PRINT Serial
#define BLINKER_MIOT_LIGHT
#define BLINKER_WIFI
#include <Blinker.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>  

char auth[] = "fe2d715bf9a0"; 
// 获取当前时间
unsigned long startTime;
WiFiManager wifiManager;
uint8_t progress = 0; // 进度条的当前进度
const int buttonPin = D5;
const int relayPin = D6;

bool relayState = LOW;
bool lastButtonState = HIGH;
bool buttonState = HIGH;

//* NeoPixel配置
#define PIN  15              //  DIN PIN (GPIO15, D8)
#define NUMPIXELS 11         // Number of you led
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE); 
/*blinker定义组件*/ 
BlinkerRGB Color("Color");
BlinkerSlider Slider1("bright");
BlinkerButton mode("power");

int LED_R = 0, LED_G = 0, LED_B = 0;  // RGB和亮度
// !static short Bright[3] = { 1,2,3 };
bool WIFI_Status;
char* hex_color;
    
//初始化ws2812
void initws2812(){
  pixels.begin();
  pixels.show();
}

char* rgbToHex(int red, int green, int blue) {
  char hexValue[7];
  sprintf(hexValue, "#%02X%02X%02X", red, green, blue);
  return hexValue;
}

//设置灯的颜色
void SetColor(int R, int G, int B) {
hex_color=rgbToHex(R, G, B);
  for (uint16_t i = 0; i < NUMPIXELS; i++)  //把灯条变色
  {
    pixels.setPixelColor(i, R, G, B);
  }
  pixels.show();                 //送出显示
}

//设置灯的亮度
void SetBright(int bright) { 
  pixels.setBrightness(bright);
  pixels.show();
}

void connect()
{ 
    // 自动连接WiFi。以下语句的参数是连接ESP8266时的WiFi名称
    wifiManager.autoConnect("vegds","12345678");
}

//APP RGB颜色设置回调
void rgb1_callback(uint8_t r_value, uint8_t g_value, uint8_t b_value, uint8_t bright) {

  //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  BLINKER_LOG("R value: ", r_value);
  BLINKER_LOG("G value: ", g_value);
  BLINKER_LOG("B value: ", b_value);
  SetColor(r_value, g_value, b_value);
}

void slider_callback(int32_t bright) {
  BLINKER_LOG("bright value: ", bright);
  SetBright(bright);
}

void drawProgressBar(int progress) {
  int width = u8g2.getWidth() - 2; // 进度条宽度
  int height = 10; // 进度条高度
  int startX = 1; // 进度条起始X坐标
  int startY = u8g2.getHeight() / 2 - height / 2; // 进度条起始Y坐标
  u8g2.drawFrame(startX, startY, width, height); // 绘制进度条边框
  u8g2.drawUTF8(40, startY+22, "正在启动");
  u8g2.drawBox(startX + 1, startY + 1, progress * (width - 2) / 100, height - 2); // 绘制进度条填充
  u8g2.drawUTF8(40, startY-30, "by:vegds");
  u8g2.sendBuffer();
  delay(1000);
}


//灯的模式回调
void mode_callback(const String& state)
{
    relayState = !relayState;
    // 更新继电器状态
    digitalWrite(relayPin, relayState);
    // 打印继电器状态
    Serial.print("Relay state: ");
    Serial.println(relayState);
}

void setup() {
  Serial.begin(115200); // 初始化串口
  initws2812();
  startTime = millis(); // 记录开机时间
  u8g2.begin();
  u8g2.enableUTF8Print(); // enable UTF8 support for the Arduino print() function
  pinMode(buttonPin, INPUT);

  // 设置继电器引脚为输出模式
  pinMode(relayPin, OUTPUT);

  // 初始继电器状态为关闭
  digitalWrite(relayPin, relayState);
#if defined(BLINKER_PRINT)
  BLINKER_DEBUG.stream(BLINKER_PRINT);
#endif
  u8g2.setFont(u8g2_font_wqy14_t_gb2312a); //此处视情况而定（例如程序大小），范例代码使用的是gb2312，即支持全部中文字体的字库。
  connect();//连接WiFi
  Blinker.begin(auth,WiFi.SSID().c_str(),WiFi.psk().c_str());//初始化blinker
  mode.attach(mode_callback);
  Slider1.attach(slider_callback);
  Color.attach(rgb1_callback);
}


void loop(){
  Blinker.run();
  uint8_t wlan=WiFi.status();
  do {
    if(progress<100){ 
      if(wlan!=WL_CONNECTED)   
      progress += 3;
      else
      progress=103;
     drawProgressBar(progress);//显示进度条
    }else if (progress==100){
      wifiManager.startConfigPortal("vegds","12345678");//启动ap

    } else if (progress==103){
      u8g2.clearDisplay();//清除屏幕
      progress=104;
    }else{
    unsigned long currentTime = millis();
    unsigned long uptime = currentTime - startTime; // 计算已开机时间
    unsigned long hours = uptime / 3600000;
    unsigned long minutes = (uptime / 60000) % 60;
    unsigned long seconds = (uptime / 1000) % 60;
    char timeString[9];
    sprintf(timeString, "%02lu:%02lu:%02lu", hours, minutes, seconds); // 格化时间字符串
    // 在屏幕上显示已开机时间
    u8g2.drawUTF8(0, 15, "启动时间:");
    u8g2.setFont(u8g2_font_8x13_tn);
    u8g2.drawStr(60, 15, timeString);
    u8g2.setFont(u8g2_font_wqy14_t_gb2312a);
    u8g2.drawUTF8(0, 30,"状态:");
    if(wlan== WL_CONNECTED){
    u8g2.drawUTF8(39, 30,"正常");
  }else{
    u8g2.drawUTF8(39, 30, "未连接到WiFi");
  }
    u8g2.drawStr(0, 45, hex_color);     
   } 
  }while (u8g2.nextPage());

}
