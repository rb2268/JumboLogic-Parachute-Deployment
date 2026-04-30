#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32l432xx.h"
#include "ee14lib.h"
#include "bmp388.h"
#include "stage.h"
#include "time.h"
#include "motor.h"
#include "buttons.h"


#define I2C_SCL_PIN D1
#define I2C_SDA_PIN D0

// This function is called by printf() to handle the text string
// We want it to be sent over the serial terminal, so we just delegate to that function
int _write(int file, char *data, int len) {
    serial_write(USART2, data, len);
    return len;
}


void config_gpio_interrupt(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI0_PB;
    EXTI->RTSR1 |= EXTI_RTSR1_RT0; // rising edge trigger
    EXTI->IMR1 |= EXTI_IMR1_IM0;
    EXTI->EMR1 |= EXTI_EMR1_EM0;
    NVIC_SetPriority(EXTI0_IRQn, 2); // (IRQ number, priority)
    NVIC_EnableIRQ(EXTI0_IRQn);
}

int main() {
    host_serial_init(9600);
    // i2c_init(I2C1, I2C_SCL_PIN, I2C_SDA_PIN);
    SysTick_Init();
    // bmp388_init();
    config_gpio_interrupt();

    for (volatile int i = 0; i < 20; i++) {
        printf("I'm alive!\n");
    }

    State_Data_t flight_state = init_staging();

    uint16_t ref_collect_cnt = 0;
    bool ref_altitude_set = false;
    float ref_pressure = 0;
    float ref_altitude = 0;

    bool motor_opened = false;

    motor_init(TIM16);

    setupButtons(A4, A6);

    while (true) {
        // printf("read: %f", bmp388_process());
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

        
        // if (get_time() % 2000 < 1000) {
        //     if (!motor_opened){
        //         NVIC_DisableIRQ(SysTick_IRQn);
        //         motor_open(TIM16, A5);
        //         NVIC_EnableIRQ(SysTick_IRQn);
        //         motor_opened = true;
        //     }
        // } else  {
        //     if (motor_opened) {
        //         NVIC_DisableIRQ(SysTick_IRQn);
        //         motor_close(TIM16, A5);
        //         NVIC_EnableIRQ(SysTick_IRQn);
        //         motor_opened = false;
        //     }
        // }

        runButtons(A4);
        turnMotor(A6);
    }
}