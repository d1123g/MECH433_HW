#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/adc.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 20
#define I2C_SCL 21


void drawmytext(int x, int y, char *m);
void drawLetter(int x, int y, char c);
void pixelBlink(int x, int y);
void display_voltage(float voltage);
// have 128 pixels in a row, 32 pixels in a column
// x,y coordinates are in the positive x direction and "negative" y direction
// 0,0 is the top left corner of the screen
// loops through every character until you hit the null character, which sprintf sticks in there for us


int main()
{
    stdio_init_all();

    // I2C Initialization at 1Mhz.
    i2c_init(I2C_PORT, 1000*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    //adc initializtion
    adc_init(); // initialize the ADC
    adc_gpio_init(26); // initialize GPIO 26 as an ADC input
    adc_select_input(0); // select ADC input 0 (GPIO 26)



    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        // Read ADC
        uint16_t adc_value = adc_read();
        float voltage = (adc_value * 3.3f) / 4095.0f;

        // Draw to OLED
        ssd1306_clear(); // clear ONCE at the start
        drawmytext(2, 2, "Damian Gonzalez's"); // 2,2 is the x,y position of the text
        drawmytext(2, 12, "HW7 I2C OLED"); // this line is 10 pixels below the previous line
        
        char vbuf[30];
        sprintf(vbuf, "Voltage: %.2fV", voltage);
        drawmytext(2, 22, vbuf); // this line is 10 pixels below the previous line

        ssd1306_update(); // update AFTER all drawings

        // Blink LED
        gpio_put(25, 1);
        sleep_ms(10); // blink the LED every 10ms
        gpio_put(25, 0);

        pixelBlink(0, 0); // blink the pixel at (0,0) every second

        // Debug print
        printf("ADC Value: %d\n", adc_value);
        printf("Voltage = %.2f V\n", voltage);

        sleep_ms(20); // updates at 50Hz
    }
}

void drawmytext(int x, int y, char *m) {
    int i = 0; // index of the character in the string
    int startX = x; // save the initial x position
    while (m[i] != '\0') { // loop through every character in the string
        if (x + 5 >= 128) {
            x = startX;      // Reset to initial x
            y += 8;          // Move to next row (each char is 8 pixels tall)
            if (y + 8 > 32) break; // Don't draw below the screen
        }
        drawLetter(x, y, m[i]);
        x += 6; // Move to next character (5 pixels + 1 spacing)
        i++;
    }
}



void drawLetter(int x, int y, char c) {
    int index = c - 0x20;
    // index is the index of the character in the ASCII array
    // 0x20 is the ASCII value of the space character, so we subtract it to get the index of the character in the ASCII array
    if (index < 0 || index >= 96) return; // skip unprintables
    // 96 is the number of characters in the ASCII array

    for (int i = 0; i < 5; i++) { // loop through the columns of the character
        // 5 is the width of the character in pixels
        char col = ASCII[index][i];
        for (int j = 0; j < 8; j++) { // loop through the rows of the character
            // 8 is the height of the character in pixels
            // we need to shift the column to the right by j bits and mask it with 0b1 to get the value of the pixel
            char on = (col >> j) & 0b1;
            ssd1306_drawPixel(x + i, y + j, on);
        }
    }
}

void pixelBlink(int x, int y) {
    ssd1306_drawPixel(x, y, 1); // set the pixel to 1 (white)
    ssd1306_update(); // update the screen
    sleep_ms(10);
    ssd1306_drawPixel(x, y, 0); // set the pixel to 0 (black)
    ssd1306_update();  // update the screen
}

void display_voltage(float voltage) {
    char buffer[20];
    sprintf(buffer, "Voltage: %.2fV", voltage);
    ssd1306_clear();
    drawmytext(0, 0, buffer);
    ssd1306_update();
}