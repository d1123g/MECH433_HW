// motor.h
// This is going to define the interface for controlling a motor using PWM.
// It includes function declarations for initializing the PWM and setting the motor speed.
#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>

#define IN1_PIN 16                 // PWM pin (for IN1)
#define IN2_PIN 17                 // PWM pin (for IN2)
#define DUTY_STEP 0.01f            // 1% per step

void pwm_motor_init(int pin);
void motor_set_speed(int pin, float duty_cycle);

#endif
