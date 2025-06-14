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

#define MOTOR_MIN 0
#define MOTOR_MAX 0.75f // max PWM speed for motors

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

    // here we set up the GPIO pins for I2C communication
    gpio_set_function(I2C_SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_OLED);
    gpio_pull_up(I2C_SCL_OLED);

    // Initialize OLED display
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


    // the following functions are called to inialize the OLED display
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        // Read button state
        //check_button();

        // Read ADC
        uint16_t adc_value = adc_read();
        float voltage = (adc_value * 3.3f) / 4095.0f;
        float gain = 0.5f * voltage / 3.3f; // normalized gain (0 to 1)

        // Process camera data
        setSaveImage(1);
        while (getSaveImage() == 1) {}
        convertImage();
        int com = findLine(IMAGESIZEY / 2); // center of line
        setPixel(IMAGESIZEY / 2, com, 0, 255, 0);
        printf("%d\r\n", com); // print COM for debugging maybe should take out

        // Control motors based on COM and gain

        controller(gain, com); // control motors based on gain and COM


        // Draw to OLED based on display mode
        ssd1306_clear();

        // originally we were going to use this to display different modes but had an error where it only registered the button once.
        // going to instead use it on a single display mode and put it on each row. 

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

        // Row 1 displays Gain
        char buf1[30];
        sprintf(buf1, "Gain: %.2f", gain);
        drawmytext(2, 2, buf1); // Row 1

        // Row 2 displays PWM for the left and right motors
        char buf2[30];
        sprintf(buf2, "PWM L:%.0f%% PWM R:%.0f%%", left_speed * 100, right_speed * 100);
        drawmytext(2, 12, buf2); // Row 2

        // Row 3 displays Center of Mass (COM)
        char buf3[30];
        sprintf(buf3, "COM: %d", com);
        drawmytext(2, 22, buf3); // Row 3

        ssd1306_update();

        ssd1306_update();

        // Blink LED to make sure communication is working
        gpio_put(25, 1);
        sleep_ms(10);
        gpio_put(25, 0);

        // pixel blink to make sure OLED is working
        pixelBlink(0, 0);

        printf("ADC Value: %d\n", adc_value);
        printf("Voltage = %.2f V\n", voltage);
        printf("Gain = %.2f\n", gain);

        sleep_ms(100); // update ~100Hz
    }
}

// function to draw text on OLED display
void drawmytext(int x, int y, char *m) { // draws a string on the OLED display
    int i = 0;
    int startX = x;
    while (m[i] != '\0') { // loop through each character in the string
        if (x + 5 >= 128) {// if we reach the end of the display, wrap to next line 
            x = startX;
            y += 8;
            if (y + 8 > 64) break; // for 64px tall display
        }
        drawLetter(x, y, m[i]); //draw the letter using our function
        x += 6;
        i++;
    }
}

void drawLetter(int x, int y, char c) { //draw a single character on the OLED display using the font array
    int index = c - 0x20; // ASCII offset for space character
    if (index < 0 || index >= 96) return;

    for (int i = 0; i < 5; i++) { // loop through each column of the character
        char col = ASCII[index][i];
        for (int j = 0; j < 8; j++) { // loop through each row of the character
            char on = (col >> j) & 0b1; // check if pixel is on (1) or off (0)
            ssd1306_drawPixel(x + i, y + j, on); // draw the pixel on the OLED display
        }
    }
}

void pixelBlink(int x, int y) { // function to blink a pixel on the OLED display for visual feedback
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
        float base_speed = 0.6f; // base speed for motors 60% PWM
        left_speed = base_speed + turn_adjust;  // adjust left motor speed
        right_speed = base_speed - turn_adjust; // adjust right motor speed
        // Clamp speeds between a preset min and max
        if (left_speed < MOTOR_MIN) left_speed = MOTOR_MIN;
        if (left_speed > MOTOR_MAX) left_speed = MOTOR_MAX;
        if (right_speed < MOTOR_MIN) right_speed = MOTOR_MIN;
        if (right_speed > MOTOR_MAX) right_speed = MOTOR_MAX;
        motor_set_speed(IN1_PIN, left_speed);   // left motor
        motor_set_speed(IN2_PIN, right_speed);  // right motor
}

// this function checks the button state and handles debouncing
// UPDATE, we do not need to use this function in the main code 
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