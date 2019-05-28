#pragma once 

#include <hwlib.hpp>

namespace r2d2::power {
    /**
     * The battery level class is a helper class
     * that reads an adc and provides a simple interface for
     * reading the voltage and percentage level of the battery.
     */ 
    class battery_level_c {
    protected:
        /**
         * Currently, this is a target::pin_adc because 
         * a hwlib::pin_adc interface doesn't exist (yet).
         */ 
        hwlib::adc &source;

        // The lowest voltage that the battery can reach.
        const uint_fast32_t min_voltage;

        // The difference between the maxmium and minimum voltages.
        const uint_fast32_t difference;        

    public:
        /**
         * Instantiate the battery level class.
         * 
         * All voltages (either given or returned) are multiplied by 1000.
         * So a voltage of 12.1V will be 12100. This is because this class doesn't use
         * floating point numbers to represent the value.
         * 
         * The minimum and maximum voltage values should be the normal range of the 
         * battery that is measured. Going outside this range means that the percentage
         * level that is calculated will be less accurate.
         * 
         * @param source The adc source that reads the battery level.
         * @param min_voltage The lowest voltage the battery can reach before it is "empty".
         * @param max_voltage The highest voltage the battery can reach before it is "full".
         */ 
        battery_level_c(
            hwlib::adc &source,
            const uint_fast32_t min_voltage,
            const uint_fast32_t max_voltage
        ) : source(source),
            min_voltage(min_voltage),
            difference(max_voltage - min_voltage) {}

        /**
         * Get the current voltage level of the battery.
         * This will generally be the voltage * 1000.
         * So a voltage of 12.1V will yield 12100.
         */ 
        uint_fast32_t get_battery_voltage() {
            /* 
             * The voltage sensor divides by 5. 
             * However, due to accuracy problem 4 produces
             * a more accurate result.
             * 
             * See https://docs.google.com/document/d/1BwI1GRxFp-6vBoxl7DooKZmc6JwJ7vQcBKDf2PtpRDM/edit?usp=sharing
             */ 
            return source.read() * 4;
        }

        /**
         * Get a estimate of how full the battery is
         * in percent.
         */ 
        uint_fast8_t get_battery_percentage() {                        
            uint_fast32_t voltage = get_battery_voltage();

            if (voltage <= min_voltage) {
                return 0;
            }

            uint_fast32_t percentage = ((voltage - min_voltage) * 100) / difference;

            return static_cast<uint_fast8_t>(percentage);
        }
    };
}