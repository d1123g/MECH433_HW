#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

//initialize pins to be used as general purpouse IO
#define GPIO_BUTTON_PIN 2      // Button GPIO 
#define GPIO_LED_PIN 15       // LED GPIO 
#define DEBOUNCE_US 100000     // 100 ms debounce time

// variable to store the last time or interrupt
static uint64_t last_interrupt_time = 0;

// variable to keep track of the press count in the current section
static uint32_t press_count = 0;

void button_press(uint gpio, uint32_t events){

    uint64_t current_time = time_us_64();
    
    // debouncing logic
    // only process the interrupt if the button has been held longer than DEBOUNCE_US
    if (current_time - last_interrupt_time > DEBOUNCE_US) {
        // if the button is pressed with the debouncing logic, it will trigger an EDGE_RISE
        // increment the counter and blink the LED
        if (events & GPIO_IRQ_EDGE_RISE) {
            press_count++;
            printf("Button pressed %d times\n", press_count);
            
            // turn off the LED
            gpio_put(GPIO_LED_PIN, 0);  // turn LED off
        }
        
        last_interrupt_time = current_time;
    }
    printf("Enter A Number of Analog Samples you wish to take. Keep it between 1 and 100\n");

}

int main() {
    stdio_init_all();

    // wait until the usb is connected 
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    //initialize GPIO pin output (LED)
    gpio_init(GPIO_LED_PIN); // PIN_NUM without the GP
    gpio_set_dir(GPIO_LED_PIN, GPIO_OUT); //set direction of the pin
    gpio_pull_up(GPIO_LED_PIN);

    //initialize GPIO pin input
    gpio_init(GPIO_BUTTON_PIN);
    gpio_set_dir(GPIO_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(GPIO_BUTTON_PIN);

    gpio_put(GPIO_LED_PIN, 1);  // turn LED on

    //initialize the ADC
    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0

    // set up the interrupt for rising edges (button press detection)
    gpio_set_irq_enabled_with_callback(GPIO_BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &button_press);

    while(1) {
        int num_samples = 0;
        printf("Waiting for number of samples you wish to take...")
        scanf("%d", &num_samples); //apply the number that we inputted into the num_samples variable
        
        char message[100];
        scanf("%s", message);
        printf("message: %s\r\n",message);
        sleep_ms(50);
    }
}
