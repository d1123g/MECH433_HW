#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define PWM_FREQ_HZ 1000          // 1 kHz PWM
#define CLOCK_DIVIDER 64.0f
#define FREQUENCY 100000     // Pico system clock frequency lower to be conservative

#define DUTY_STEP 0.01f            // 1% per step

#define APH_PIN 17 // Direction pin for motor
#define AEN_PIN 16 // PWM pin for motor

// Initialize PWM for motor control
uint16_t pwm_wrap = 0;

// Initialize the PWM for the motor control pin
void pwm_motor_init(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM); // Set pin to PWM function
    uint slice_num = pwm_gpio_to_slice_num(pin); // Get the PWM slice number

    pwm_wrap = (uint16_t)(FREQUENCY / (CLOCK_DIVIDER * PWM_FREQ_HZ)) - 1; // Calculate wrap value for the PWM

    pwm_set_clkdiv(slice_num, CLOCK_DIVIDER); // Set the clock divider for the PWM
    pwm_set_wrap(slice_num, pwm_wrap); // Set the wrap value for the PWM
    pwm_set_enabled(slice_num, true); // Enable the PWM slice
}

// Set the speed of the motor by adjusting the PWM duty cycle
void motor_set_speed(uint pin, float duty_cycle) {
    // Ensure the duty cycle is within the range [0.0, 1.0]
    if (duty_cycle < 0.0f) // Ensure duty cycle is not negative
        duty_cycle = 0.0f; 

    if (duty_cycle > 1.0f) // Ensure duty cycle does not exceed 100%
        duty_cycle = 1.0f;

    uint16_t level = (uint16_t)(duty_cycle * (pwm_wrap + 1)); // Calculate the level for the PWM based on the duty cycle
    pwm_set_gpio_level(pin, level); // Set the PWM level for the specified pin
}

int main() {
    sleep_ms(10000); // Wait for 10 seconds before starting
    stdio_init_all(); // Initialize all standard I/O

    gpio_init(APH_PIN); // Initialize the direction pin
    gpio_set_dir(APH_PIN, true); // Set direction pin as output

    pwm_motor_init(AEN_PIN); // Initialize the PWM for the motor control pin


    // here we will control the motor speed and direction using the PWM pin
    float duty = 0.0f; // Initial duty cycle set to 0%
    gpio_put(APH_PIN, 1);  // Forward direction

    // Print instructions for user
    printf("Press '+' to increase speed, '-' to decrease, 'f' for forward, 'r' for reverse\n");

    // Main loop to read user input and control motor speed and direction
    // Use getchar_timeout_us to read input without blocking
    while (true) {
        int c = getchar_timeout_us(0);  // non-blocking

        if (c != PICO_ERROR_TIMEOUT) { // Check if a character was read
            // Handle user input for speed and direction control
            if (c == '+') {
                duty += DUTY_STEP; // Increase duty cycle by DUTY_STEP
                if (duty > 1.0f) duty = 1.0f;
                printf("Increased duty: %.2f%%\n", duty * 100);
            } else if (c == '-') {
                duty -= DUTY_STEP; // Decrease duty cycle by DUTY_STEP
                if (duty < 0.0f) duty = 0.0f;
                printf("Decreased duty: %.2f%%\n", duty * 100);
            } else if (c == 'f') {
                gpio_put(APH_PIN, 1); // Set direction to forward
                printf("Direction: forward\n");
            } else if (c == 'r') {
                gpio_put(APH_PIN, 0); // Set direction to reverse
                printf("Direction: reverse\n");
            }
        }

        motor_set_speed(AEN_PIN, duty); // Set the motor speed based on the current duty cycle
        sleep_ms(10); // Sleep for a short duration
    }
}
