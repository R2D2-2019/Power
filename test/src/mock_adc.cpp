#include <mock_adc.hpp>

namespace r2d2::power {

    mock_adc_c::mock_adc_c(uint16_t return_value):
        adc(12),
        return_value(return_value)
    {};

    hwlib::adc::adc_value_type mock_adc_c::read(){
        return return_value;
    };

    void mock_adc_c::refresh(){
        // Empty function.
    };

    void mock_adc_c::set_return_value(uint16_t value){
        return_value = value;
    }; 
}