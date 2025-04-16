#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include <math.h>

// SPI defines
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   20
#define PIN_SCK  18
#define PIN_MOSI 19

// DAC reference voltage
#define VREF 3.3f

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

// Function declarations
void writeDac(int channel, float voltage);
uint64_t get_time(void);
void math_time(void);
uint64_t compute_time(uint64_t t1, uint64_t t2);

int main() {
    stdio_init_all();
    spi_init(SPI_PORT, 1000 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // Initialize button on GPIO 2
    const uint BUTTON_PIN = 2;
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); // assumes active-low button to GND

    // DAC waveform
    float t = 0.0f;
    const float dt = 0.01f;

    while (true) {
        // Run math_time on button press (with debounce)
        if (gpio_get(BUTTON_PIN) == 0) {
            sleep_ms(20); // debounce
            if (gpio_get(BUTTON_PIN) == 0) {
                math_time(); // Run once
                while (gpio_get(BUTTON_PIN) == 0) {
                    sleep_ms(10); // wait for release
                }
            }
        }

        // 2 Hz sine wave DAC output
        float v1 = (sinf(2 * M_PI * 2.0f * t) + 1.0f) * (VREF / 2.0f);
        writeDac(0, v1);
        t += dt;
        sleep_ms(10);
    }

    return 0;
}

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
    uint64_t clock_cycles =  150*elapsed_time_us/1000; // convert microseconds to cycles
    return clock_cycles;
}
