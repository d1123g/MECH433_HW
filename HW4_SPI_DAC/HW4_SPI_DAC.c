#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include <math.h>

// here are our spi defines, these pins corresponds to the pin number without the GPI
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   20
#define PIN_SCK  18
#define PIN_MOSI 19

// here is our DAC reference voltage
#define VREF 3.3f


// lines to help with the chip select to include delays
static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}


void writeDac(int channel, float voltage);

int main() {
    // enables either the USB or UART communication (for us we are using USB to communicate over putty)
    stdio_init_all();

    spi_init(SPI_PORT, 1000*1000); // the baud, or bits per second

    //assign the GPIO pins to the SPI hardware functions 
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    //assign the gpio pin for chip select purely in the software side (this is so we can manually toggle it)
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);

    // chip select is active-low, so we'll initialize it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS,1);

    // time variable
    float t = 0.0f;
    //step size
    const float dt_s = 0.01f; 
    const float dt = 0.02f; // use 0.01f for the sine wave to have a 2 HZ signal and use 0.02f for the triangle wave
    static bool rising = true;
    static float v1 = 0.0f;
    static float v2 = 0.0f;

    while (true) {
        // here we create a 2HZ sin wave using (2*pi*f*t)
        //because sinf toggles from -1 to 1, we need to add 1, and then scale our Vref by multiplying the amplitude by 1/2
        float v1 = (sinf(2 * M_PI * 1.0f * t) + 1.0f) * (VREF / 2.0f); // scaling from 0 to Vref
        writeDac(0, v1); // always writes to 0 (channel A)

        // // Debug print to check values (this is by using putty)
        // printf("t: %.2f, voltage: %.2f, DAC_input: %u\n", t, v, (uint16_t)((v / VREF) * 1023.0f));

        // here we create a 1HZ triangle wave using y = mx + b
        
        if (rising){
            v2 = v2+VREF*dt; // here is the rising side for the triangle
            if (v2 >= VREF){
                v2 = VREF;
                rising = false;
            }
        }
        else{
            v2 = v2 - VREF*dt;// here is the falling edge for the triangle
            if (v2 <= 0.0f){
                v2 = 0.0f;
                rising = true;
            }
        }

        writeDac(1, v2); // always writes to 0 (channel A)

        // Debug print to check values (this is by using putty)
        // printf("t: %.2f, voltage: %.2f, DAC_input: %u\n", t, v, (uint16_t)((v / VREF) * 1023.0f));

        t = t + dt;
        sleep_ms(10); // sleep every 0.01 seconds (10ms per point)

    }

    return 0;
}

void writeDac(int channel, float voltage){
    // here we clamp the voltage between 0 and VREF
    if (voltage < 0 ){
        voltage = 0;
    }
    if (voltage > VREF){
        voltage = VREF;
    }

    // convert voltage to 12-bit DAC code (0 to 4095)
    uint16_t DAC_input = (uint16_t)((voltage / VREF) * 4095.0f);

    // Construct the 16-bit command word for the DAC
    uint16_t command = 0;
    command |= (channel & 0x01) << 15; // bit 15: DACB/A selection (1 = B, 0 = A)
    command |= (1 << 14);              // bit 14: BUF (1 = buffered)
    command |= (1 << 13);              // bit 13: Gain (1 = 1x) *doesn't make sense how it can produce a voltage higher than ref
    command |= (1 << 12);              // bit 12: SHDN (1 = active mode)
    command |= (DAC_input & 0x0FFF);   // bits 11-0: 12-bit DAC input value

    // Split the command into two bytes (separate the 16 bit number into two 8 bit numbers to send)
    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    // SPI transmission
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}
