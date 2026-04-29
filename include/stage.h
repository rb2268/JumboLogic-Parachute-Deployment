/* header file for the state machine used for deployment */

#ifndef STAGE_H
#define STAGE_H

#include "time.h"

#define ACCEL_LOCKOUT false
#define TIME_LOCKOUT 5000

typedef enum {
    STANDBY, BOOST, COAST, DESCENT
} STATE;

typedef struct {
    STATE state;

    float min_pressure;
    int baro_timestamp;

    float min_imu_mag;
    float max_imu_mag;
    int imu_timestamp;
} State_Data;

State_Data next_state(State_Data curr_state, float baro_pressure, float IMU_mag, uint32_t timestamp);

float pressure_to_altitude_f(float pressure);
float pressure_to_altitude_m(float pressure);

#endif