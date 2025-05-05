/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#define CMD_GET_ADC 0 // command to get the adc value
#define CMD_LED_ON 1 // command to turn on the led
#define CMD_LED_OFF 2 // command to turn off the led
#define DONE_FLAG 3 // flag to indicate that the command is done

volatile uint32_t adc_value = 0; // variable to store the adc value

// core 1 functions -------------
void init_peripherals() {
    adc_init(); // initalize the adc
    adc_gpio_init(26); // initalize the gpio pin 26
    gpio_init(15); // initalize the gpio pin 15
    gpio_set_dir(15, GPIO_OUT); // set the gpio pin 15 as output
}

void handle_command(uint32_t cmd) {
    switch (cmd) {
        case CMD_GET_ADC: 
            adc_select_input(0); // select the adc input 0
            adc_value = adc_read(); // read the adc value
            break;
        case CMD_LED_ON:
            gpio_put(15, 1); // turn on the led
            break;
        case CMD_LED_OFF:
            gpio_put(15, 0); // turn off the led
            break;
        }
        multicore_fifo_push_blocking(DONE_FLAG); // write back to core 0
    }

// og core 1 entry function
// void core1_entry() {
//     printf("Hello from core 1!\n"); // print the message
//     multicore_fifo_push_blocking(FLAG_VALUE); //write back to core 0

//     uint32_t g = multicore_fifo_pop_blocking(); //read from core 0

//     if (g != FLAG_VALUE)
//         printf("Hmm, that's not right on core 1!\n");
//     else
//         printf("Its all gone well on core 1!");

//     while (1)
//         tight_loop_contents(); // do nothing forever
// }

void core1_entry() {
    init_peripherals(); // initialize the peripherals
    while (1) {
        uint32_t cmd = multicore_fifo_pop_blocking(); // read the command from core 0
        handle_command(cmd); // handle the command
    }
}

// core 0 functions -------------

void send_command(uint32_t cmd) {
    multicore_fifo_push_blocking(cmd); // write the command to core 1
}

void wait_for_done() {
    uint32_t done_flag = multicore_fifo_pop_blocking(); // read the done flag from core 1
    if (done_flag != DONE_FLAG) { // check if the done flag is not equal to the done flag
        printf("Error: command not done\n"); // print the error message
    }
}

void handle_user_input(int ch) {
    if (ch >=0 && ch <= 2) { // check if the input is valid
        send_command(ch); // send the command to core 1
        wait_for_done(); // wait for the command to be done

        switch (ch) {
            case CMD_GET_ADC: // if the command is to get the adc value
                float voltage = adc_value * 3.3f / (4095.0f); // convert the adc value to voltage
                printf("ADC voltage: %0.2f V\n", voltage); // print the adc value
                break;
            case CMD_LED_ON: // if the command is to turn on the led
                printf("LED is ON\n"); // print the message
                break;
            case CMD_LED_OFF: // if the command is to turn off the led
                printf("LED is OFF\n"); // print the message
                break;
        }

    } else {
        printf("Error: invalid input\n"); // print the error message
    }

}

// og main
// int main() {
//     stdio_init_all();
//     printf("Hello, multicore!\n");

//     /// \tag::setup_multicore[]

//     multicore_launch_core1(core1_entry);

//     // Wait for it to start up

//     uint32_t g = multicore_fifo_pop_blocking(); //read from core 1

//     if (g != FLAG_VALUE)
//         printf("Hmm, that's not right on core 0!\n");
//     else {
//         multicore_fifo_push_blocking(FLAG_VALUE);
//         printf("It's all gone well on core 0!");
//     }

//     /// \end::setup_multicore[]
// }

int main() {
    stdio_init_all(); // initialize the stdio
    sleep_ms(1000); // wait for 1 second
    printf("Core 0 ready. Enter 0 (ADC), 1 (LED ON) or 2 (LED OFF):\n"); // print the message

    multicore_launch_core1(core1_entry); // launch core 1

    while (1){
        int ch;
        if (scanf("%d", &ch) == 1) { // read the input from the user
            handle_user_input(ch); // handle the user input
        } else {
            printf("Error: invalid input\n"); // print the error message
            while (getchar() != '\n'); // clear the input buffer
        }
    }
}