#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#define GPIO_WATCH_PIN 2      // Button GPIO 
#define GPIO_LED_PIN 15       // LED GPIO 
#define DEBOUNCE_US 50000     // 50 ms debounce time

static uint64_t last_interrupt_time = 0;
// variable to keep track of the press count in the current section
static uint32_t press_count = 0;  

// GPIO callback function
void gpio_callback(uint gpio, uint32_t events) {
    uint64_t current_time = time_us_64();
    
    // debouncing logic
    // only process the interrupt if the button has been held longer than DEBOUNCE_US
    if (current_time - last_interrupt_time > DEBOUNCE_US) {
        // if the button is pressed with the debouncing logic, it will trigger an EDGE_RISE
        // increment the counter and blink the LED
        if (events & GPIO_IRQ_EDGE_RISE) {
            press_count++;
            printf("Button pressed %d times\n", press_count);
            
            // blink the LED (toggle GPIO15)
            gpio_put(GPIO_LED_PIN, 1);  // turn LED on
            sleep_ms(100);               // wait for 100ms
            gpio_put(GPIO_LED_PIN, 0);  // turn LED off
        }
        
        last_interrupt_time = current_time;
    }
}

int main() {
    stdio_init_all();
    
    // debugging message to indicate that the program has started
    printf("Hello GPIO IRQ\n");

    // initialize the button pin as the input
    gpio_init(GPIO_WATCH_PIN);
    gpio_set_dir(GPIO_WATCH_PIN, GPIO_IN);
    // here we add in an internal pull-up resistor along with our physical resistor to ensure that there are no floatin voltages (probably don't need)
    gpio_pull_up(GPIO_WATCH_PIN); // Enable internal pull-up resistor

    // initialize the LED pin as output
    gpio_init(GPIO_LED_PIN);
    gpio_set_dir(GPIO_LED_PIN, GPIO_OUT);
    gpio_put(GPIO_LED_PIN, 0);  // ensure the LED is initially off

    // set up the interrupt for rising edges (button press detection)
    gpio_set_irq_enabled_with_callback(GPIO_WATCH_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // have the code wait forever to keep track of the number of times we have pressed the button
    while (1);
}
