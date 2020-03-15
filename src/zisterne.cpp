#include <zisterne.h>


void Zisterne::setup(const FPublish& publish)
{
    ipublish = publish;
    ina219.begin();
}

void Zisterne::expire()
{
    measure();
    measure2();
}

void Zisterne::measure()
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float power_mW = 0;

  unsigned long mstart = ::micros();
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();  // strom messung alleine dauert ca. 2055 us ~= ca 2ms
  power_mW = ina219.getPower_mW();
  unsigned long duration = ::micros() - mstart;
  Serial.printf("duration: %ld\n", duration);
  Serial.printf("sV=%f mV, bV=%f V, I=%f mA, P=%f mW\n", shuntvoltage, busvoltage, current_mA, power_mW);

  char message[256] = {0};
  snprintf(message, sizeof(message)-1, "{\"shuntvoltage\":%f, \"busvoltage\":%f, \"current_mA\":%f, \"power_mW\":%f}"
    , shuntvoltage
    , busvoltage
    , current_mA
    , power_mW
    );
  ipublish("zisterne/ina219", message);    
}

float Zisterne::currentToWaterlevel(float current_ma)
{
  return ( current_ma - 4)*(5.0/16);
}

void Zisterne::measure2()
{
  // (4...20)mA <=> (0...5)m
  float current_mA = ina219.getCurrent_mA();
  if (0.0 == average_current_ma) average_current_ma = current_mA;
  char message[256] = {0};
  if (current_mA > 4)
  {
    // average is computed over 10 samples
    average_current_ma = (9*average_current_ma +current_mA ) / 10;
    float waterlevel = currentToWaterlevel(average_current_ma);
    snprintf(message, sizeof(message)-1, "{\"waterlevel\":%f}", waterlevel);
    ipublish("zisterne", message);    
  }
  else
  {
    snprintf(message, sizeof(message)-1, "{\"current_mA\":%f}", current_mA);
    ipublish("zisterne/ina219_error", message);    
  }
}