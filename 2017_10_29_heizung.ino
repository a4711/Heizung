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
	mqtt.setOnConnected([](){mqtt.publish("lampe","connected");});
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

	tsystem.add([](){detectError.expire();}, MyIOT::TimerSystem::TimeSpec(15));
	tsystem.add([](){button.expire();}, MyIOT::TimerSystem::TimeSpec(1));
	tsystem.add([](){tdist.expire();}, MyIOT::TimerSystem::TimeSpec(60,0));
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
