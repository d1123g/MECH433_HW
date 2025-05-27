#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define PWM_FREQ_HZ 20000          // 20 kHz PWM
#define CLOCK_DIVIDER 64.0f
#define SYS_CLOCK_HZ 125000000     // Pico system clock

#define DUTY_STEP 0.01f            // 1% per step

#define APH_PIN 17 // Direction pin for motor
#define AEN_PIN 16 // PWM pin for motor

uint16_t pwm_wrap = 0;

void pwm_motor_init(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);

    pwm_wrap = (uint16_t)(SYS_CLOCK_HZ / (CLOCK_DIVIDER * PWM_FREQ_HZ)) - 1;

    pwm_set_clkdiv(slice_num, CLOCK_DIVIDER);
    pwm_set_wrap(slice_num, pwm_wrap);
    pwm_set_enabled(slice_num, true);
}

// Set duty cycle (0.0 – 1.0)
void motor_set_speed(uint pin, float duty_cycle) {
    if (duty_cycle < 0.0f) duty_cycle = 0.0f;
    if (duty_cycle > 1.0f) duty_cycle = 1.0f;

    uint16_t level = (uint16_t)(duty_cycle * (pwm_wrap + 1));
    pwm_set_gpio_level(pin, level);
}

int main() {
    sleep_ms(10000); // Wait for 10 seconds before starting
    stdio_init_all();

    gpio_init(APH_PIN);
    gpio_set_dir(APH_PIN, true);

    pwm_motor_init(AEN_PIN);

    float duty = 0.0f;
    gpio_put(APH_PIN, 1);  // Forward direction

    printf("Press '+' to increase speed, '-' to decrease, 'f' for forward, 'r' for reverse\n");

    while (true) {
        int c = getchar_timeout_us(0);  // non-blocking

        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '+') {
                duty += DUTY_STEP;
                if (duty > 1.0f) duty = 1.0f;
                printf("Increased duty: %.2f%%\n", duty * 100);
            } else if (c == '-') {
                duty -= DUTY_STEP;
                if (duty < 0.0f) duty = 0.0f;
                printf("Decreased duty: %.2f%%\n", duty * 100);
            } else if (c == 'f') {
                gpio_put(APH_PIN, 1);
                printf("Direction: forward\n");
            } else if (c == 'r') {
                gpio_put(APH_PIN, 0);
                printf("Direction: reverse\n");
            }
        }

        motor_set_speed(AEN_PIN, duty);
        sleep_ms(10);
    }
}
