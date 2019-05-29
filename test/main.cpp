#define CATCH_CONFIG_MAIN
#include <bitset>
#include <iostream>

#include <catch.hpp>
#include <mock_bus.hpp>

#include <mock_adc.hpp>
#include <module.hpp>

TEST_CASE("Test if Mock_adc_c works") {
    // This mock_adc will return the given mock_adc_value when calling the read() function.
    auto mock_adc_pin = r2d2::power::mock_adc_c(0);
    
    // 3150 * 4 == 12600. 12600 is the maximum voltage (12.6v) of the lipo.
    mock_adc_pin.set_return_value(3150);
    REQUIRE(mock_adc_pin.read() == 3150);
    
    // 2700 * 4 == 10800. 10800 is the minimum voltage (10.8v) of the lipo.
    mock_adc_pin.set_return_value(2700);
    REQUIRE(mock_adc_pin.read() == 2700);
}

TEST_CASE("Test if get_battery_voltage works") {
    // Battery definitions
    // Minimum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t min_voltage = 10800; 
    // Maximum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t max_voltage = 12600;

    // Comm
    r2d2::mock_comm_c comm;

    // This mock_adc will return the given mock_adc_value when calling the
    // read() function.
    auto mock_adc_pin = r2d2::power::mock_adc_c(0);

    // Instances
    r2d2::power::battery_level_c level_meter(mock_adc_pin, min_voltage, max_voltage);

    // 3150 * 4 == 12600. 12600 is the maximum voltage (12.6v) of the lipo.
    mock_adc_pin.set_return_value(3150);
    // value has to be the same as given to the mock value * 4.
    REQUIRE(int(level_meter.get_battery_voltage()) == 3150 * 4); 

    // (3150 + 2700) * 2 == 11700. 11700 is half the voltage (11.7v) of the lipo.
    mock_adc_pin.set_return_value((3150 + 2700)/2);
    // value has to be the same as given to the mock value * 4.
    REQUIRE(int(level_meter.get_battery_voltage()) == 2925 * 4); 

    // 2700 * 4 == 10800. 10800 is the minimum voltage (10.8v) of the lipo.
    mock_adc_pin.set_return_value(2700);
    // value has to be the same as given to the mock value * 4.
    REQUIRE(int(level_meter.get_battery_voltage()) == 2700 * 4); 
}

TEST_CASE("Test if get_battery_percentage works") {
    // Battery definitions
    // Minimum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t min_voltage = 10800; 
    // Maximum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t max_voltage = 12600;

    // Comm
    r2d2::mock_comm_c comm;

    // initialize later This mock_adc will return the given mock_adc_value when calling
    // the read() function.
    auto mock_adc_pin = r2d2::power::mock_adc_c(0);

    // Instances
    r2d2::power::battery_level_c level_meter(mock_adc_pin, min_voltage,  max_voltage);

    // 3150 * 4 == 12600. 12600 is the maximum voltage (12.6v) of the lipo.
    mock_adc_pin.set_return_value(3150);
    REQUIRE(int(level_meter.get_battery_percentage()) == 100); // 100%

    // (3150 + 2700) * 2 == 11700. 11700 is half the voltage (11.7v) of the lipo.
    mock_adc_pin.set_return_value((3150 + 2700) / 2);
    REQUIRE(int(level_meter.get_battery_percentage()) == 50); // 50%

    // 2700 * 4 == 10800. 10800 is the minimum voltage (10.8v) of the lipo.
    mock_adc_pin.set_return_value(2700);
    REQUIRE(int(level_meter.get_battery_percentage()) == 0); // 0%
}

int calc_adc_value(uint_fast32_t min_voltage, uint_fast32_t max_voltage, uint8_t percentage) {
    return static_cast<int>((min_voltage + (((max_voltage - min_voltage) / 100) * percentage)) / 4);
};

TEST_CASE("Test calc_adc_value"){
    // Battery definitions
    // Minimum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t min_voltage = 10800; 
    // Maximum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t max_voltage = 12600;

    REQUIRE(calc_adc_value(min_voltage, max_voltage, 100) == max_voltage/4);
    REQUIRE(calc_adc_value(min_voltage, max_voltage, 0) == min_voltage/4);
    REQUIRE(calc_adc_value(min_voltage, max_voltage, 50) == 2925);
    REQUIRE(calc_adc_value(min_voltage, max_voltage, 18) == 2781);
    REQUIRE(calc_adc_value(min_voltage, max_voltage, 10) == 2745);
};

TEST_CASE("Test if there are NO batterie warnings send") {
    // Battery definitions
    // Minimum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t min_voltage = 10800; 
    // Maximum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t max_voltage = 12600; 
    constexpr uint_fast8_t warning_percentage = 20;

    // Comm
    r2d2::mock_comm_c comm;

    uint8_t mock_adc_percentage = 100; // 100%

    // This mock_adc will return the given mock_adc_value when calling the
    // read() function.
    auto mock_adc_pin = r2d2::power::mock_adc_c(calc_adc_value(min_voltage, max_voltage, mock_adc_percentage));

    // Instances
    r2d2::power::battery_level_c level_meter(mock_adc_pin, min_voltage, max_voltage);
    r2d2::power::module_c module(comm, level_meter, warning_percentage);

    // When the module didn't run there should be no frames in the
    // mock_can.
    REQUIRE(comm.get_send_frames().size() == 0); 

    // Measure the status of the batteries once.
    module.process();
    // When the batterie is fully charged && there are no request
    // from other modules the program should not send any frames.
    REQUIRE(comm.get_send_frames().size() == 0); 

    // Changing the percentage of the accu
    mock_adc_percentage = 21; // 21%
    mock_adc_pin.set_return_value(calc_adc_value(min_voltage, max_voltage, mock_adc_percentage));
    // Measure the status of the batteries once.
    module.process();
    // When the percentage hasn't dropped under 20% there should
    // still not be any packets send to the mock_can.
    REQUIRE(comm.get_send_frames().size() == 0); 
};

TEST_CASE("Test if there ARE batterie warnings send") {
    // Battery definitions
    // Minimum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t min_voltage = 10800; 
    // Maximum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t max_voltage = 12600; 
    constexpr uint_fast8_t warning_percentage = 20;

    // Comm
    r2d2::mock_comm_c comm;

    uint8_t mock_adc_percentage = 100; // 100%

    // This mock_adc will return the given mock_adc_value when calling the
    // read() function.
    auto mock_adc_pin = r2d2::power::mock_adc_c(calc_adc_value(min_voltage, max_voltage, mock_adc_percentage));

    // Instances
    r2d2::power::battery_level_c level_meter(mock_adc_pin, min_voltage, max_voltage);
    r2d2::power::module_c module(comm, level_meter, warning_percentage);

    // Changing the percentage of the accu
    mock_adc_percentage = 19; // 19%
    mock_adc_pin.set_return_value(calc_adc_value(min_voltage, max_voltage, mock_adc_percentage));
    // Measure the status of the batteries once.
    module.process();
    // When the percentage dropped under 20% there has to be a
    // packet send to the mock_can.
    REQUIRE(comm.get_send_frames().size() == 1); 


    // Changing the percentage of the accu
    mock_adc_percentage = 16; // 15%
    mock_adc_pin.set_return_value(calc_adc_value(min_voltage, max_voltage, mock_adc_percentage));
    // Measure the status of the batteries once.
    module.process();
    // When the percentage drops again but not under the next
    // threshold (under 15% now) there should not be a new packet
    // send to the mock_can.
    REQUIRE(comm.get_send_frames().size() == 1); 


    /**
     * In the next Requirement occures an error that we didn't figured out yet.
     * We think it has to do with the module.process() function or maybe the mock_can?
     * But because of lack of time we didn't solve the problem.
     */

    // Changing the percentage of the accu
    mock_adc_percentage = 14; // 14%
    mock_adc_pin.set_return_value(calc_adc_value(min_voltage, max_voltage, mock_adc_percentage));
    // Measure the status of the batteries once.
    module.process();
    // When the percentage drops again lower than 15 (14% < 15%),
    // there should be another packet send to the mock_can.
    REQUIRE(comm.get_send_frames().size() == 2); 
}