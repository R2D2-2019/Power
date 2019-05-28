#define CATCH_CONFIG_MAIN
#include <iostream>
#include <bitset>

#include <catch.hpp>
#include <mock_bus.hpp>

#include <module.hpp>
#include <mock_adc.hpp>

//TODO: change warning percentage to trigger at that percentage

TEST_CASE("Test working Mock_adc") {
    uint16_t mock_adc_value = 0; // initialize later
    // This mock_adc will return the given mock_adc_value when calling the read() function.
    auto mock_adc_pin = r2d2::power::mock_adc_c(mock_adc_value);

    mock_adc_value = 3150; // 3150 * 4 == 12600. 12600 is the maximum voltage (12.6v) of the lipo.
    REQUIRE(mock_adc_pin.read() == mock_adc_value);
    mock_adc_value = 2700; // 2700 * 4 == 10800. 10800 is the minimum voltage (10.8v) of the lipo.
    REQUIRE(mock_adc_pin.read() == mock_adc_value);
}

TEST_CASE("Test get_battery_voltage") {
    // Battery definitions
    constexpr uint_fast32_t min_voltage = 10800; // Minimum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t max_voltage = 12600; // Maximum voltage of the battery. Else we might dammage the lipo.

    // Comm
    r2d2::mock_comm_c comm;

    uint16_t mock_adc_value = 0; // initialize later
    // This mock_adc will return the given mock_adc_value when calling the read() function.
    auto mock_adc_pin = r2d2::power::mock_adc_c(mock_adc_value);

    // Instances
    r2d2::power::battery_level_c level_meter(mock_adc_pin, min_voltage, max_voltage);

    mock_adc_value = 3150; // 3150 * 4 == 12600. 12600 is the maximum voltage (12.6v) of the lipo.
    REQUIRE(int(level_meter.get_battery_voltage()) == mock_adc_value*4); // value has to be the same as given to the mock value * 4

    mock_adc_value = (3150 + 2700) / 2; // (3150 + 2700) * 2 == 11700. 11700 is half the voltage (11.7v) of the lipo.
    REQUIRE(int(level_meter.get_battery_voltage()) == mock_adc_value*4); // value has to be the same as given to the mock value * 4

    mock_adc_value = 2700; // 2700 * 4 == 10800. 10800 is the minimum voltage (10.8v) of the lipo.
    REQUIRE(int(level_meter.get_battery_voltage()) == mock_adc_value*4); // value has to be the same as given to the mock value * 4
}

TEST_CASE("Test get_battery_percentage") {
    // Battery definitions
    constexpr uint_fast32_t min_voltage = 10800; // Minimum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t max_voltage = 12600; // Maximum voltage of the battery. Else we might dammage the lipo.

    // Comm
    r2d2::mock_comm_c comm;

    uint16_t mock_adc_value = 0; // initialize later
    // This mock_adc will return the given mock_adc_value when calling the read() function.
    auto mock_adc_pin = r2d2::power::mock_adc_c(mock_adc_value);

    // Instances
    r2d2::power::battery_level_c level_meter(mock_adc_pin, min_voltage, max_voltage);

    mock_adc_value = 3150; // 3150 * 4 == 12600. 12600 is the maximum voltage (12.6v) of the lipo.
    REQUIRE(int(level_meter.get_battery_percentage()) == 100); // 100%

    mock_adc_value = (3150 + 2700) / 2; // (3150 + 2700) * 2 == 11700. 11700 is half the voltage (11.7v) of the lipo.
    REQUIRE(int(level_meter.get_battery_percentage()) == 50); // 50%

    mock_adc_value = 2700; // 2700 * 4 == 10800. 10800 is the minimum voltage (10.8v) of the lipo.
    REQUIRE(int(level_meter.get_battery_percentage()) == 0); // 0%
}

int calc_adc_value(uint_fast32_t min_voltage, uint_fast32_t max_voltage, uint8_t percentage){
    return static_cast<int>((min_voltage + (((max_voltage - min_voltage) / 100) * percentage)) / 4);
};

TEST_CASE("Warning percentage send") {
    // Battery definitions
    constexpr uint_fast32_t min_voltage = 10800; // Minimum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast32_t max_voltage = 12600; // Maximum voltage of the battery. Else we might dammage the lipo.
    constexpr uint_fast8_t warning_percentage = 20;

    // Comm
    r2d2::mock_comm_c comm;

    uint8_t mock_adc_percentage = 100; //100%
    uint16_t mock_adc_value = calc_adc_value(min_voltage, max_voltage, mock_adc_percentage);

    // This mock_adc will return the given mock_adc_value when calling the read() function.
    auto mock_adc_pin = r2d2::power::mock_adc_c(mock_adc_value);

    // Instances
    r2d2::power::battery_level_c level_meter(mock_adc_pin, min_voltage, max_voltage);
    r2d2::power::module_c module(comm, level_meter, warning_percentage);

    REQUIRE(comm.get_send_frames().size() == 0); // When the module didn't run there should be no frames in the mock_can.

    // Measure the status of the batteries once.
    module.process();
    REQUIRE(comm.get_send_frames().size() == 0); // When the batterie is fully charged && there are no request from 
                                                        // other modules the program should not send any frames.

    // Changing the percentage of the accu
    mock_adc_percentage = 21; //21%
    mock_adc_value = calc_adc_value(min_voltage, max_voltage, mock_adc_percentage);
    // Measure the status of the batteries once.
    module.process();
    REQUIRE(comm.get_send_frames().size() == 0); // When the percentage hasn't dropped under 20% there should still 
                                                        // not be any packets send to the mock_can.


    // Changing the percentage of the accu
    mock_adc_percentage = 19; //19%
    mock_adc_value = calc_adc_value(min_voltage, max_voltage, mock_adc_percentage);
    // Measure the status of the batteries once.
    module.process();
    REQUIRE(comm.get_send_frames().size() == 1); // When the percentage dropped under 20% there has to be a packet send to the mock_can.

    module.process();
    REQUIRE(comm.get_send_frames().size() == 1); // When the percentage is under 20% but hasn't got any lower than before 
        	                                            //there shouldn't be more packets sent.

    // Changing the percentage of the accu
    mock_adc_percentage = 15; //15%
    mock_adc_value = calc_adc_value(min_voltage, max_voltage, mock_adc_percentage);
    // Measure the status of the batteries once.
    module.process();
    REQUIRE(comm.get_send_frames().size() == 1); // When the percentage drops again but not under the next threshold (under 15% now) there should 
                                                        //not be a new packet send to the mock_can.

    // Changing the percentage of the accu
    mock_adc_percentage = 14; //14%
    mock_adc_value = calc_adc_value(min_voltage, max_voltage, mock_adc_percentage);
    // Measure the status of the batteries once.
    module.process();
    REQUIRE(comm.get_send_frames().size() == 2); // When the percentage drops again lower than 15 (14% < 15%), there should be another packet send to the mock_can.
}