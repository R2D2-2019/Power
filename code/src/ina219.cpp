#include "ina219.hpp"
namespace r2d2::power{
ina219_c::ina219_c(r2d2::i2c::i2c_bus_c &i2c_bus): bus(i2c_bus){
reset_configuration();
set_calibration_value(0x1000);
current_lsb_uA = 100;
power_lsb_mW = 1;
}



void ina219_c::reset_configuration(){
    write_register(ina219_internal_register::config_reg,ina219_configuration_values::config_reset);
}

void ina219_c::set_calibration_value(uint16_t calib_val){
    write_register(ina219_internal_register::calibration_reg,calib_val);
}

uint16_t ina219_c::read_voltage(){
    uint8_t LSB=4;//in mV
    uint8_t data_reg_offset = 3;
    uint16_t voltage_reg_value = read_register(ina219_internal_register::bus_voltage_reg);
    uint16_t voltage_data_mask = 0xFFF8;
    uint16_t voltage_data=voltage_reg_value&voltage_data_mask;
    voltage_data=voltage_data>>data_reg_offset;
    voltage_data= voltage_data*LSB;
    return voltage_data;
}

uint32_t ina219_c::read_power(){
    uint16_t power_reg_value = read_register(ina219_internal_register::power_reg);
    uint32_t power_data=power_reg_value*power_lsb_mW;
    return power_data;
}


uint16_t ina219_c::read_current(){
    uint16_t convert_to_mA_divider = 1000;
    uint16_t current_reg_value = read_register(ina219_internal_register::current_reg);
    uint16_t current_data=current_reg_value*current_lsb_uA;
    current_data=current_data/convert_to_mA_divider;
    return current_data;
}

void ina219_c::write_register(ina219_internal_register address,uint16_t data){
    uint8_t data_array[]={static_cast<uint8_t>(data>>8),static_cast<uint8_t>(data&0xFF)};
    bus.write(ina_i2c_adress,data_array,sizeof(data_array)/sizeof(uint8_t),address,sizeof(address)/sizeof(uint8_t));
}
uint16_t ina219_c::read_register(ina219_internal_register address){
    uint16_t data = 0x0000;
    uint8_t data_array[size_of_ina_register];
    int offset = 1;
    bus.read(ina_i2c_adress,data_array,size_of_ina_register,address,sizeof(address));
    for (uint8_t i = 0; i<sizeof(data_array);i++){
        
        data = data|data_array[i]<<(8*(sizeof(data_array)-i-offset));
    }
    return data;
}


}