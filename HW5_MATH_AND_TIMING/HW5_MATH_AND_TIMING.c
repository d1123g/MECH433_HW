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

#define VREF 3.3f

// Function Prototypes
uint64_t get_time(void);
uint64_t compute_time(uint64_t t1, uint64_t t2);

// Union for float-byte conversion
union FloatInt {
    float f;
    uint32_t i;
};

// Chip select helpers
static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

// ==== SRAM SPI INIT FUNCTION ====
void spi_ram_init() {
    spi_init(SPI_PORT_m, 1000 * 1000);

    gpio_set_function(PIN_MISO_m, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_m, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_m, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_m);
    gpio_set_dir(PIN_CS_m, GPIO_OUT);
    gpio_put(PIN_CS_m, 1);

    // Set SRAM to sequential mode
    cs_select(PIN_CS_m);
    uint8_t mode_seq[] = {0x01, 0x40};  // Write Mode Register, Sequential
    spi_write_blocking(SPI_PORT_m, mode_seq, 2);
    cs_deselect(PIN_CS_m);
}

// ==== SRAM WRITE FLOAT ====
void write_float_to_ram(uint16_t address, float value) {
    union FloatInt num;
    num.f = value;

    cs_select(PIN_CS_m);
    uint8_t cmd[3] = {0x02, (address >> 8) & 0xFF, address & 0xFF};
    spi_write_blocking(SPI_PORT_m, cmd, 3);

    uint8_t bytes[4] = {
        (num.i >> 24) & 0xFF,
        (num.i >> 16) & 0xFF,
        (num.i >> 8) & 0xFF,
        num.i & 0xFF
    };
    spi_write_blocking(SPI_PORT_m, bytes, 4);
    cs_deselect(PIN_CS_m);
}

// ==== SRAM READ FLOAT ====
float read_float_from_ram(uint16_t address) {
    union FloatInt num;
    uint8_t bytes[4];

    cs_select(PIN_CS_m);
    uint8_t cmd[3] = {0x03, (address >> 8) & 0xFF, address & 0xFF};
    spi_write_blocking(SPI_PORT_m, cmd, 3);
    spi_read_blocking(SPI_PORT_m, 0x00, bytes, 4);
    cs_deselect(PIN_CS_m);

    num.i = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    return num.f;
}

// ==== DAC WRITE FUNCTION ====
void writeDac(int channel, float voltage) {
    if (voltage < 0) voltage = 0;
    if (voltage > VREF) voltage = VREF;

    uint16_t DAC_input = (uint16_t)((voltage / VREF) * 4095.0f);

    uint16_t command = 0;
    command |= (channel & 0x01) << 15; // bit 15: channel select
    command |= (1 << 14);              // bit 14: buffer
    command |= (1 << 13);              // bit 13: gain (1x)
    command |= (1 << 12);              // bit 12: active mode
    command |= (DAC_input & 0x0FFF);   // bits 11-0: DAC value

    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

// ==== MATH TIMING ====
void math_time(void) {
    volatile float f1, f2;
    printf("Enter two floats to use: \n");
    scanf("%f %f", &f1, &f2);
    printf("Entered Numbers: %f and %f\n", f1, f2);

    volatile float f_add, f_sub, f_mult, f_div;
    uint64_t f_add_t1, f_add_t2, f_sub_t1, f_sub_t2, f_mult_t1, f_mult_t2, f_div_t1, f_div_t2;
    int i;

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

    uint64_t add_time = compute_time(f_add_t1, f_add_t2);
    uint64_t sub_time = compute_time(f_sub_t1, f_sub_t2);
    uint64_t mult_time = compute_time(f_mult_t1, f_mult_t2);
    uint64_t div_time = compute_time(f_div_t1, f_div_t2);

    printf("\nResults:\nAddition: %f\nSubtraction: %f\nMultiplication: %f\nDivision: %f\n",
           f_add, f_sub, f_mult, f_div);
    printf("Timings (Number of Clock Cycles for an Operation):\nAdd: %llu\nSub: %llu\nMult: %llu\nDiv: %llu\n",
           add_time, sub_time, mult_time, div_time);
}

uint64_t get_time(void) {
    absolute_time_t t1 = get_absolute_time();
    return to_us_since_boot(t1);
}

uint64_t compute_time(uint64_t t1, uint64_t t2) {
    uint64_t elapsed_time_us = t2 - t1;
    uint64_t clock_cycles = 150 * elapsed_time_us / 1000; // convert microseconds to cycles
    return clock_cycles;
}

// ==== MAIN ====
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

    // Button init
    const uint BUTTON_PIN = 2;
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    // ==== INIT SRAM & LOAD SINE ====
    spi_ram_init();
    for (int i = 0; i < 1000; i++) {
        float v = (sinf(2 * M_PI * ((float)i / 1000.0f)) + 1.0f) * (VREF / 2.0f);
        write_float_to_ram(i * 4, v);  // each float is 4 bytes
    }

    // ==== MAIN LOOP ====
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

        // 1Hz sine wave from SRAM
        float v = read_float_from_ram(index * 4);
        writeDac(0, v);
        sleep_ms(1);
        index = (index + 1) % 1000;
    }

    return 0;
}
