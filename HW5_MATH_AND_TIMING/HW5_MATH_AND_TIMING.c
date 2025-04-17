// Here are our standard and pico sdk headers for the input ouutput, SPI, math, and types
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include <math.h>
#include <stdint.h>

// SPI defines for the DAC
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   20
#define PIN_SCK  18
#define PIN_MOSI 19

// SPI defines for the 23k256 chip
#define SPI_PORT_m spi1
#define PIN_MISO_m 12
#define PIN_CS_m   21
#define PIN_SCK_m  10
#define PIN_MOSI_m 11

// here is our voltage reference for DAC (max output voltage)
#define VREF 3.3f

// function protyptes for math timing section.
uint64_t get_time(void);
uint64_t compute_time(uint64_t t1, uint64_t t2);

// here is our untion to convert between float and bytes 
// this is needed for the4 SPI transfer to and from the SRAM
union FloatInt {
    float f;
    uint32_t i;
};

// these are the lines from the previous homework that we use to help us communicate with the DAC
// they are needed to make sure that we instil a delay within our communication

// pulls CS low with nop delays
static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

// pulls CS high to end the SPI transaction
static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

// SRAM init
// this is where we initialize the SRAM unit for the external memory. This is done over SPI1
void spi_ram_init() {
    spi_init(SPI_PORT_m, 1000 * 1000);

    gpio_set_function(PIN_MISO_m, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_m, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_m, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_m);
    gpio_set_dir(PIN_CS_m, GPIO_OUT);
    gpio_put(PIN_CS_m, 1);

    // Set SRAM to sequential mode
    // we do this so that the address auto-increments during the reading and writing.
    cs_select(PIN_CS_m);
    uint8_t mode_seq[] = {0x01, 0x40};  // Write Mode Register, Sequential
    spi_write_blocking(SPI_PORT_m, mode_seq, 2);
    cs_deselect(PIN_CS_m);
}

// writing floats to SRAM
// converts float into 4 bytes using union. 
void write_float_to_ram(uint16_t address, float value) {
    union FloatInt num;
    num.f = value;

    // sends write command (0x02) and 16-bit address to SRAM
    cs_select(PIN_CS_m); //sets cs to low
    uint8_t cmd[3] = {0x02, (address >> 8) & 0xFF, address & 0xFF};

    spi_write_blocking(SPI_PORT_m, cmd, 3);

    // sends the 4 bytes of float data
    // we break a 32-bit float into 4 individual bytes so that it can be sent over SPI
    //add in the & 0xFF masks off everything except the lowest 8 bits of the result to make sure each byte is exactly 8 bits
    uint8_t bytes[4] = {
        (num.i >> 24) & 0xFF, // bytes[0] is the most significant byte
        (num.i >> 16) & 0xFF, // bytes[1] is the next 8 bits
        (num.i >> 8) & 0xFF, // bytes[2] is the next 8 bits after that
        num.i & 0xFF // bytes[3] is the least significant bits
    };
    spi_write_blocking(SPI_PORT_m, bytes, 4);
    cs_deselect(PIN_CS_m); // sends cs to high
}

//reading the float data from the SRAM
float read_float_from_ram(uint16_t address) {
    union FloatInt num;
    uint8_t bytes[4];

    cs_select(PIN_CS_m); //sets cs to low
    uint8_t cmd[3] = {0x03, (address >> 8) & 0xFF, address & 0xFF};
    spi_write_blocking(SPI_PORT_m, cmd, 3);

    //reads 4 bytes, then it recontstructs the float using bit shifting. 
    spi_read_blocking(SPI_PORT_m, 0x00, bytes, 4);
    cs_deselect(PIN_CS_m);

    num.i = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    return num.f;
}

// writing to dac function
void writeDac(int channel, float voltage) {
    // clamp the voltages
    if (voltage < 0) voltage = 0;
    if (voltage > VREF) voltage = VREF;

    // equation to make sure our voltage is in the right units.
    uint16_t DAC_input = (uint16_t)((voltage / VREF) * 4095.0f);

    uint16_t command = 0;
    command |= (channel & 0x01) << 15; // bit 15: channel select
    command |= (1 << 14);              // bit 14: buffer
    command |= (1 << 13);              // bit 13: gain (1x)
    command |= (1 << 12);              // bit 12: active mode
    command |= (DAC_input & 0x0FFF);   // bits 11-0: DAC value

    // sending over which channel we want to send it to and what is our array as well. 
    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS);// make cs to low
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS); // make cs to high
}

// math timing section
void math_time(void) {
    // set the f1 and f2 floats
    volatile float f1, f2;

    // prompt the user for the floats
    printf("Enter two floats to use: \n");

    // scan for the floats
    scanf("%f %f", &f1, &f2);
    //echos the numbers that we inputted for our sake
    printf("Entered Numbers: %f and %f\n", f1, f2);

    //set the volatile floats for each time we run in main.
    volatile float f_add, f_sub, f_mult, f_div;

    //set our times
    uint64_t f_add_t1, f_add_t2, f_sub_t1, f_sub_t2, f_mult_t1, f_mult_t2, f_div_t1, f_div_t2;

    // set the counter
    int i;

    // for each operation we run 1000 times and before the for loop, we get the time, and then after the 1000 loop counter, we get the time as well


    f_add_t1 = get_time();
    for (i = 1; i <= 1000; i++) {
        f_add = f1 + f2;
    }
    f_add_t2 = get_time();

    f_sub_t1 = get_time();
    for (i = 1; i <= 1000; i++) {
        f_sub = f1 - f2;
    }
    f_sub_t2 = get_time();

    f_mult_t1 = get_time();
    for (i = 1; i <= 1000; i++) {
        f_mult = f1 * f2;
    }
    f_mult_t2 = get_time();

    f_div_t1 = get_time();
    for (i = 1; i <= 1000; i++) {
        f_div = f1 / f2;
    }
    f_div_t2 = get_time();

    // get the elapsed times for each of the operations.
    uint64_t add_time = compute_time(f_add_t1, f_add_t2);
    uint64_t sub_time = compute_time(f_sub_t1, f_sub_t2);
    uint64_t mult_time = compute_time(f_mult_t1, f_mult_t2);
    uint64_t div_time = compute_time(f_div_t1, f_div_t2);

    // print the actual results of the operation
    printf("\nResults:\nAddition: %f\nSubtraction: %f\nMultiplication: %f\nDivision: %f\n",
           f_add, f_sub, f_mult, f_div);

    // print the time that elapsed
    printf("Timings (Number of Clock Cycles for an Operation):\nAdd: %llu\nSub: %llu\nMult: %llu\nDiv: %llu\n",
           add_time, sub_time, mult_time, div_time);
}

// our get time function
uint64_t get_time(void) {
    absolute_time_t t1 = get_absolute_time();
    return to_us_since_boot(t1);
}

// our computing elapsed time function
uint64_t compute_time(uint64_t t1, uint64_t t2) {
    uint64_t elapsed_time_us = t2 - t1;
    uint64_t clock_cycles = 150 * elapsed_time_us / 1000; // convert microseconds to cycles
    return clock_cycles;
}

int main() {
    stdio_init_all();

    // SPI for DAC
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // Button initializing for the math timing function
    const uint BUTTON_PIN = 2;
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    //initializng the external memory and we load the memory with our function detailing the sinwave. 
    spi_ram_init();
    for (int i = 0; i < 1000; i++) {
        float v = (sinf(2 * M_PI * ((float)i / 1000.0f)) + 1.0f) * (VREF / 2.0f);
        write_float_to_ram(i * 4, v);  // each float is 4 bytes
        sleep_us(1000); // 1 ms delay 
    }

    // here is our main loop for the math timing.
    int index = 0;
    while (true) {
        if (gpio_get(BUTTON_PIN) == 0) {
            sleep_ms(20);
            if (gpio_get(BUTTON_PIN) == 0) {
                math_time();
                while (gpio_get(BUTTON_PIN) == 0) {
                    sleep_ms(10);
                }
            }
        }

        // reading float from the ram and then write it to the dac
        float v = read_float_from_ram(index * 4);
        writeDac(0, v);
        sleep_ms(1);
        index = (index + 1) % 1000;
    }

    return 0;
}
