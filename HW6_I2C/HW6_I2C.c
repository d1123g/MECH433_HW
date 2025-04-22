#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// ==== I2C and GPIO CONFIG ====
#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define ADDR 0x20 // MCP23008 I2C address (00100000 binary)

#define HEARTBEAT_LED 7

#define DEBOUNCE 250

// ==== MCP23008 REGISTER ADDRESSES ====
#define IODIR 0x00
#define IPOL  0x01
#define GPIO  0x09
#define OLAT  0x0A

#define HEARTBEAT_TIME 250

// ==== MCP23008 Pin Names ====
#define GP0 0
#define GP6 6
#define GP7 7

// ==== Global variable to track OLAT state ====
uint8_t olat_state = 0x00;

// ==== Function Prototypes ====
void setPin(uint8_t address, uint8_t reg, uint8_t value);
uint8_t readPin(uint8_t address, uint8_t reg);
void write_olat();
void set_olat_bit(int pin, bool high);
void initialize_gpio_chip_extender();
void LED_Blink();
void LED_HEARTBEAT();

int main() {
    stdio_init_all();
    // we give some time for USB to connect
    sleep_ms(1000); 

    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // normally here we would put our pull up resistors in the pic, but we did it on the hardware side, so we are good. 

    // Initialize MCP23008
    initialize_gpio_chip_extender();

    //we use the start time to help us in our debouncing
    uint64_t start_time = to_ms_since_boot(get_absolute_time());

    while (true) {
        //we control the GP7 external pin based on when we push a button as an input for GP0
        LED_Blink();   
        //we blink GP6 at a regular interval.     
        LED_HEARTBEAT();  

        //we debounce and print out a heart beat pun (which is lub dub)
        if (to_ms_since_boot(get_absolute_time()) - start_time > DEBOUNCE) {
            start_time = to_ms_since_boot(get_absolute_time());
            printf("Lub Dub\n");
        }
    }
}

// ==== I2C Helper Functions ====

// here we write to our pin
void setPin(uint8_t address, uint8_t reg, uint8_t value) {
    uint8_t buf[] = {reg, value};
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

// here we read our pin input
uint8_t readPin(uint8_t address, uint8_t reg) {
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);
    uint8_t value;
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);
    return value;
}

// ==== Write updated OLAT value ===
//here we initialize our OLAT function to able to write
void write_olat() {
    setPin(ADDR, OLAT, olat_state);
}

// ==== Set/Clear a bit in OLAT and write ====
void set_olat_bit(int pin, bool high) {
    if (high) {
        olat_state |= (1 << pin);
    } else {
        olat_state &= ~(1 << pin);
    }
    write_olat();
}

// ==== MCP23008 Initialization ====
void initialize_gpio_chip_extender() {
    // we set the following: GP0 = input, GP1-GP7 = output
    setPin(ADDR, IODIR, 0b00000001); 
    // No polarity inversion
    setPin(ADDR, IPOL,  0x00);      

    olat_state = 0x00;
    // Initialize OLAT
    write_olat(); 

    // Blink GP6 and GP7 once as a way to initialize
    set_olat_bit(GP6, 1);
    set_olat_bit(GP7, 1);
    sleep_ms(500);
    set_olat_bit(GP6, 0);
    set_olat_bit(GP7, 0);
    sleep_ms(500);
}

// ==== Blink LED on GP7 based on Button on GP0 ====
void LED_Blink() {
    //initilaize a variable state to read the state of the pin
    //we will call it button_state because it will depend on our button state
    uint8_t button_state = readPin(ADDR, GPIO);

    // rememeber that we are shorting our circuit when we are pressing it  
    if (!(button_state & (1 << GP0))) {
        // Button pressed (GP0 LOW) LED ON (GP7 LOW)
        set_olat_bit(GP7, 0);

        
        printf("Button Pressed: LED ON\n");
    } else {
        // Button not pressed (GP0 HIGH) LED OFF (GP7 HIGH)
        set_olat_bit(GP7, 1);

        //print so that we can debug on putty
        printf("Button Released: LED OFF\n");
    }
}

// ==== Blink GP6 as heartbeat ====

// blinking the led so that we can make sure that it make sure that there is I2C communication
void LED_HEARTBEAT() {
    set_olat_bit(GP6, 0); // GP6 LOW LED OFF
    sleep_ms(HEARTBEAT_TIME);
    set_olat_bit(GP6, 1); // GP6 HIGH LED ON
    sleep_ms(HEARTBEAT_TIME);
}
