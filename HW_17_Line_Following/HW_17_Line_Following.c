#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/adc.h"

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 20
#define I2C_SCL 21

void drawmytext(int x, int y, char *m);
void drawLetter(int x, int y, char c);
void pixelBlink(int x, int y);

int main()
{
    stdio_init_all();

    // I2C Initialization at 1MHz.
    i2c_init(I2C_PORT, 1000*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    // ADC initialization
    adc_init();
    adc_gpio_init(26); // GPIO 26 as ADC input
    adc_select_input(0); // ADC input 0

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        // Read ADC
        uint16_t adc_value = adc_read();
        float voltage = (adc_value * 3.3f) / 4095.0f;
        float gain = 10.0f * voltage / 3.3f; // normalized gain (0 to 1)

        // Draw to OLED
        ssd1306_clear();
        drawmytext(2, 2, "ADC Versus Gain");

        char vbuf[30];
        sprintf(vbuf, "V: %.2fV", voltage);
        drawmytext(2, 12, vbuf);

        char gbuf[30];
        sprintf(gbuf, "Gain: %.2f", gain);
        drawmytext(2, 22, gbuf);

        ssd1306_update();

        // Blink LED
        gpio_put(25, 1);
        sleep_ms(10);
        gpio_put(25, 0);

        pixelBlink(0, 0);

        // Debug print to console
        printf("ADC Value: %d\n", adc_value);
        printf("Voltage = %.2f V\n", voltage);
        printf("Gain = %.2f\n", gain);

        sleep_ms(10); // update at ~100Hz
    }
}

void drawmytext(int x, int y, char *m) {
    int i = 0;
    int startX = x;
    while (m[i] != '\0') {
        if (x + 5 >= 128) {
            x = startX;
            y += 8;
            if (y + 8 > 64) break; // updated for 64px tall display
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
