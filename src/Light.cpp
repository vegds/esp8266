#include "Light.h"
/**
 * @brief 初始化ws2812
 * @param pixels Adafruit_NeoPixel对象
 */
int NUMPIXELS = 17; // OLED灯个数
void Light::initws2812(Adafruit_NeoPixel pixels)
{
    ws2812 = pixels;
    ws2812.begin();
    ws2812.show();
}

//* 设置灯的颜色
void Light::SetColor(int R, int G, int B)
{
    for (uint16_t i = 0; i < NUMPIXELS; i++) // 把灯条变色
    {
        ws2812.setPixelColor(i, R, G, B);
    }
    ws2812.show(); // 送出显示
}

//* 设置灯的亮度
void Light::SetBright(int bright)
{
    ws2812.setBrightness(bright * 84 + 1);
    ws2812.show();
}

// void Light::getColor(){
//
// }

/**
 * @brief 将RGB颜色转换成十六进制
 * @param r RED
 * @param g GREEN
 * @param b BULE
 * @return 16decimal color code
 */
char *rgbToHex(uint8_t r, uint8_t g, uint8_t b)
{
    char *hex = new char[7];
    sprintf(hex, "#%02X%02X%02X", r, g, b);
    return hex;
}
