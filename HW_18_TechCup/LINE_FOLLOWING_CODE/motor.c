// motor.c
// This code implements the functions declared in motor.h for controlling a motor using PWM.

#include "motor.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"


// here we define the PWM frequency and clock settings for motor control
#define PWM_FREQ_HZ 10000.0f           // 10 kHz PWM
#define CLOCK_DIVIDER 1.0f
#define FREQUENCY 125000000.0f         // 125 MHz clock frequency based on the raspberry pico 2

// This defines the wrap value for the PWM signal
uint16_t pwm_wrap = 0;

// function is used to initialize the PWM for motor control
void pwm_motor_init(int pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM); // set the pin to PWM function
    int slice_num = pwm_gpio_to_slice_num(pin); // get the pwm slice number for the pin

    pwm_wrap = (uint16_t)(FREQUENCY / (CLOCK_DIVIDER * PWM_FREQ_HZ)) - 1; // apply the formula to calculate the wrap value

    pwm_set_clkdiv(slice_num, CLOCK_DIVIDER); // set the clock divider for the pwm slice
    pwm_set_wrap(slice_num, pwm_wrap); // here we set the wrap value for the pwm slice
    pwm_set_enabled(slice_num, true); // we enable the pwm slice
}

void motor_set_speed(int pin, float duty_cycle) { // this function sets the motor speed by adjusting the PWM duty cycle
    if (duty_cycle < 0.0f) duty_cycle = 0.0f;  // clamps duty cycle to a minimum of 0%
    if (duty_cycle > 1.0f) duty_cycle = 1.0f; // clamps duty cycle to a maximum of 100

    uint16_t level = (uint16_t)(duty_cycle * (pwm_wrap + 1)); // calculate the PWM level based on the duty cycle
    pwm_set_gpio_level(pin, level); // set the PWM level for the specified pin
}
