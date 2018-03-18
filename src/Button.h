#ifndef Button_H_
#define Button_H_

#include <ESP8266WiFi.h>

class Button
{
  public:
    Button():state(true){}

    enum {PIN=0};

    typedef std::function<void(const char*, const char*)> FPublish;

    void setup(const FPublish& f_publish)
    {
      publish = f_publish;
      pinMode(PIN, INPUT);
    }
  
    void expire()
    {
      bool newState = digitalRead(PIN);
      if (state && !newState)
      {
    	  if (publish)
    	  {
    		  publish("lampe", "reset");
    		  delay(100);
    	  }

          WiFi.disconnect();
          delay(1000);
          ESP.reset();
          delay(1000);
      }
      state = newState;
    }


  private:
    bool state;
    FPublish publish;
} button;

#endif
