#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define PWM_FREQ_HZ 10000.0f           // 1 kHz PWM
#define CLOCK_DIVIDER 1.0f
#define FREQUENCY 125000000.0f     // 125 MHz clock frequency

#define DUTY_STEP 0.01f            // 1% per step

#define IN1_PIN 16                 // PWM pin (for IN1)
#define IN2_PIN 17                 // Digital pin (for IN2)

uint16_t pwm_wrap = 0;

// Initialize PWM on IN1
void pwm_motor_init(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);

    pwm_wrap = (uint16_t)(FREQUENCY / (CLOCK_DIVIDER * PWM_FREQ_HZ)) - 1;

    pwm_set_clkdiv(slice_num, CLOCK_DIVIDER);
    pwm_set_wrap(slice_num, pwm_wrap);
    pwm_set_enabled(slice_num, true);
}  

// Set PWM duty cycle on IN1
void motor_set_speed(uint pin, float duty_cycle) {
    if (duty_cycle < 0.0f) duty_cycle = 0.0f;
    if (duty_cycle > 1.0f) duty_cycle = 1.0f;

    uint16_t level = (uint16_t)(duty_cycle * (pwm_wrap + 1));
    pwm_set_gpio_level(pin, level);
}

int main() {
    sleep_ms(1000);  // startup delay
    stdio_init_all();

    gpio_init(IN2_PIN);
    gpio_set_dir(IN2_PIN, true);  // Digital output for IN2

    pwm_motor_init(IN1_PIN);  // PWM on IN1

    float duty = 0.0f;
    bool forward = true;

    printf("Press '+' to increase speed, '-' to decrease, 'f' for forward, 'r' for reverse\n");

    while (true) {
        int c = getchar_timeout_us(0);

        if (c != PICO_ERROR_TIMEOUT) { // Check if a character was received
            if (c == '+') { // Increase duty cycle
                duty += DUTY_STEP;
                if (duty > 1.0f) duty = 1.0f;
                printf("Increased duty: %.2f%%\n", duty * 100);
            } else if (c == '-') { // Decrease duty cycle
                duty -= DUTY_STEP;
                if (duty < 0.0f) duty = 0.0f;
                printf("Decreased duty: %.2f%%\n", duty * 100);
            } else if (c == 'f') { // Set direction to forward
                forward = true;
                printf("Direction: forward\n");
            } else if (c == 'r') { // Set direction to reverse
                forward = false;
                printf("Direction: reverse\n");
            }
        }

        if (forward) {
            motor_set_speed(IN1_PIN, duty);
            gpio_put(IN2_PIN, 0);
        } else {
            motor_set_speed(IN1_PIN, 0);  // Turn off IN1 PWM
            gpio_put(IN2_PIN, 1);
            pwm_motor_init(IN2_PIN);      // Enable PWM on IN2 for reverse
            motor_set_speed(IN2_PIN, duty);
        }

        sleep_ms(100);
    }
}
