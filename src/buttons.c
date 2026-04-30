#include "ee14lib.h"

// State definitions
typedef enum { BLINKING, STEADY } State;
State currentState = BLINKING;
int currentLed = 0;
EE14Lib_Pin leds[] = {A0, A1, A2, A3};
uint32_t lastButtonPress = 0;

bool buttonPressed = false;
bool lastButtonPressed = false;


void setupButtons(EE14Lib_Pin input) {
    for(int i=0; i<4; i++) gpio_config_mode(leds[i], OUTPUT);

    gpio_config_mode(input, INPUT);
    gpio_config_pullup(input, PULL_UP);
    
    timer_config_freerun(TIM1, 3999);
}

void runButtons(EE14Lib_Pin input){
    buttonPressed = gpio_read(input);
    uint32_t now = get_time();
    if(!buttonPressed) {
        lastButtonPress = now;
    }
    if(buttonPressed) { //Button pressed
        if(currentState == STEADY) {
            currentState = BLINKING;
        }
        else {
            if(!lastButtonPressed) {
                gpio_write(leds[currentLed], 0);
                currentLed = (currentLed + 1) % 4;
            }
        }
    }
    if(now - lastButtonPress > 5000) {
        currentState = STEADY;
    }
    if(currentState == STEADY) {
        gpio_write(leds[currentLed], 1);
    }
    else {
        // Toggle LED every 250ms for blinking
        bool blinkBit = (now / 250) % 2;
        gpio_write(leds[currentLed], blinkBit);
    }
    lastButtonPressed = buttonPressed;
}