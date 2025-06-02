#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/adc.h"
#include "cam.h"
#include "motor.h"

// I2C defines
#define I2C_PORT_OLED i2c0
#define I2C_SDA_OLED 20
#define I2C_SCL_OLED 21

// Button Defines
#define BUTTON_PIN 13 // GPIO pin for the button

float left_speed = 0;
float right_speed = 0;

// debounce defines
static bool last_button_state = false;
static int display_mode = 0;
static absolute_time_t last_debounce_time;
const uint64_t debounce_delay_us = 50000; // 50 ms in microseconds

void drawmytext(int x, int y, char *m);
void drawLetter(int x, int y, char c);
void pixelBlink(int x, int y);
void controller(float gain, int com);
void check_button();



int main()
{
    stdio_init_all();

    init_camera_pins();

    // I2C Initialization at 1MHz.
    i2c_init(I2C_PORT_OLED, 1000*1000);
    gpio_set_function(I2C_SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_OLED);
    gpio_pull_up(I2C_SCL_OLED);

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    // ADC initialization
    adc_init();
    adc_gpio_init(26); // GPIO 26 as ADC input
    adc_select_input(0); // ADC input 0

    // Button setup (button pulls HIGH when pressed, so we use pull-down)
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);

    // Initialize motors
    pwm_motor_init(IN1_PIN);
    pwm_motor_init(IN2_PIN);

    int display_mode = 0; // 0: Voltage/Gain, 1: PWM, 2: COM
    bool last_button_state = false;

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        // Read button state
        //check_button();

        // Read ADC
        uint16_t adc_value = adc_read();
        float voltage = (adc_value * 3.3f) / 4095.0f;
        float gain = 1.0f * voltage / 3.3f; // normalized gain (0 to 1)

        // Process camera data
        setSaveImage(1);
        while (getSaveImage() == 1) {}
        convertImage();
        int com = findLine(IMAGESIZEY / 2); // center of line
        setPixel(IMAGESIZEY / 2, com, 0, 255, 0);
        printf("%d\r\n", com);

        // Control motors based on COM and gain

        controller(gain, com); // control motors based on gain and COM


        // Draw to OLED based on display mode
        ssd1306_clear();

        // if (display_mode == 0) {
        //     drawmytext(2, 2, "ADC Versus Gain");

        //     char vbuf[30];
        //     sprintf(vbuf, "V: %.2fV", voltage);
        //     drawmytext(2, 12, vbuf);

        //     char gbuf[30];
        //     sprintf(gbuf, "Gain: %.2f", gain);
        //     drawmytext(2, 22, gbuf);

        // } else if (display_mode == 1) {
        //     drawmytext(2, 2, "PWM Values");

        //    char lbuf[30], rbuf[30];
        //     sprintf(lbuf, "Left PWM: %.0f%%", left_speed * 100);
        //     sprintf(rbuf, "Right PWM: %.0f%%", right_speed * 100);
        //     drawmytext(2, 12, lbuf);
        //     drawmytext(2, 22, rbuf);


        // } else if (display_mode == 2) {
        //     drawmytext(2, 2, "Line COM");

        //     char combuf[30];
        //     sprintf(combuf, "COM: %d", com);
        //     drawmytext(2, 12, combuf);
        // }

        // Display Gain, PWM L/R, and COM in 3 rows
        ssd1306_clear();

        char buf1[30];
        sprintf(buf1, "Gain: %.2f", gain);
        drawmytext(2, 2, buf1); // Row 1

        char buf2[30];
        sprintf(buf2, "PWM L:%.0f%% PWM R:%.0f%%", left_speed * 100, right_speed * 100);
        drawmytext(2, 12, buf2); // Row 2

        char buf3[30];
        sprintf(buf3, "COM: %d", com);
        drawmytext(2, 22, buf3); // Row 3

        ssd1306_update();

        ssd1306_update();

        // Blink LED
        gpio_put(25, 1);
        sleep_ms(10);
        gpio_put(25, 0);

        pixelBlink(0, 0);

        printf("ADC Value: %d\n", adc_value);
        printf("Voltage = %.2f V\n", voltage);
        printf("Gain = %.2f\n", gain);

        sleep_ms(1); // update ~100Hz
    }
}

void drawmytext(int x, int y, char *m) {
    int i = 0;
    int startX = x;
    while (m[i] != '\0') {
        if (x + 5 >= 128) {
            x = startX;
            y += 8;
            if (y + 8 > 64) break; // for 64px tall display
        }
        drawLetter(x, y, m[i]);
        x += 6;
        i++;
    }
}

void drawLetter(int x, int y, char c) {
    int index = c - 0x20;
    if (index < 0 || index >= 96) return;

    for (int i = 0; i < 5; i++) {
        char col = ASCII[index][i];
        for (int j = 0; j < 8; j++) {
            char on = (col >> j) & 0b1;
            ssd1306_drawPixel(x + i, y + j, on);
        }
    }
}

void pixelBlink(int x, int y) {
    ssd1306_drawPixel(x, y, 1);
    ssd1306_update();
    sleep_ms(10);
    ssd1306_drawPixel(x, y, 0);
    ssd1306_update();
}


void controller(float gain, int com) {
        //compute error
        int setpoint = IMAGESIZEX / 2;  // center of image
        int error = setpoint - com;     // how far off the line center we are
        float turn_adjust = gain * (float)error; // adjust turn based on gain and error
        float base_speed = 0.5f; // base speed for motors 50% PWM
        left_speed = base_speed + turn_adjust;  // adjust left motor speed
        right_speed = base_speed - turn_adjust; // adjust right motor speed
        // Clamp speeds between 0 and 1
        if (left_speed < 0) left_speed = 0;
        if (left_speed > 1) left_speed = 1;
        if (right_speed < 0) right_speed = 0;
        if (right_speed > 1) right_speed = 1;
        motor_set_speed(IN1_PIN, left_speed);   // left motor
        motor_set_speed(IN2_PIN, right_speed);  // right motor
}

void check_button() {
    bool current_button_state = gpio_get(BUTTON_PIN);

    if (current_button_state != last_button_state) {
        // Button state changed, reset debounce timer
        last_debounce_time = get_absolute_time();
    }

    if (absolute_time_diff_us(last_debounce_time, get_absolute_time()) > debounce_delay_us) {
        // debounce interval passed
        if (current_button_state && !last_button_state) {
            // rising edge detected: button just pressed
            if (display_mode == 2) {
                display_mode = 0;
            } else {
                display_mode++;
            }
            printf("Switched to mode %d\n", display_mode);
        }
    }
    last_button_state = current_button_state;
}