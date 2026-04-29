#include <stdio.h>
#include <stdbool.h>
#include "stm32l432xx.h"
#include "ee14lib.h"
#include "stage.h"
#include "time.h"


#define I2C_SCL_PIN D1
#define I2C_SDA_PIN D0
#define BARO_I2C_ADDR 0x76


// This function is called by printf() to handle the text string
// We want it to be sent over the serial terminal, so we just delegate to that function
int _write(int file, char *data, int len) {
    serial_write(USART2, data, len);
    return len;
}


int main()
{
    host_serial_init(9600);
    i2c_init(I2C1, I2C_SCL_PIN, I2C_SDA_PIN);
    SysTick_Init();

    while (true) {

        
        //i2c_read(I2C1, BARO_I2C_ADDR data, 1);// Figure out how to actually read, idk
        
    }
}