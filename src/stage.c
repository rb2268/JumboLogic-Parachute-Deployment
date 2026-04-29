#include "stage.h"
#include <math.h>

// find the next state based on current barometric data
State_Data next_state(State_Data curr_state, float baro_data, float IMU_mag, uint32_t timestamp) {
    
}

// takes pressure in pascals and outputs altitude in feet
// see https://www.weather.gov/media/epz/wxcalc/pressureAltitude.pdf
float pressure_to_altitude_f(float pressure) {
    float press_mb = pressure  * 0.01;
    float altitude = 145366.45 * (1 - powf(press_mb / 1.01325, 0.190284));
    return altitude;
}

// takes pressure in pascals and outputs altitude in meters
float pressure_to_altitude_m(float pressure) {
    return pressure_to_altitude_f(pressure) * 0.3048;
}