// motor.c
#include "motor.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

#define PWM_FREQ_HZ 10000.0f           // 10 kHz PWM
#define CLOCK_DIVIDER 1.0f
#define FREQUENCY 125000000.0f         // 125 MHz clock frequency

uint16_t pwm_wrap = 0;

void pwm_motor_init(int pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    int slice_num = pwm_gpio_to_slice_num(pin);

    pwm_wrap = (uint16_t)(FREQUENCY / (CLOCK_DIVIDER * PWM_FREQ_HZ)) - 1;

    pwm_set_clkdiv(slice_num, CLOCK_DIVIDER);
    pwm_set_wrap(slice_num, pwm_wrap);
    pwm_set_enabled(slice_num, true);
}

void motor_set_speed(int pin, float duty_cycle) {
    if (duty_cycle < 0.0f) duty_cycle = 0.0f;
    if (duty_cycle > 1.0f) duty_cycle = 1.0f;

    uint16_t level = (uint16_t)(duty_cycle * (pwm_wrap + 1));
    pwm_set_gpio_level(pin, level);
}
