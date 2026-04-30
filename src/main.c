#include <stdio.h>
#include <stdbool.h>
#include "stm32l432xx.h"
#include "ee14lib.h"
#include "stage.h"
#include "time.h"
#include "motor.h"


#define I2C_SCL_PIN D1
#define I2C_SDA_PIN D0

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

    State_Data_t flight_state = init_staging();

    uint16_t ref_collect_cnt = 0;
    bool ref_altitude_set = false;
    float ref_pressure = 0;
    float ref_altitude = 0;

    bool motor_opened = false;

    motor_init(TIM16);

    while (true) {
        /*
        LOGIC:
        COLLECT BARO SAMPLE
        COLLECT IMU SAMPLE

        IF (STANDBY and ref_collect_cnt < 250):
            ref_pressure = (ref_pressure * ref_collect_cnt + BARO SAMPLE) / (++ref_collect_cnt)
        ELSE IF (NOT ref_altitude_set):
            ref_altitude_set = true
            ref_altitude = pressure_to_altitude_f(ref_pressure)
        
        float imu_mag = 0;
        if (ACCEL_LOCKOUT):
            imu_mag = sqrt(IMU_SAMPLE_X ^ 2 + IMU_SAMPLE_Y ^ 2 + IMU_SAMPLE_Z ^ 2)

        flight_state = next_state(flight_state, BARO SAMPLE, ref_pressure, imu_mag, get_time());

        IF (flight_state.state > STANDBY):
            
        */

        
        if (get_time() % 2000 < 1000) {
            if (!motor_opened){
                NVIC_DisableIRQ(SysTick_IRQn);
                motor_open(TIM16, A5);
                NVIC_EnableIRQ(SysTick_IRQn);
                motor_opened = true;
            }
        } else  {
            if (motor_opened) {
                NVIC_DisableIRQ(SysTick_IRQn);
                motor_close(TIM16, A5);
                NVIC_EnableIRQ(SysTick_IRQn);
                motor_opened = false;
            }
        }
    }
}