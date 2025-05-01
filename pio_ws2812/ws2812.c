/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include <stdio.h>
 #include <stdlib.h>
 
 #include "pico/stdlib.h"
 #include "hardware/pio.h"
 #include "hardware/clocks.h"
 #include "ws2812.pio.h"
 
 #include "hardware/pwm.h"
 /**
  * NOTE:
  *  Take into consideration if your WS2812 is a RGB or RGBW variant.
  *
  *  If it is RGBW, you need to set IS_RGBW to true and provide 4 bytes per 
  *  pixel (Red, Green, Blue, White) and use urgbw_u32().
  *
  *  If it is RGB, set IS_RGBW to false and provide 3 bytes per pixel (Red,
  *  Green, Blue) and use urgb_u32().
  *
  *  When RGBW is used with urgb_u32(), the White channel will be ignored (off).
  *
  */
 #define IS_RGBW false
 #define NUM_PIXELS 150
 
 #ifdef PICO_DEFAULT_WS2812_PIN
 #define WS2812_PIN PICO_DEFAULT_WS2812_PIN
 #else
 // default to pin 2 if the board doesn't have a default WS2812 pin defined
 #define WS2812_PIN 5
 #endif
 
 // Check the pin is compatible with the platform
 #if WS2812_PIN >= NUM_BANK0_GPIOS
 #error Attempting to use a pin>=32 on a platform that does not support it
 #endif
 
 // following defines are for the rc servo
 #define SERVO_PIN 0
 #define SERVO_MIN_PULSE_WIDTH_US 500
 #define SERVO_MAX_PULSE_WIDTH_US 2500
 #define SERVO_FREQ_HZ 50
 #define CLOCK_DIVIDER 64.0f
 #define SYS_CLOCK_HZ 150000000
 
 
 uint16_t pwm_wrap = 0;
 float servo_angle = 0.0f;
 float servo_direction = 0.5f;
 
 uint16_t pulse_us_to_level(uint16_t us) {
     float ticks_per_us = (SYS_CLOCK_HZ / CLOCK_DIVIDER) / 1e6f;
     return (uint16_t)(us * ticks_per_us + 0.5f);
 }
 
 void servo_pwm_init(uint pin) {
     gpio_set_function(pin, GPIO_FUNC_PWM);
     uint slice_num = pwm_gpio_to_slice_num(pin);
     pwm_wrap = (uint16_t)(SYS_CLOCK_HZ / (CLOCK_DIVIDER * SERVO_FREQ_HZ)) - 1;
     pwm_set_clkdiv(slice_num, CLOCK_DIVIDER);
     pwm_set_wrap(slice_num, pwm_wrap);
     pwm_set_enabled(slice_num, true);
 }
 
 void servo_set_angle(uint pin, float angle) {
     if (angle < 0) angle = 0;
     if (angle > 180) angle = 180;
     float pulse_width = SERVO_MIN_PULSE_WIDTH_US + (angle / 180.0f) * (SERVO_MAX_PULSE_WIDTH_US - SERVO_MIN_PULSE_WIDTH_US);
     uint16_t level = pulse_us_to_level((uint16_t)pulse_width);
     pwm_set_gpio_level(pin, level);
 }
 
 static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
     pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
 }
 
 static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
     return
             ((uint32_t) (r) << 8) |
             ((uint32_t) (g) << 16) |
             (uint32_t) (b);
 }
 
 static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
     return
             ((uint32_t) (r) << 8) |
             ((uint32_t) (g) << 16) |
             ((uint32_t) (w) << 24) |
             (uint32_t) (b);
 }
 
 void pattern_snakes(PIO pio, uint sm, uint len, uint t) {
     for (uint i = 0; i < len; ++i) {
         uint x = (i + (t >> 1)) % 64;
         if (x < 10)
             put_pixel(pio, sm, urgb_u32(0xff, 0, 0));
         else if (x >= 15 && x < 25)
             put_pixel(pio, sm, urgb_u32(0, 0xff, 0));
         else if (x >= 30 && x < 40)
             put_pixel(pio, sm, urgb_u32(0, 0, 0xff));
         else
             put_pixel(pio, sm, 0);
     }
 }
 
 void pattern_random(PIO pio, uint sm, uint len, uint t) {
     if (t % 8)
         return;
     for (uint i = 0; i < len; ++i)
         put_pixel(pio, sm, rand());
 }
 
 void pattern_sparkle(PIO pio, uint sm, uint len, uint t) {
     if (t % 8)
         return;
     for (uint i = 0; i < len; ++i)
         put_pixel(pio, sm, rand() % 16 ? 0 : 0xffffffff);
 }
 
 void pattern_greys(PIO pio, uint sm, uint len, uint t) {
     uint max = 100; // let's not draw too much current!
     t %= max;
     for (uint i = 0; i < len; ++i) {
         put_pixel(pio, sm, t * 0x10101);
         if (++t >= max) t = 0;
     }
 }
 
 // This pattern will make the first 4 LEDs blue and the rest random colors
 // our hw assignment
 void pattern_blue_walk(PIO pio, uint sm, uint len, uint t) {
     // Only use the first 4 LEDs
     int blue_index = (t / 158) % 4;  // move every 500ms if loop is every 10ms
 
     for (int i = 0; i < len; ++i) {
         if (i < 4) {
             if (i == blue_index)
                 put_pixel(pio, sm, urgb_u32(0, 0, 255));  // solid blue
             else
                 put_pixel(pio, sm, urgb_u32(rand() % 256, rand() % 256, rand() % 256));  // random colors
         } else {
             put_pixel(pio, sm, 0);  // turn off unused LEDs
         }
     }
 }
 
 
 typedef void (*pattern)(PIO pio, uint sm, uint len, uint t);
 const struct {
     pattern pat;
     const char *name;
 } pattern_table[] = {
         // {pattern_snakes,  "Snakes!"},
         // {pattern_random,  "Random data"},
         // {pattern_sparkle, "Sparkles"},
         // {pattern_greys,   "Greys"},
         {pattern_blue_walk, "Blue walker"},
 };
 
 int main() {
     //set_sys_clock_48();
     stdio_init_all();
     servo_pwm_init(SERVO_PIN);
     printf("WS2812 Smoke Test, using pin %d\n", WS2812_PIN);
 
     // todo get free sm
     PIO pio;
     uint sm;
     uint offset;
 
     // This will find a free pio and state machine for our program and load it for us
     // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
     // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
     bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
     hard_assert(success);
 
     ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
 
     int t = 0;
     while (1) {
         int pat = rand() % count_of(pattern_table);
         int dir = (rand() >> 30) & 1 ? 1 : -1;
         puts(pattern_table[pat].name);
         puts(dir == 1 ? "(forward)" : "(backward)");
         for (int i = 0; i < 1000; ++i) {
             pattern_table[pat].pat(pio, sm, NUM_PIXELS, t);
             sleep_ms(10);
             t += dir;
 
             // Update servo angle slowly every frame
             servo_set_angle(SERVO_PIN, servo_angle);
             servo_angle += servo_direction;
 
             if (servo_angle >= 180.0f) {
                 servo_angle = 180.0f;
                 servo_direction = -0.5f;
             } else if (servo_angle <= 0.0f) {
                 servo_angle = 0.0f;
                 servo_direction = 0.5f;
             }
 
         }
     }
 
     // This will free resources and unload our program
     pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
 }
 