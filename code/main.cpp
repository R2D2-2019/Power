#include "hwlib.hpp"


// Voltage * 1000
#define battery_max_voltage 12600
#define battery_empty_voltage 10800

#define voltage_meter_deviation -2500


int main(void) {

    // kill the watchdog
    WDT->WDT_MR = WDT_MR_WDDIS;
    hwlib::wait_ms(1000);

    auto battery_voltage_pin = hwlib::target::pin_adc(hwlib::target::ad_pins::a1);
    int one_percent_voltage = (battery_max_voltage - battery_empty_voltage) / 100;

    int current_voltage = battery_max_voltage;
    int current_percentage = 100;

    for (;;){
        // Multiply by 5 due to the voltage sensor dividing the voltage by 5 so it is readable by the Arduino Analog pin.
        // Officially this should be multiplied by 5, however this gives an inaccurate result. (See https://docs.google.com/document/d/1BwI1GRxFp-6vBoxl7DooKZmc6JwJ7vQcBKDf2PtpRDM/edit?usp=sharing)

        current_voltage = battery_voltage_pin.read() * 4.05;

        if(current_voltage > battery_max_voltage){
            current_voltage = battery_max_voltage;
        }

        // If battery voltage is below empty, battery percentage is 0
        if (current_voltage <= battery_empty_voltage){
            current_percentage = 0;
        }else{
            current_percentage = (current_voltage - battery_empty_voltage) / one_percent_voltage;
        }

        hwlib::cout << "Battery V:" << current_voltage << hwlib::endl;
        hwlib::cout << "Battery percentage:" << current_percentage << hwlib::endl;

        hwlib::wait_ms(250);
    }
}