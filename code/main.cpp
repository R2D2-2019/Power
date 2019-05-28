#include <hwlib.hpp>

#include <comm.hpp>
#include <module.hpp>

int main(void) {
    // kill the watchdog
    WDT->WDT_MR = WDT_MR_WDDIS;

    hwlib::wait_ms(1000);

    // Comm
    r2d2::comm_c comm;

    // Battery definitions
    constexpr uint_fast32_t min_voltage = 10800;
    constexpr uint_fast32_t max_voltage = 12600;
    constexpr uint_fast8_t warning_percentage = 20;

    // The pin the battery level will be measured from.
    auto adc_pin = hwlib::target::pin_adc(hwlib::target::ad_pins::a1);

    // Instances
    r2d2::power::battery_level_c level_meter(adc_pin, min_voltage, max_voltage);
    r2d2::power::module_c module(comm, level_meter, warning_percentage);
    
    for (;;) {
        module.process();
        hwlib::wait_ms(100);
    }
}