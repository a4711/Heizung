#include "Arduino.h"

#include <OneWire.h>

#include "src/myiot_timer_system.h"
#include "src/myiot_ota.h"
#include "src/myiot_DeviceConfig.h"
#include "src/myiot_mqtt.h"
#include "src/myiot_webServer.h"

#include "src/Button.h"
#include "src/DetectError.h"
#include "src/TemperatureDistribution.h"
#include "src/StatusLED.h"

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


void setup_web_server()
{
  webServer.setup(config);
  webServer.on("/ex", [](){
    String txt = "<!DOCTYPE html>\r\n";
    txt += "<html><body><form action=\"save\" method=\"GET\">\
            Error Lampe: <INPUT type=\"text\" value=\"" + String(detectError.getValue()) + "\"><br>\
            Temperatur 0: <INPUT type=\"text\" value=\"" + tdist.get_temperature(0) + "\"><br>\
            Temperatur 1: <INPUT type=\"text\" value=\"" + tdist.get_temperature(1) + "\"><br>\
            </form></body></html>";
    webServer.send(200, "text/html", txt);
  });
}


void setup()
{
	Serial.begin(115200);

  statusLed.setup(LED_STATUS);
  statusLed.setError(StatusLED::Setup);

	config.setup();
  statusLed.setError(StatusLED::Ok);

	mqtt.setup(config.getDeviceName(), config.getMqttServer());
	mqtt.setOnConnected([](){mqtt.publish("lampe","connected");});

	auto publish = [](const char* topic, const char* message){mqtt.publish(topic,message);};

	ota.setup(config.getDeviceName());
	detectError.setup( publish ) ;
	button.setup(publish);
	setup_web_server();
	tdist.setup(publish);


	tsystem.add(&ota, MyIOT::TimerSystem::TimeSpec(0,10e6));
	tsystem.add(&mqtt, MyIOT::TimerSystem::TimeSpec(0,100e6));
  tsystem.add(&webServer, MyIOT::TimerSystem::TimeSpec(0,100e6));

	tsystem.add([](){detectError.expire();}, MyIOT::TimerSystem::TimeSpec(15));
	tsystem.add([](){button.expire();}, MyIOT::TimerSystem::TimeSpec(1));
	tsystem.add([](){tdist.expire();}, MyIOT::TimerSystem::TimeSpec(60,0));
}

// The loop function is called in an endless loop
void loop()
{
  tsystem.run_loop(10,1);

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
