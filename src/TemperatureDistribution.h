#ifndef TemperatureDistribution_H_
#define TemperatureDistribution_H_

#include <functional>
#include <limits>
#include <stdio.h>

#include <OneWire.h>
#include <DallasTemperature.h>


template <typename T>
Print& operator<<(Print& textOutput, T value)
{
  textOutput.print(value);
  return textOutput;
}

class TemperatureDistribution
{
  public:
    typedef std::function<void(const char*, const char*)> FPublish;

    TemperatureDistribution(OneWire& aOneWire) : oneWire(aOneWire), controller(&aOneWire)
    {
      resetSensors();
    }

    void setup(const FPublish& fpublish)
    {
      ipublish = fpublish;

      Serial << "TemperatureDistribution::setup ... ";
      controller.begin();
      controller.setResolution(TEMP_12_BIT);
      Serial << "Device Count: " << controller.getDeviceCount() << "\n";
      detect();
    }

    float get_temperature() 
    {
      return get_temperature(0);
    }

    float get_temperature(size_t index)
    {
      if (checkSensorIndex(index))
      {
        for (int i = 0; i < 3; i++) // try three times
        {
         controller.requestTemperaturesByAddress(sensors[index]);
         float retVal = controller.getTempC(sensors[index]);
         if (85.0 != retVal) return retVal; // initial read return 85 degree.
        }
      }
      return 0.0;
    }

    void publish(size_t index, float temperature) const
    {
      if (ipublish)
      {
        String topic("temperature");
        topic += index;
        String message(temperature);
        ipublish(topic.c_str(),message.c_str());
      }
    }

    void dump(size_t index, float temperature) const
    {
      Serial << "published : temperature" << index << " : " << temperature << "\n";
    }

    void expire()
    {
      for (size_t index = 0; !isIndexFree(index); index++)
      {
         float temperature = get_temperature(index);
         publish(index, temperature);
         dump(index, temperature);
      }
    }

  private:

    void detect()
    {
      resetSensors();
      DeviceAddress addr;
      oneWire.reset_search();
      while (oneWire.search(addr))
      {
        Serial.println("");
        if (0x28 == addr[0])
        {
          addSensorAddress(addr);
        }
      }
    }

    size_t addSensorAddress(const DeviceAddress& deviceAddress)
    {
      size_t index = findFreeIndex();
      if (isIndexInRange(index))
      {
        ::memcpy(sensors[index], deviceAddress, sizeof(DeviceAddress));
        Serial << "added temperature sensor: ";
        printAddress(deviceAddress);
        Serial << "\n";
      }
      else
      {
        Serial << "Failed to add sensor: ";
        printAddress(deviceAddress);
        Serial << " :  not enough space in array\n";
      }
      return index;
    }

    void printAddress(const DeviceAddress& deviceAddress) const
    {
      for (uint8_t i = 0; i < 8; i++)
      {
        // zero pad the address if necessary
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
      }
    }

    void resetSensors()
    {
      ::memset(sensors, 0, sizeof(sensors));
    }
    constexpr static size_t maxNumberOfSensors()
    {
      return sizeof(sensors)/sizeof(DeviceAddress);
    }
    
    size_t numberOfSensors() const
    {
      size_t index = 0;
      for (; isIndexInRange(index); index++)
      {
        if (isIndexFree(index))
        {
          break;
        }
      }
      return index;
    }

    bool checkSensorIndex(size_t index) const
    {
      if (!isIndexInRange(index))
      {
        Serial << "checkSensorIndex: index " << index << " is out of rang\n";
        return false;
      }
      if (isIndexFree(index))
      {
        Serial << "checkSensorIndex: index " << index << " is empty\n";
        return false;
      }
      return true;
    }


    bool isIndexInRange(size_t index) const
    {
      return index < maxNumberOfSensors();
    }

    bool isIndexFree(size_t index) const
    {
      return isIndexInRange(index) && 0 == sensors[index][0];
    }

    size_t findFreeIndex() const
    {
      for (size_t index = 0; isIndexInRange(index); index ++)
      {
        if (isIndexFree(index))
        {
          return index;
        }
      }
      return std::numeric_limits<size_t>::max();
    }

    FPublish ipublish;
    DeviceAddress sensors[5];
    OneWire& oneWire;
    DallasTemperature controller;
};

#endif



