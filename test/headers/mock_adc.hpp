#pragma once 

#include <hwlib.hpp>

namespace r2d2::power {

    class mock_adc_c : public hwlib::adc {
        private:
            uint16_t & return_value;

        public:
            /**
            * This mock_adc constructor will set the simulated value given in the constructor 
            * as return_value so the class returns this value when 'reading' from the pin.
            */
            mock_adc_c(uint16_t & return_value):
                adc(12),
                return_value(return_value)
            {};

            /**
             * This read function returns the return_value as copy. 
             */
            adc_value_type read() override{
                return return_value;
            };

            /**
             * The constructor receives the return_value as reference parameter so there is no need to refresh
             * a value with a refresh function. Still we have to define the function because the function is
             * abstract. 
             */
            void refresh() override{};
    };
}