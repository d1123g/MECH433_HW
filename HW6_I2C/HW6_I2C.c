#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 21
#define I2C_SCL 22

//Tasks
//Have pin GP7 output high at 100 Hz 
//always read from pin GPI0 on the external board (not on the PICO2)
//Whenever pin GP0 on the external board reads low, write high to pin GP7 on the external board and hold it as long as pin GP0 is reading low
//when pin GP0 goes back to high turn the light back off and go back to having pin GP7 blink at high

int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    //gpio_pull_up(I2C_SDA); we do this on the hardware side
    //gpio_pull_up(I2C_SCL); we do this on the hardware side
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
