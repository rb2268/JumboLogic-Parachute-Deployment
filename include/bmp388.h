#ifndef BMP388_H
#define BMP388_H

#define BARO_I2C_ADDR 0x76

// baro register defines
#define reg_ERR_REG    0x02
#define reg_STATUS     0x03

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

#define reg_INT_STATUS 0x11
#define reg_INT_CTRL   0x19
#define reg_PWR_CTRL   0x1B
#define reg_OSR        0x1C
#define reg_ODR        0x1D
#define reg_CONFIG     0x1F // configure IIR

/***
 * a struct containing important data
 */
// volatile float curr_sample;
// volatile float prev_sample;
// volatile float ref_sample;


void bmp388_init();
float bmp388_read();

#endif