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

OneWire oneWire(ONEWIRE);

TemperatureDistribution tdist(oneWire);

class StatusLED
{
public:
	void setup()
	{
		pinMode(LED_STATUS, OUTPUT);
	}
	void expire()
	{
		bool state = digitalRead(LED_STATUS);
		digitalWrite(LED_STATUS, !state);
	}
} statusLed;



void setup()
{
	Serial.begin(115200);

	config.setup();
	Serial.print("DeviceName: ");
	Serial.println(config.getDeviceName());
	Serial.print("MQTT Server: ");
	Serial.println(config.getMqttServer());
    IPAddress localIp = WiFi.localIP();
    Serial.print("localIp: ");
    Serial.println(localIp.toString());

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID().c_str());

	mqtt.setup(config.getDeviceName(), config.getMqttServer());
	mqtt.setOnConnected([](){mqtt.publish("lampe","connected");});

	auto publish = [](const char* topic, const char* message){mqtt.publish(topic,message);};

	ota.setup(config.getDeviceName());
	detectError.setup( publish ) ;
	button.setup(publish);
	webServer.setup(config);
	statusLed.setup();
	tdist.setup(publish);

	tsystem.add(&ota, MyIOT::TimerSystem::TimeSpec(0,10e6));
	tsystem.add(&mqtt, MyIOT::TimerSystem::TimeSpec(0,100e6));
	tsystem.add(&webServer, MyIOT::TimerSystem::TimeSpec(0,100e6));

	tsystem.add([](){detectError.expire();}, MyIOT::TimerSystem::TimeSpec(15));
	tsystem.add([](){button.expire();}, MyIOT::TimerSystem::TimeSpec(1));
	tsystem.add([](){statusLed.expire();}, MyIOT::TimerSystem::TimeSpec(0,500e6));
	tsystem.add([](){tdist.expire();}, MyIOT::TimerSystem::TimeSpec(60,0));
}

// The loop function is called in an endless loop
void loop()
{
  tsystem.run_loop(10,1);
}
