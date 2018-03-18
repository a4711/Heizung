#ifndef DetectError_H_
#define DetectError_H_

#include <functional>

class DetectError
{
  public:
	virtual ~DetectError(){}
    enum { 
      sensorPin = A0    // select the input pin for the potentiometer
    };

  void setup(std::function<void(const char* topic, const char* message)> fpublish)
  {
    publish = fpublish;
  }
  
  void expire()
  {
    // read the value from the sensor:
    int sensorValue = analogRead(sensorPin);
    if (publish)
    {
       char buffer[256];
       sprintf(buffer,"%d",sensorValue);
       publish("lampe", buffer);
    }
    Serial.print("Light Value: ");
    Serial.println(sensorValue);
  }

 private:
   std::function<void(const char* topic, const char* message)> publish;
  
} detectError;

#endif
