#pragma once
#include <i2c_bus.hpp>

namespace r2d2::power{

    /**
    *ina219_internal_register are the internal addresses for the INA219 current,shunt and power monitor. 
    */
    enum ina219_internal_register : uint8_t{
        config_reg = 0x00,
        shunt_voltage_reg = 0x01,
        bus_voltage_reg = 0x02,
        power_reg = 0x03,
        current_reg = 0x04,
        calibration_reg = 0x05  
    };
    
    /**
    *ina219_configuration_values are the corresponding values of the different settings for the INA219 board. 
    */
    
    enum ina219_configuration_values: uint16_t{
        ///Reset the board's configuration register to its default value(0x399F).
        config_reset = 0x8000,
        ///Set the bus range to 16 or 32 Volt.
        config_BV_mask = 0x4000,
        config_BV_range_16V = 0x0000,
        config_BV_range_32V = 0x4000,
        ///Sets PGA gain and range.
        config_PGA_mask = 0x1800,
        config_PGA_1_40mV = 0x0000,
        config_PGA_2_80mV = 0x0800,
        config_PGA_4_160mV = 0x1000,
        config_PGA_8_320mV = 0x1800,
        ///set the bus ADC resolution in bits or the sampling rate when averaging results.
        config_BADC_mask = 0x0780,
        config_BADC_9b_84us = 0x0000,
        config_BADC_10b_148us = 0x0080,
        config_BADC_11b_276us = 0x0100,
        config_BADC_12b_532us = 0x0180,
        config_BADC_2_1060us = 0x0480,
        config_BADC_4_2130us = 0x0500,
        config_BADC_8_4260us = 0x0580,
        config_BADC_16_8510us = 0x0600,
        config_BADC_32_17020us = 0x0680,
        config_BADC_64_34050us = 0x0700,
        config_BADC_128_68100us = 0x0780,
        /// Set the shunt ADC resolution in bits or the sampling rate when averaging results.
        config_SADC_mask = 0x0078,
        config_SADC_9b_84us = 0x0000,
        config_SADC_10b_148us = 0x0008,
        config_SADC_11b_276us = 0x0010,
        config_SADC_12b_532us = 0x0018,
        config_SADC_2_1060us = 0x0048,
        config_SADC_4_2130us = 0x0050,
        config_SADC_8_4260us = 0x0058,
        config_SADC_16_8510us = 0x0060,
        config_SADC_32_17020us = 0x0068,
        config_SADC_64_34050us = 0x0070,
        config_SADC_128_68100us = 0x0078,
        /// Select the operating mode of the board.
        config_OM_mask = 0x0007,
        config_OM_powerdown = 0x0000,
        config_OM_SV_triggered = 0x0001,
        config_OM_BV_triggered = 0x0002,
        config_OM_SBV_triggered = 0x0003,
        config_OM_ADC_off = 0x0004,
        config_SV_continuous = 0x0005,
        config_BV_continuous = 0x0006,
        config_SBV_continues = 0x0007
    };
 

  
    /**
    *Class ina219_c is an interface for the ina219 current shunt and power monitor.
    */
    class ina219_c{
        public:
        /// The I2C device adress of the INA219.
        constexpr static uint8_t ina_i2c_adress = 0x40;
        /// The size of a register on the INA219, 2 bytes.
        constexpr static uint8_t size_of_ina_register = 0x2;
        /**
        *@brief Constructs a new ina219_c object,
        *Initializes the board with an calibration value for a solution expecting a load of 3A
        *
        *@param i2c_bus
        */
        ina219_c(r2d2::i2c::i2c_bus_c &i2c_bus);
        /**
        *@brief Reset the configuration register to it's default value (0x399F).
        */
        void reset_configuration();
        /**
        *@brief Set a calibration value for the board. The calibration value is used to calculate the power and current registers. If there is no calibration value set in the calibration register, the power and current registers remain empty.
        *
        *@param calib_val 
        */
        void set_calibration_value(uint16_t calib_val);
        /**
        *@brief Read the voltage register and calculate the corresponding value in mV.
        *
        *@return current voltage in mV.
        */
        uint16_t read_voltage();
        /**
        *@brief Read the power register and calculate the corresponding value in mW.
        *
        *@return current power in mW.
        */
        uint32_t read_power();
        /**
        *@brief Read the current register and calculate the corresponding value in mA.
        *
        *@return current current in mA.
        */
        uint16_t read_current();
        /**
        *@brief Write data to a register over the I2C bus.
        *
        *@param address
        *@param data
        */
        void write_register(ina219_internal_register address, uint16_t data);
        /**
        *@brief read data of a register over the I2C bus.
        *
        *@param address
        *@return register_data
        */
        uint16_t read_register(ina219_internal_register address);
        
        protected:
        /// The I2C bus, used for communication.
        r2d2::i2c::i2c_bus_c bus;
        private:
        /// The LSB value of the current register in uA.
        uint32_t current_lsb_uA =0;
        /// The LSB value of the power register in mW.
        uint32_t power_lsb_mW = 0 ;
    };

}//end namespace r2d2::power