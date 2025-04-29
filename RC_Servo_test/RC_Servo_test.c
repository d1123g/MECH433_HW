#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_MIN_PULSE_WIDTH_US 500    // 0 degrees
#define SERVO_MAX_PULSE_WIDTH_US 2500   // 180 degrees
#define SERVO_FREQ_HZ 50                // 50 Hz PWM
#define CLOCK_DIVIDER 64.0f             // PWM clock divider
#define SYS_CLOCK_HZ 150000000          // RP2040 system clock (150 MHz)
#define STEP_DEGREES 0.5f               // smaller step for smoother motion
#define STEP_DELAY_MS 15                // longer delay for slower sweep

uint16_t pwm_wrap = 0;  // persistently store the wrap value

// Convert microseconds to PWM level
uint16_t pulse_us_to_level(uint16_t us) {
    float ticks_per_us = (SYS_CLOCK_HZ / CLOCK_DIVIDER) / 1e6f;
    return (uint16_t)(us * ticks_per_us + 0.5f);
}

// Initialize PWM
void servo_pwm_init(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);

    pwm_wrap = (uint16_t)(SYS_CLOCK_HZ / (CLOCK_DIVIDER * SERVO_FREQ_HZ)) - 1;

    pwm_set_clkdiv(slice_num, CLOCK_DIVIDER);
    pwm_set_wrap(slice_num, pwm_wrap);
    pwm_set_enabled(slice_num, true);
}

// Set servo angle (0° to 180°)
void servo_set_angle(uint pin, float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    float pulse_width = SERVO_MIN_PULSE_WIDTH_US + 
        (angle / 180.0f) * (SERVO_MAX_PULSE_WIDTH_US - SERVO_MIN_PULSE_WIDTH_US);

    uint16_t level = pulse_us_to_level((uint16_t)pulse_width);
    pwm_set_gpio_level(pin, level);
}

int main() {
    stdio_init_all();

    const uint SERVO_PIN = 0;
    servo_pwm_init(SERVO_PIN);

    float angle = 0.0f;
    float direction = STEP_DEGREES;

    while (true) {
        servo_set_angle(SERVO_PIN, angle);
        sleep_ms(STEP_DELAY_MS);

        angle += direction;

        if (angle >= 180.0f) {
            angle = 180.0f;
            direction = -STEP_DEGREES;
        } else if (angle <= 0.0f) {
            angle = 0.0f;
            direction = STEP_DEGREES;
        }
    }
}
