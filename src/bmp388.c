/***
 * bmp388.c
 * 
 * This file implements functions for the STM32LK4xx microcontroller to
 * communicate with the BMP388 temperature and pressure sensor
 */

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "ee14lib.h"
#include "bmp388.h"

// const float MSB_TO_PASCALS = 0.015625f;


/***
 * Calibration bs
 */

typedef struct BMP388_calib_data {
    float par_t1;  // LSB: 0x31 & MSB: 0x32
    float par_t2;  // LSB: 0x33 & MSB: 0x34
    float par_t3;  // 0x35
    float t_lin;   // put calibrated temp data in here ig

    float par_p1;  // LSB: 0x36 & MSB: 0x37
    float par_p2;  // lSB: 0x38 & MSB: 0x39
    float par_p3;  // 0x3A
    float par_p4;  // 0x3B
    float par_p5;  // LSB: 0x3C & MSB: 0x3D
    float par_p6;  // LSB: 0x3E, MSB: 0x3F
    float par_p7;  // 0x40
    float par_p8;  // 0x41
    float par_p9;  // LSB: 0x42, MSB: 0x43
    float par_p10; // 0x44
    float par_p11; // 0x45
} *CalibDat;

struct BMP388_calib_data calib_data_storage;
CalibDat calib_data = &calib_data_storage;

void fill_in_calib_data(CalibDat calib_data)
{
    volatile unsigned char reg_addr = 0x31;
    volatile unsigned char output_buf[2];
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    int temp = output_buf[0] | output_buf[1] << 8;
    calib_data->par_t1 = (float) temp / 0.00390625;

    reg_addr = 0x33;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    temp = output_buf[0] | output_buf[1] << 8;
    calib_data->par_t2 = (float) temp / 1073741824.0;

    
    reg_addr = 0x35;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 1);
    temp = output_buf[0];
    calib_data->par_t3 = (float) (int8_t)temp / 2.814749767E14;

    reg_addr = 0x36;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    temp = (int16_t)(output_buf[0] | output_buf[1] << 8);
    calib_data->par_p1 = ((float)temp - 16384.0f) / 1048576.0f;



    reg_addr = 0x38;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    temp = (int16_t)(output_buf[0] | output_buf[1] << 8);
    calib_data->par_p2 = ((float)temp - 16384.0f) / 536870912.0f;

    reg_addr = 0x3A;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    calib_data->par_p3 = (float) (int8_t)output_buf[0] / 4294967296.0;
    calib_data->par_p4 = (float) (int8_t)output_buf[1] / 1.374389535E11;

    reg_addr = 0x3C;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    temp = output_buf[0] | output_buf[1] << 8;
    calib_data->par_p5 = (float) temp / 0.125;

    reg_addr = 0x3E;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    temp = output_buf[0] | output_buf[1] << 8;
    calib_data->par_p6 = ((float) temp) / 64.0;

    reg_addr = 0x40;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    temp = output_buf[0];
    calib_data->par_p7 = (float) (int8_t)temp / 256.0;
    temp = output_buf[1];
    calib_data->par_p8 = (float) (int8_t)temp / 32768.0;

    reg_addr = 0x42;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    temp = (int16_t)(output_buf[0] | output_buf[1] << 8);  // needs (int16_t) cast
    calib_data->par_p9 = ((float)temp) / 281474976710656.0f;

    reg_addr = 0x44;
    i2c_write2read(I2C1, BARO_I2C_ADDR, &reg_addr, 1, output_buf, 2);
    temp = output_buf[0];
    calib_data->par_p10 = (float) (int8_t)temp / 281474976710656.0;
    temp = output_buf[1];
    calib_data->par_p11 = (float) (int8_t)temp / 36893488147419103232.0;
}


static float BMP388_compensate_temperature(uint32_t uncomp_temp, CalibDat calib_data)
{
    float partial_data1;
    float partial_data2;
    partial_data1 = (float)(uncomp_temp - calib_data->par_t1);
    partial_data2 = (float)(partial_data1 * calib_data->par_t2);
    /* Update the compensated temperature in calib structure since this is
    * needed for pressure calculation */
    calib_data->t_lin = partial_data2 + (partial_data1 * partial_data1) * calib_data->par_t3;
    /* Returns compensated temperature */
    return calib_data->t_lin;
}


static float BMP388_compensate_pressure(uint32_t uncomp_press, CalibDat calib_data)
{
    /* Variable to store the compensated pressure */
    float comp_press;
    /* Temporary variables used for compensation */
    float partial_data1;
    float partial_data2;
    float partial_data3;
    float partial_data4;
    float partial_out1;
    float partial_out2;
    /* Calibration data */
    partial_data1 = calib_data->par_p6 * calib_data->t_lin;
    partial_data2 = calib_data->par_p7 * (calib_data->t_lin * calib_data->t_lin);
    partial_data3 = calib_data->par_p8 * (calib_data->t_lin * calib_data->t_lin * calib_data->t_lin);
    partial_out1 = calib_data->par_p5 + partial_data1 + partial_data2 + partial_data3;
    partial_data1 = calib_data->par_p2 * calib_data->t_lin;
    partial_data2 = calib_data->par_p3 * (calib_data->t_lin * calib_data->t_lin);
    partial_data3 = calib_data->par_p4 * (calib_data->t_lin * calib_data->t_lin * calib_data->t_lin);
    partial_out2 = (float)uncomp_press *
    (calib_data->par_p1 + partial_data1 + partial_data2 + partial_data3);
    partial_data1 = (float)uncomp_press * (float)uncomp_press;
    partial_data2 = calib_data->par_p9 + calib_data->par_p10 * calib_data->t_lin;
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3 + ((float)uncomp_press * (float)uncomp_press * (float)uncomp_press) * calib_data->par_p11;
    comp_press = partial_out1 + partial_out2 + partial_data4;

    return comp_press;
}







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
    fill_in_calib_data(calib_data);


    volatile unsigned char i2c_write_buf[2];

    // set iir filter
    volatile unsigned char iir_cfg = 0b00 << 1;
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
    volatile unsigned char pwr_cfg = 0b111111; // and temp sensor too!
    i2c_write_buf[0] = reg_PWR_CTRL;
    i2c_write_buf[1] = pwr_cfg;
    i2c_write(I2C1, BARO_I2C_ADDR, i2c_write_buf, 2);

    // // enable fifo bc idk if it's default enabled
    // // order of msb --> lsb config bits are:
    // //  - store temp data in fifo ................................... 0
    // //  - store pressure data in fifo ............................... 1
    // //  - Return sensortime frame after the last valid data frame ... 0
    // //  - Stop writing samples into FIFO when FIFO is full .......... 0
    // //  - Enable fifo ............................................... 0
    // volatile unsigned char fifo_cfg = 0b00000;
    // i2c_write_buf[0] = reg_FIFO_CFG_1;
    // i2c_write_buf[1] = fifo_cfg;
    // i2c_write(I2C1, BARO_I2C_ADDR, i2c_write_buf, 2);

    // // interrupts
    // // order of msb --> lsb config bits are:
    // //   - enable interrupts .......................................... 0
    // //   - NOT USED ................................................... 0
    // //   - enable FIFO full interrupt for INT pin & status ............ 1
    // //   - enable FIFO watermark reached interrupt for INT & Status ... 0
    // //   - Latching of interrupts for INT pin & INT_STATUS register ... 0
    // //   - Interrupt pin active low (0) or high (1) ................... 0
    // //   - Configure output: open-drain (1) or push-pull (0) .......... 1
    // volatile unsigned char int_cfg = 0b00000000; // Interrupts are BANNED
    // i2c_write_buf[0] = reg_INT_CTRL;
    // i2c_write_buf[1] = int_cfg;
    // i2c_write(I2C1, BARO_I2C_ADDR, i2c_write_buf, 2);
}





/***
 * Read once from the sensor
 */
float bmp388_read()
{
    static float last_valid_pressure = 0.0;
    static float last_valid_temp = 0.0;
    
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
        while (!(status & 0b110000)) {} // pressure and temp data ready
    } else {
        printf("data not ready!!!\n");
    }

    volatile unsigned char temp_addr = reg_TEMP_L;
    volatile unsigned char temp_data[3];
    if (i2c_write2read(I2C1, BARO_I2C_ADDR, &temp_addr, 1, temp_data, 3)) {
        volatile int temp_bits = (temp_data[0] | temp_data[1] << 8 | temp_data[2] << 16);
        last_valid_temp = BMP388_compensate_temperature(temp_bits, calib_data);
    }
    calib_data->t_lin = last_valid_temp;

    volatile unsigned char pressure_addr = reg_PRESSURE_L;
    volatile unsigned char pressure_data[3];
    if (i2c_write2read(I2C1, BARO_I2C_ADDR, &pressure_addr, 1, pressure_data, 3)) {
        volatile int pressure_bits = (pressure_data[0] | pressure_data[1] << 8 | pressure_data[2] << 16);
        // printf("pressure bits: %lu\n", pressure_bits);
        last_valid_pressure = BMP388_compensate_pressure(pressure_bits, calib_data);
    }
    printf("pressure: %lu\n", (long unsigned) last_valid_pressure);
    return last_valid_pressure;
}




/***
 * Handles interrupts from the barometer
 */
// void EXTI0_IRQHandler(void) {
//     if(!(EXTI->PR1 & EXTI_PR1_PIF0)){
//         return;
//     }
// 
//     // read from barometer (LITTLE ENDIAN!!)
//     volatile unsigned char pressure_regaddr = reg_PRESSURE_L;
//     volatile unsigned char i2c_read_buffer[3];
//     NVIC_DisableIRQ(EXTI0_IRQn);
//     i2c_write2read(I2C1, BARO_I2C_ADDR, &pressure_regaddr, 1, i2c_read_buffer, 3); //read is autoincrement
//     NVIC_EnableIRQ(EXTI0_IRQn);
// 
//     // Interrupt status at barometer is cleared by reading drdy bit high
//     volatile unsigned char interrupt_regaddr = reg_INT_STATUS;
//     volatile unsigned char i2c_garbage_buffer;
//     i2c_write2read(I2C1, BARO_I2C_ADDR, &interrupt_regaddr, 1, &i2c_garbage_buffer, 1);
// 
//     prev_sample = curr_sample;
//     curr_sample = (int)(
//                             i2c_read_buffer[2] << 16 |
//                             i2c_read_buffer[1] << 8 |
//                             i2c_read_buffer [2]
//                         ) * MSB_TO_PASCALS;
// 
// 
//     EXTI->PR1 = EXTI_PR1_PIF0; // Clear pending bit by setting it
// }


