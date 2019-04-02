#include "hwlib.hpp"


// Voltage * 1000
#define battery_max_voltage 12600
#define battery_empty_voltage 10800

#define voltage_meter_deviation -2500


int main(void) {

    // kill the watchdog
    WDT->WDT_MR = WDT_MR_WDDIS;
    hwlib::wait_ms(1000);
    namespace target = hwlib::target;

    auto batteryVoltagePin = hwlib::target::pin_adc(target::ad_pins::a1);
    int onePercent = (battery_max_voltage - battery_empty_voltage) / 100;

    int currentVoltage = battery_max_voltage;
    int currentPercentage = 100;

    for (;;){
        // Multiply by 5 due to the voltage sensor dividing the voltage by 5 so it is readable by the Arduino Analog pin.
        currentVoltage = (batteryVoltagePin.read() * 5) + voltage_meter_deviation;

        // If battery voltage is below empty, battery percentage is 0
        if (currentVoltage <= battery_empty_voltage){
            currentPercentage = 0;
        }else{
            currentPercentage = (currentVoltage - battery_empty_voltage) / onePercent;
        }

        hwlib::cout << "Battery V:" << currentVoltage << hwlib::endl;
        hwlib::cout << "Battery percentage:" << currentPercentage << hwlib::endl;

        hwlib::wait_ms(250);
    }
}