/* header file for the state machine used for deployment */

#ifndef STAGE_H
#define STAGE_H

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#define ACCEL_LOCKOUT false
#define ACCEL_DIFF_STA 1.5
#define STANDBY_LOCKOUT 5000
#define ASCENT_LOCKOUT 2000
#define APOGEE_WINDOW 300

typedef enum {
    STANDBY, ASCENT, DESCENT, DEPLOY
} STATE;

typedef struct State_Data {
    STATE state;

    float min_pressure;
    int baro_timestamp;

    float min_imu_mag;
    float max_imu_mag;

    uint32_t standby_timestamp;
    uint32_t ascent_timestamp;
    float deploy_altitude;
} State_Data_t;

State_Data_t init_staging(void);

State_Data_t next_state(State_Data_t curr_state, float baro_pressure, float target_altitude, float imu_mag, uint32_t timestamp);
bool standby_to_ascent(State_Data_t state, float imu_mag_diff, uint32_t timestamp);
bool ascent_to_descent(State_Data_t state, float baro_pressure, uint32_t timestamp);
bool descent_to_deploy(State_Data_t state, float baro_pressure, float target_altitude);

float pressure_to_altitude_f(float pressure);
float pressure_to_altitude_m_DO_NOT_USE(float pressure);

#endif