#include "DHT20.h"

class Environment
{
  private:
    float humid;
    float templeture;
    DHT20 DHT;

  public:
    void startMeasure(int DHT_DATA_PIN, int DHT_CLOCK_PIN, DHT20 DHT);
    float getHumid();
    float getTempleture();
    void getInfo();
};
