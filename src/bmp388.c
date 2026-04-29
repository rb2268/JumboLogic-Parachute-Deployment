/***
 * bmp388.c
 * 
 * This file implements functions for the STM32LK4xx microcontroller to
 * communicate with the BMP388 temperature and pressure sensor
 */

#include "ee14lib.h"

#define BARO_I2C_ADDR 0x76

// baro register defines
#define reg_ERR_REG    0x02

#define reg_PRESSURE_H 0x06 // outputs in unsigned 24 bit
#define reg_PRESSURE_M 0x05
#define reg_PRESSURE_L 0x04

#define reg_SENSRTIM_H 0x0E // same here
#define reg_SENSRTIM_M 0x0D
#define reg_SENSRTIM_L 0x0C

#define reg_FIFO_LEN_H 0x13
#define reg_FIFO_LEN_L 0x12

#define reg_FIFO_CFG_1 0x17
#define reg_FIFO_CFG_2 0x18

#define reg_INT_CTRL   0x19
#define reg_PWR_CTRL   0x1B
#define reg_OSR        0x1C
#define reg_ODR        0x1D
#define reg_CONFIG     0x1F // configure IIR

const float msb_to_pascal = 0.33;

/***
 * Initializes the barometer with the following:
 *  - IIR filter to 3 to reduce a bit of noise in data
 *  - Set the sensor to output data every 20ms (50Hz)
 *  - Set oversampling for pressure to 8x
 *  - Configure FIFO for continuous data collection without data loss
 *  - Enable interrupts for nonblocking read
 */
EE14Lib_Err bmp388_init()
{
    char i2c_transact[2];

    // set iir filter
    char iir_cfg = 0b10 << 1;
    i2c_transact[0] = reg_CONFIG;
    i2c_transact[1] = iir_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_transact, 2);

    // output frequency 50Hz
    char odr_sel = 0x02;
    i2c_transact[0] = reg_ODR;
    i2c_transact[1] = odr_sel;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_transact, 2);

    // pressure oversampling 8x
    char osr_cfg = 0b011;
    i2c_transact[0] = reg_OSR;
    i2c_transact[1] = osr_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_transact, 2);

    // enable fifo bc idk if it's default enabled
    // order of msb --> lsb config bits are:
    //  - store temp data in fifo ................................... 0
    //  - store pressure data in fifo ............................... 1
    //  - Return sensortime frame after the last valid data frame ... 0
    //  - Stop writing samples into FIFO when FIFO is full .......... 0
    //  - Enable fifo ............................................... 1
    char fifo_cfg = 0b01001;
    i2c_transact[0] = reg_FIFO_CFG_1;
    i2c_transact[1] = fifo_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_transact, 2);

    // interrupts
    // order of msb --> lsb config bits are:
    //   - enable interrupts .......................................... 1
    //   - enable Fifo full interrupt for INT pin & status ............ 0
    //   - enable FIFO watermark reached interrupt for INT & Status ... 0
    //   - Latching of interrupts for INT pin & INT_STATUS register ... 0
    //   - Interrupt pin active low (0) or high (1) ................... 1
    //   - Configure output: open-drain (1) or push-pull (0) .......... 1
    char int_cfg = 0b100011;
    i2c_transact[0] = reg_INT_CTRL;
    i2c_transact[1] = int_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_transact, 2);
}
