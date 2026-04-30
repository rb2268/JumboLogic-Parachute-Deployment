/***
 * bmp388.c
 * 
 * This file implements functions for the STM32LK4xx microcontroller to
 * communicate with the BMP388 temperature and pressure sensor
 */

#include <stdio.h>
#include <stdbool.h>
#include "ee14lib.h"
#include "bmp388.h"

const float MSB_TO_PASCALS = 0.017368F;

/***
 * Initializes the barometer with the following:
 *  - IIR filter to 3 to reduce a bit of noise in data
 *  - Set the sensor to output data every 20ms (50Hz)
 *  - Set oversampling for pressure to 8x
 *  - Configure FIFO for continuous data collection without data loss
 *  - Enable interrupts for nonblocking read
 */
void bmp388_init()
{
    volatile unsigned char i2c_write_buf[2];

    // set iir filter
    volatile unsigned char iir_cfg = 0b100 << 1;
    i2c_write_buf[0] = reg_CONFIG;
    i2c_write_buf[1] = iir_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_write_buf, 2);

    // output frequency 50Hz
    volatile unsigned char odr_sel = 0x02;
    i2c_write_buf[0] = reg_ODR;
    i2c_write_buf[1] = odr_sel;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_write_buf, 2);

    // pressure oversampling 8x
    volatile unsigned char osr_cfg = 0b000011;
    i2c_write_buf[0] = reg_OSR;
    i2c_write_buf[1] = osr_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_write_buf, 2);

    // ENABLE THE PRESSURE SENSOR DUMBASS
    volatile unsigned char pwr_cfg = 0b111111; // enable the temp sensor too ig
    i2c_write_buf[0] = reg_PWR_CTRL;
    i2c_write_buf[1] = pwr_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_write_buf, 2);

    // enable fifo bc idk if it's default enabled
    // order of msb --> lsb config bits are:
    //  - store temp data in fifo ................................... 0
    //  - store pressure data in fifo ............................... 1
    //  - Return sensortime frame after the last valid data frame ... 0
    //  - Stop writing samples into FIFO when FIFO is full .......... 0
    //  - Enable fifo ............................................... 1
    volatile unsigned char fifo_cfg = 0b00000;
    i2c_write_buf[0] = reg_FIFO_CFG_1;
    i2c_write_buf[1] = fifo_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_write_buf, 2);

    // interrupts
    // order of msb --> lsb config bits are:
    //   - enable interrupts .......................................... 0
    //   - NOT USED ................................................... 0
    //   - enable FIFO full interrupt for INT pin & status ............ 1
    //   - enable FIFO watermark reached interrupt for INT & Status ... 0
    //   - Latching of interrupts for INT pin & INT_STATUS register ... 0
    //   - Interrupt pin active low (0) or high (1) ................... 0
    //   - Configure output: open-drain (1) or push-pull (0) .......... 1
    volatile unsigned char int_cfg = 0b00000000; // Interrupts are BANNED
    i2c_write_buf[0] = reg_INT_CTRL;
    i2c_write_buf[1] = int_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_write_buf, 2);
}


/***
 * Read once from the sensor
 */
float bmp388_read()
{
    static float last_valid_value = 0.0;
    
    volatile unsigned char err_addr = reg_ERR_REG;
    volatile unsigned char err;
    if (i2c_write2read(I2C1, BARO_I2C_ADDR, &err_addr, 1, &err, 1)) {
        if (err) {
            printf("ERROR!!!\n");
        }
    }

    volatile unsigned char addr = reg_STATUS;
    volatile unsigned char status;
    if (i2c_write2read(I2C1, BARO_I2C_ADDR, &addr, 1, &status, 1)) {
        while (!(status & 0b10000)) {} // pressure data ready
    } else {
        printf("write 2 read failed\n");
    }

    volatile unsigned char pressure_addr = reg_PRESSURE_L;
    volatile unsigned char pressure_data[3];
    if (i2c_write2read(I2C1, BARO_I2C_ADDR, &pressure_addr, 1, pressure_data, 3)) {
        volatile int pressure_bits = pressure_data[0] << 16 | pressure_data[1] << 8 | pressure_data[2];
        printf("pressure bits: %lu\n", pressure_bits);
        last_valid_value = (float) pressure_bits * MSB_TO_PASCALS;
    }
    return last_valid_value;
}



/***
 * Handles interrupts from the barometer
 */
// void EXTI0_IRQHandler(void) {
//     if(!(EXTI->PR1 & EXTI_PR1_PIF0)){
//         return;
//     }

//     // read from barometer (LITTLE ENDIAN!!)
//     volatile unsigned char pressure_regaddr = reg_PRESSURE_L;
//     volatile unsigned char i2c_read_buffer[3];
//     NVIC_DisableIRQ(EXTI0_IRQn);
//     i2c_write2read(I2C1, BARO_I2C_ADDR, &pressure_regaddr, 1, i2c_read_buffer, 3); //read is autoincrement
//     NVIC_EnableIRQ(EXTI0_IRQn);

//     // Interrupt status at barometer is cleared by reading drdy bit high
//     volatile unsigned char interrupt_regaddr = reg_INT_STATUS;
//     volatile unsigned char i2c_garbage_buffer;
//     i2c_write2read(I2C1, BARO_I2C_ADDR, &interrupt_regaddr, 1, &i2c_garbage_buffer, 1);

//     prev_sample = curr_sample;
//     curr_sample = (int)(
//                             i2c_read_buffer[2] << 16 |
//                             i2c_read_buffer[1] << 8 |
//                             i2c_read_buffer [2]
//                         ) * MSB_TO_PASCALS;


//     EXTI->PR1 = EXTI_PR1_PIF0; // Clear pending bit by setting it
// }


