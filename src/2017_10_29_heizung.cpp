#include "Arduino.h"

#include <OneWire.h>

#include "myiot_timer_system.h"
#include "myiot_ota.h"
#include "myiot_DeviceConfig.h"
#include "myiot_mqtt.h"
#include "myiot_webServer.h"

#include "Button.h"
#include "DetectError.h"
#include "TemperatureDistribution.h"
#include "StatusLED.h"
#include "zisterne.h"

/* PIN Configuration
ADC    ->   LDR to measure error led of heating system (measure means ON/OFF)
GPIO0  ->   Button for configuration (wifi setup) the device.
GPIO2  ->   blue LED of ESP12F module
GPIO14 ->   OneWire : Dallas Temperature Sensor DS18B20 
*/


enum Constants
{
	LED_STATUS = 2,
	ONEWIRE = 14
};



MyIOT::TimerSystem tsystem;
MyIOT::Mqtt mqtt;
MyIOT::DeviceConfig config;
MyIOT::WebServer webServer;
MyIOT::OTA ota;
StatusLED statusLed;

OneWire oneWire(ONEWIRE);
TemperatureDistribution tdist(oneWire);
Zisterne zisterne;


void setup_web_server()
{
  webServer.setup(config);
  webServer.on("/ex", [](){
    String txt = "<!DOCTYPE html>\r\n";
    txt += "<html><body><form action=\"save\" method=\"GET\">\
            Error Lampe: <INPUT type=\"text\" value=\"" + String(detectError.getValue()) + "\"><br>";

    size_t count = tdist.numberOfSensors();
    txt += "Number of sensors: <INPUT type=\"text\" value=\"" + String(count) + "\"><br>";
    for (size_t idx = 0; idx < count; idx++)
    {
    	char buffer[100] = {0};
    	DeviceAddress da = {0};
    	tdist.get_address(idx, da);

    	sprintf(buffer, "%2X-%2X-%2X-%2X-%2X-%2X-%2X-%2X", da[0], da[1], da[2], da[3], da[4], da[5], da[6], da[7] );
    	txt += "Temperature " + String(idx) + ": <INPUT type=\"text\" value=\"" + tdist.get_temperature(idx, true)
    			+ "\">  " + buffer + " <br>";
    }

    txt += "OneWire::reset returns: " + String(tdist.get_one_wire_reset()) + "<br>";

    txt += "</form></body></html>";
    webServer.send(200, "text/html", txt);
  });
}


void setup_system()
{
	config.setup();
	mqtt.setup(config.getDeviceName(), config.getMqttServer());
	mqtt.setOnConnected([](){mqtt.publish("startup","connected");});
	ota.setup(config.getDeviceName());
	setup_web_server();
	tsystem.add(&ota, MyIOT::TimerSystem::TimeSpec(0,10e6));
	tsystem.add(&mqtt, MyIOT::TimerSystem::TimeSpec(0,100e6));
  tsystem.add(&webServer, MyIOT::TimerSystem::TimeSpec(0,100e6));
}


void setup()
{
	Serial.begin(74880);

    statusLed.setup(LED_STATUS);
    statusLed.setError(StatusLED::Setup);
    setup_system();
    statusLed.setError(StatusLED::Ok);

	auto publish = [](const char* topic, const char* message){mqtt.publish(topic,message);};

	detectError.setup( publish ) ;
	button.setup(publish);
	tdist.setup(publish);
  zisterne.setup(publish);

	tsystem.add([](){detectError.expire();}, MyIOT::TimerSystem::TimeSpec(15));
	tsystem.add([](){button.expire();}, MyIOT::TimerSystem::TimeSpec(1));
	tsystem.add([](){tdist.expire();}, MyIOT::TimerSystem::TimeSpec(60,0));
	tsystem.add(&zisterne, MyIOT::TimerSystem::TimeSpec(5,0));
}

// The loop function is called in an endless loop
void loop()
{
  tsystem.run_loop(1,1);

  if (  WiFi.isConnected())
  {
    statusLed.setError(StatusLED::Ok);
  }
  else
  {
    statusLed.setError(StatusLED::WifiError);
    Serial << "WifiError, millis = " << millis() << "\r\n";
  }
}
