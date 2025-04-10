#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define GPIO_BUTTON_PIN 2      // Button GPIO 
#define GPIO_LED_PIN 15        // LED GPIO 
#define DEBOUNCE_US 10000     // 100 ms debounce time

static uint64_t last_interrupt_time = 0;

void button_press(uint gpio, uint32_t events){
    uint64_t current_time = time_us_64();
    
    if (current_time - last_interrupt_time > DEBOUNCE_US) {
        if (events & GPIO_IRQ_EDGE_RISE) {
            gpio_put(GPIO_LED_PIN, 0);  // turn LED off
        }
        last_interrupt_time = current_time;
    }
    printf("Enter A Number of Analog Samples you wish to take. Keep it between 1 and 100\n");
}

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    gpio_init(GPIO_LED_PIN);
    gpio_set_dir(GPIO_LED_PIN, GPIO_OUT);
    gpio_pull_up(GPIO_LED_PIN);

    gpio_init(GPIO_BUTTON_PIN);
    gpio_set_dir(GPIO_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(GPIO_BUTTON_PIN);

    gpio_put(GPIO_LED_PIN, 1);  // turn LED on

    adc_init();
    adc_gpio_init(26);  // ADC0 is on GPIO26
    adc_select_input(0);

    gpio_set_irq_enabled_with_callback(GPIO_BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &button_press);

    while(1) {
        int num_samples = 0;
        printf("Waiting for number of samples you wish to take...\n"); 

        scanf("%d", &num_samples);

        if (num_samples >= 1 && num_samples <= 100) {
            printf("Taking %d samples:\n", num_samples); 
            for (int i = 0; i < num_samples; i++) {
                uint16_t result = adc_read();
                printf("Sample %d: %u\n", i + 1, result);  
                sleep_ms(100);  // delay to not overwhelm serial
            }
        } else {
            printf("Invalid number. Please enter a number in between 1 and 100 por favor.\n");
        }

        sleep_ms(100);  // delay to not overwhelm serial
        gpio_put(GPIO_LED_PIN, 1);  // turn LED on
    }
}
