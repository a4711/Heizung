#include <Arduino.h>
#include <myiot_timer_system.h>
#include <Adafruit_INA219.h>


class Zisterne : public MyIOT::ITimer
{
public:
    typedef std::function<void(const char*, const char*)> FPublish;

    void setup(const FPublish& publish);
    void expire() override;
    void destroy() override {}

private:
    void measure();
    void measure2();
    float currentToWaterlevel(float current_ma);

    FPublish ipublish;
    Adafruit_INA219 ina219;
    float average_current_ma {0.0};
    unsigned long publish_reduction {0};
};