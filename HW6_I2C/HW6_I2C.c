#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// ==== I2C and GPIO CONFIG ====
#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define ADDR 0x20 // MCP23008 I2C address (00100000 binary)

#define ONBOARD_LED 25
#define HEARTBEAT_LED 7

// ==== MCP23008 REGISTER ADDRESSES ====
#define IODIR 0x00
#define IPOL  0x01
#define GPIO  0x09
#define OLAT  0x0A

// ==== Function Prototypes ====
void setPin(uint8_t address, uint8_t reg, uint8_t value);
uint8_t readPin(uint8_t address, uint8_t reg);
void initialize_gpio_chip_extender();
void LED_Blink();

//gpi7 is still on, needs to fix that
int main() {
    stdio_init_all();
    sleep_ms(1000); // Give some time for USB to connect

    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    //gpio_pull_up(I2C_SDA); we do this on the hardware side
    //gpio_pull_up(I2C_SCL); we do this on the hardware side

    // Initialize heartbeat LED
    gpio_init(HEARTBEAT_LED);
    gpio_set_dir(HEARTBEAT_LED, GPIO_OUT);
    gpio_put(HEARTBEAT_LED, 1);

    // Initialize onboard LED
    gpio_init(ONBOARD_LED);
    gpio_set_dir(ONBOARD_LED, GPIO_OUT);
    gpio_put(ONBOARD_LED, 1);

    // Initialize MCP23008
    initialize_gpio_chip_extender();

    uint64_t start_time = to_ms_since_boot(get_absolute_time());

    while (true) {
        LED_Blink(); // Control GP7 based on GP0 state

        // Toggle onboard LED every 500ms as a heartbeat
        if (to_ms_since_boot(get_absolute_time()) - start_time > 500) {
            start_time = to_ms_since_boot(get_absolute_time());
            gpio_put(ONBOARD_LED, !gpio_get(ONBOARD_LED));
            print("Lub Dub\n") //printing heartbeat pun
        }
    }
}

// ==== I2C Helper Functions ====
void setPin(uint8_t address, uint8_t reg, uint8_t value) {
    uint8_t buf[] = {reg, value};
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

uint8_t readPin(uint8_t address, uint8_t reg) {
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);
    uint8_t value;
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);
    return value;
}

// ==== MCP23008 Initialization ====
void initialize_gpio_chip_extender() {
    // GP0 = input, GP7 = output
    setPin(ADDR, IODIR, 0b00000001); // GP0 input, GP7 output
    setPin(ADDR, IPOL,  0b00000000); // Normal polarity
    setPin(ADDR, OLAT,  0b10000000); // GP7 HIGH (LED off)

    sleep_ms(500); // Small delay

    setPin(ADDR, OLAT, 0b00000000); // GP7 LOW (LED on briefly)
    sleep_ms(500);
    setPin(ADDR, OLAT, 0b10000000); // GP7 HIGH (LED off)
}

// ==== Main I/O Loop ====
void LED_Blink() {
    uint8_t button_state = readPin(ADDR, GPIO);

    if (!(button_state & 0b00000001)) {
        // GP0 LOW (button pressed) → turn ON GP7 (LED ON = LOW)
        setPin(ADDR, OLAT, 0b00000000);
        printf("Button Pressed: LED ON\n");
    } else {
        // GP0 HIGH (button not pressed) → turn OFF GP7 (LED OFF = HIGH)
        setPin(ADDR, OLAT, 0b10000000);
        printf("Button Released: LED OFF\n");
    }
}
