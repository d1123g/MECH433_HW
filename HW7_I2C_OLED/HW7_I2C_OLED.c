#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9




// have 128 pixels in a row, 32 pixels in a column
// x,y coordinates are in the positive x direction and "negative" y direction
// 0,0 is the top left corner of the screen
// loops through every character until you hit the null character, which sprintf sticks in there for us


int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c


    ssd1306_init();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        char m[50];
        sprintf(m, "hello");
        ssd1306_clear(); //just to make sure that it clears the screen before you write
        //if you wanted a hack you could do the following
        //sprintf(m, "hello       ");//this would write hello and then 7 spaces, which would effectively clear the screen
        drawmytext(2,7,m);
        ssd1306_update();
        //sometimes people update a certain part of the screen that is not necessary. sometimes there is a flicker
        printf("check if it has written on OLED\n");
    }
}

drawmytext(int x, int y, char *m) {
    int i = 0;
    while (m[i] != '\0') {
        drawLetter(x + i*5, y, m[i]);
        i++;
    }
}

drawLetter(int x, int y, char c) {
    // i represents the row, j represents the column
    //loop throught column in the font array
    int i = 0;
    int j = 0;
    //
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
        
            char col = ASCII[c][i];
            char on = (col >> j) & 0b1;
            ssd1306_drawPixel(x + i, y + j, on);

        }
    }
}
