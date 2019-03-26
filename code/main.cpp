#include "hwlib.hpp"

int main(void) {

    auto batteryVoltagePin = hwlib::target::pin_in(hwlib::target::pins::a1);
    double currentVoltage = 0.00;

  // kill the watchdog
    WDT->WDT_MR = WDT_MR_WDDIS;
    hwlib::wait_ms(1000);
    for (;;){
        // Multiply by 5 due to the voltage sensor dividing the voltage by 5 so it is readable by the Arduino Analog pin.
        currentVoltage = batteryVoltagePin.read() * 5;

        hwlib::cout << "Battery V:" << int(currentVoltage) << hwlib::endl;
        hwlib::wait_ms(250);
    }
}