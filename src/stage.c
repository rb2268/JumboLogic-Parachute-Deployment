#include "stage.h"

// initializes a State_Data struct
State_Data_t init_staging(void) {
    State_Data_t init = {STANDBY, INFINITY, 0, -INFINITY, INFINITY, INFINITY, NAN, NAN, NAN};
    return init;
}

// find the next state based on current barometric data and 
State_Data_t next_state(State_Data_t curr_state, float baro_pressure, float target_altitude, float imu_mag, uint32_t timestamp) {
    if (baro_pressure < curr_state.min_pressure) {
        curr_state.min_pressure = baro_pressure;
        curr_state.baro_timestamp = timestamp;
    }

    if (imu_mag > curr_state.max_imu_mag) {
        curr_state.max_imu_mag = imu_mag;
    }

    if (imu_mag < curr_state.min_imu_mag) {
        curr_state.min_imu_mag = imu_mag;
    }

    switch (curr_state.state) {
        case STANDBY:
            if (standby_to_ascent(curr_state, imu_mag, timestamp)) {
                curr_state.state = ASCENT;
                curr_state.ascent_timestamp = timestamp;
            }
            break;
        case ASCENT:
            if (ascent_to_descent(curr_state, baro_pressure, timestamp)) {
                curr_state.state = DESCENT;
            }
            break;
        case DESCENT:
            if (descent_to_deploy(curr_state, baro_pressure, target_altitude)) {
                curr_state.state = DEPLOY;
                curr_state.deploy_altitude = pressure_to_altitude_f(baro_pressure);
            }
            break;
        case DEPLOY:
            break;
        default:
            break;
    }

    return curr_state;
}

/* ================================ STAGE TRANSITIONS ================================ */

bool standby_to_ascent(State_Data_t state, float imu_mag, uint32_t timestamp) {
    return (((ACCEL_LOCKOUT && imu_mag - state.min_imu_mag > ACCEL_DIFF_STA) || !ACCEL_LOCKOUT)
        && timestamp - state.standby_timestamp > STANDBY_LOCKOUT);
}

bool ascent_to_descent(State_Data_t state, float baro_pressure, uint32_t timestamp) {
    return ((baro_pressure > state.min_pressure + 100) && (timestamp - state.baro_timestamp > APOGEE_WINDOW)
        && (timestamp - state.ascent_timestamp > ASCENT_LOCKOUT));
}

bool descent_to_deploy(State_Data_t state, float baro_pressure, float target_altitude) {
    return pressure_to_altitude_f(baro_pressure) <= target_altitude;
}

/* =================================== CONVERSIONS =================================== */

// takes pressure in pascals and outputs altitude in feet
// see https://www.weather.gov/media/epz/wxcalc/pressureAltitude.pdf
float pressure_to_altitude_f(float pressure) {
    float press_mb = pressure  * 0.01;
    float altitude = 145366.45 * (1 - powf(press_mb * 0.00098692326, 0.190284));
    return altitude;
}

// takes pressure in pascals and outputs altitude in meters
float pressure_to_altitude_m_DO_NOT_USE(float pressure) {
    return pressure_to_altitude_f(pressure) * 0.3048;
}