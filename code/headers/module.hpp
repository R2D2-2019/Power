#pragma once 

#include <base_module.hpp>
#include <battery.hpp>

namespace r2d2::power {
    class module_c : public base_module_c {
    private:
        battery_level_c &level_meter;

        /**
         * The warning increment is an offset of the battery
         * percentage that prevents spamming the bus with warnings
         * when the threshold is reached.
         */ 
        constexpr static uint8_t warning_increment = 5;

        /**
         * The battery level in percentage that
         * triggers the warning on the bus.
         */ 
        uint8_t warning_percentage;

        /**
         * The percentage level at which the 
         * last warning was given.
         */ 
        uint8_t last_warning_percentage;

    public:
        /**
         * @param comm The internal communication bus instance.
         * @param level_meter The battery level meter that represents the battery.
         * @param warning_percentage The battery percentage level at which a warning will be send on the bus.
         */
        module_c(
            base_comm_c &comm,
            battery_level_c &level_meter,
            uint_fast8_t warning_percentage
        ) : base_module_c(comm),
            level_meter(level_meter),
            warning_percentage(warning_percentage),
            last_warning_percentage(255) {

            comm.listen_for_frames({
                frame_type::BATTERY_LEVEL
            });
        }

        /**
         * Let the module process data.
         */
        void process() override {
            frame_battery_level_s frame;
            frame.percentage = level_meter.get_battery_percentage();
            frame.voltage = level_meter.get_battery_voltage();

            while (comm.has_data()) {
                auto frame = comm.get_data();

                // Only process requests here.
                if (!frame.request) {
                    continue;
                }

                comm.send(frame);
            }

            // Check for dangerously low battery levels.            
            if (frame.percentage < warning_percentage 
                && frame.percentage - warning_increment < last_warning_percentage) {
                // Power low; high priority warning packet.
                comm.send(frame, priority::HIGH);

                // Adjust last warning, otherwise we'll spam the warning
                // on the bus.
                last_warning_percentage = warning_percentage;
            }

            // If the battery is recharged, reset the warnings.
            if (frame.percentage > warning_percentage) {
                last_warning_percentage = 255;
            }
        }
    };
}