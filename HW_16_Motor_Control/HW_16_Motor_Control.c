#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

// Define GPIO pins for motor control
#define MOTOR_LEFT_PWM_PIN 15
#define MOTOR_LEFT_DIR_PIN 16
#define MOTOR_LEFT_EN_PIN 17
#define MOTOR_RIGHT_PWM_PIN 18
#define MOTOR_RIGHT_DIR_PIN 19
#define MOTOR_RIGHT_EN_PIN 20

#define PWM_WRAP_LIMIT 65535 // Set the PWM wrap limit for 16-bit resolution
#define PWM_FREQ 1000 // Set the PWM frequency to 1 kHz

#define WHEEL_DIAMETER 0.05 // Wheel diameter in meters 
#define WHEEL_CIRCUMFERENCE (WHEEL_DIAMETER * 3.14159) // Wheel circumference in meters




// Function to initialize the motor control pins
void motor_init() {
    // Initialize GPIO pins

    // Initialize GPIO pins for the left motor
    gpio_init(MOTOR_LEFT_PWM_PIN);
    gpio_set_dir(MOTOR_LEFT_PWM_PIN, GPIO_FUNC_PWM);

    gpio_init(MOTOR_LEFT_DIR_PIN);
    gpio_set_dir(MOTOR_LEFT_DIR_PIN, GPIO_OUT);

    gpio_init(MOTOR_LEFT_EN_PIN);
    gpio_set_dir(MOTOR_LEFT_EN_PIN, GPIO_OUT);

    
    // Initialize GPIO pins for the right motor
    gpio_init(MOTOR_RIGHT_PWM_PIN);
    gpio_set_dir(MOTOR_RIGHT_PWM_PIN, GPIO_FUNC_PWM);

    gpio_init(MOTOR_RIGHT_DIR_PIN);
    gpio_set_dir(MOTOR_RIGHT_DIR_PIN, GPIO_OUT);

    gpio_init(MOTOR_RIGHT_EN_PIN);
    gpio_set_dir(MOTOR_RIGHT_EN_PIN, GPIO_OUT);

    // PWM initialiation
    uint slice_num_left = pwm_gpio_to_slice_num(MOTOR_LEFT_PWM_PIN);
    pwm_set_wrap(slice_num_left, PWM_WRAP_LIMIT);
    pwm_set_enabled(slice_num_left, true);

    uint slice_num_right = pwm_gpio_to_slice_num(MOTOR_RIGHT_PWM_PIN);
    pwm_set_wrap(slice_num_right, PWM_WRAP_LIMIT);
    pwm_set_enabled(slice_num_right, true);
}

// Function to stop the motor 
// probably only using for drifting lol
void motor_stop(int motor) {
    if (motor == 0) { // Left motor
        gpio_put(MOTOR_LEFT_EN_PIN, 0); // Disable left motor
    } else if (motor == 1) { // Right motor
        gpio_put(MOTOR_RIGHT_EN_PIN, 0); // Disable right motor
    }
}

// Function to set the PWM frequency
void motor_set_pwm_freq(int freq) {
    pwm_set_clkdiv(MOTOR_LEFT_PWM_PIN, (float)clock_get_hz(clk_sys) / (PWM_WRAP_LIMIT * freq));
    pwm_set_clkdiv(MOTOR_RIGHT_PWM_PIN, (float)clock_get_hz(clk_sys) / (PWM_WRAP_LIMIT * freq));
}

// Function for h-bridge motor control
void motor_control(int motor, int speed) {
    if (motor == 0) { // Left motor
        if (speed > 0) {
            gpio_put(MOTOR_LEFT_DIR_PIN, 1); // Set direction to forward
        } else {
            gpio_put(MOTOR_LEFT_DIR_PIN, 0); // Set direction to backward
        }
        pwm_set_gpio_level(MOTOR_LEFT_PWM_PIN, abs(speed)); // Set PWM level
    } else if (motor == 1) { // Right motor
        if (speed > 0) {
            gpio_put(MOTOR_RIGHT_DIR_PIN, 1); // Set direction to forward
        } else {
            gpio_put(MOTOR_RIGHT_DIR_PIN, 0); // Set direction to backward
        }
        pwm_set_gpio_level(MOTOR_RIGHT_PWM_PIN, abs(speed)); // Set PWM level
    }
}

int main()
{
    stdio_init_all();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
