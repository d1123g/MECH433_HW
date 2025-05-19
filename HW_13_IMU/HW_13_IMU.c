#include <stdio.h>
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "hardware/i2c.h"
#include "math.h"
#include "font.h"

// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75

// I2C address of the MPU6050
#define I2C_PORT i2c0
#define SDA_PIN1 8
#define SCL_PIN1 9
#define SDA_PIN2 21
#define SCL_PIN2 22

#define MPU6050_ADDRESS 0x68

// Led pin
#define LED_BUILTIN 25

// OLED defines
#define C 30 // Scale factor for the arrow length
#define x_center 64 // Center x-coordinate of the OLED display
#define y_center 16 // Center y-coordinate of the OLED display



// to turn on the chip write 0x00 to PWR_MGMT_1 register to turn on the chip
// to turn off the chip write 0x40 to PWR_MGMT_1 register to turn off the chip

// to enable the accelerometer write to the ACCEL_CONFIG register.
// Set the sensitivity to 2g by writing 0x00 to the ACCEL_CONFIG register.

// to enable the gyroscope write to the GYRO_CONFIG register.
// Set the sensitivity to 2000 dps by writing 0x18 to the GYRO_CONFIG register.

void i2c_write(uint8_t address, uint8_t reg, uint8_t value);

// initialize the mpu6050
void initialize_MPU6050()
{
    // Set the PWR_MGMT_1 register to 0x00 to wake up the MPU6050
    i2c_write(MPU6050_ADDRESS, PWR_MGMT_1, 0x00);
    // Set the ACCEL_CONFIG register to 0x00 to set the accelerometer sensitivity to 2g
    i2c_write(MPU6050_ADDRESS, ACCEL_CONFIG, 0x00);
    // Set the GYRO_CONFIG register to 0x18 to set the gyroscope sensitivity to 2000 dps
    i2c_write(MPU6050_ADDRESS, GYRO_CONFIG, 0x18);
}

// initialize the pins
void initialize_pins()
{
    // Initialize the LED pin
    gpio_init(LED_BUILTIN);
    gpio_set_dir(LED_BUILTIN, GPIO_OUT);
    gpio_put(LED_BUILTIN, 0); // turn off the LED

    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000); // 400kHz
    gpio_set_function(SDA_PIN1, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN1, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN1);
    gpio_pull_up(SCL_PIN1);

    gpio_set_function(SDA_PIN2, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN2, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN2);
    gpio_pull_up(SCL_PIN2);
}

// ==== I2C Helper Functions ====

// here we write to our pin
void i2c_write(uint8_t address, uint8_t reg, uint8_t value) {
    uint8_t buf[] = {reg, value};
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

// here we read our pin input
uint8_t i2c_read(uint8_t address, uint8_t reg) {
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);
    uint8_t value;
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);
    return value;
}

// error loop 
void error_loop() {
    gpio_put(LED_BUILTIN, 1); // turn on the LED
    while (1) {
        sleep_ms(1000);
    }
}

// ====OLED Functions====

void draw_arrow(float accel_x, float accel_y) {
    ssd1306_clear();

    int x0 = x_center;
    int y0 = y_center;

    int x1 = x0 + accel_x;
    int y1 = y0 - accel_y; // Invert Y-axis

    // Draw main arrow line
    ssd1306_draw_line(x0, y0, x1, y1, 1);

    // ==== Arrowhead ====
    float angle = atan2(y1 - y0, x1 - x0); // Angle of the arrow
    float arrow_length = 5.0;              // Length of each side of the arrowhead
    float arrow_angle = M_PI / 6;          // 30 degrees

    // Points for left side of arrowhead
    int x2 = x1 - (int)(arrow_length * cos(angle - arrow_angle));
    int y2 = y1 - (int)(arrow_length * sin(angle - arrow_angle));

    // Points for right side of arrowhead
    int x3 = x1 - (int)(arrow_length * cos(angle + arrow_angle));
    int y3 = y1 - (int)(arrow_length * sin(angle + arrow_angle));

    // Draw the two arrowhead lines
    ssd1306_draw_line(x1, y1, x2, y2, 1);
    ssd1306_draw_line(x1, y1, x3, y3, 1);
}

void drawLetter(int x, int y, char c) {
    int index = c - 0x20; // 0x20 is the ASCII value of the space character
    // index is the index of the character in the ASCII array
    // 0x20 is the ASCII value of the space character, so we subtract it to get the index of the character in the ASCII array
    if (index < 0 || index >= 96) return; // skip unprintables
    // 96 is the number of characters in the ASCII array

    for (int i = 0; i < 5; i++) { // loop through the columns of the character
        // 5 is the width of the character in pixels
        char col = ASCII[index][i];
        for (int j = 0; j < 8; j++) { // loop through the rows of the character
            // 8 is the height of the character in pixels
            // we need to shift the column to the right by j bits and mask it with 0b1 to get the value of the pixel
            char on = (col >> j) & 0b1;
            ssd1306_drawPixel(x + i, y + j, on);
        }
    }
}

void drawmytext(int x, int y, char *m) {
    int i = 0; // index of the character in the string
    int startX = x; // save the initial x position
    while (m[i] != '\0') { // loop through every character in the string
        if (x + 5 >= 128) {
            x = startX;      // Reset to initial x
            y += 8;          // Move to next row (each char is 8 pixels tall)
            if (y + 8 > 32) break; // Don't draw below the screen
        }
        drawLetter(x, y, m[i]);
        x += 6; // Move to next character (5 pixels + 1 spacing)
        i++;
    }
}

int main()
{
    sleep_ms(10000);
    stdio_init_all();
    initialize_pins();
    ssd1306_setup(); // Initialize the OLED display
    ssd1306_clear();
    ssd1306_update();
    uint8_t who_am_i = i2c_read(MPU6050_ADDRESS, WHO_AM_I);
    printf("WHO_AM_I: 0x%02X\n", who_am_i);
    if (who_am_i != 0x68 && who_am_i != 0x98) {
        printf("Error: MPU6050 not found\n");
        error_loop();
    }
    printf("MPU6050 found\n");
    initialize_MPU6050();

    printf("Reading sensor data...\n");
    while (1) {
        // Read raw accel values
        int16_t accel_x = (i2c_read(MPU6050_ADDRESS, ACCEL_XOUT_H) << 8) | i2c_read(MPU6050_ADDRESS, ACCEL_XOUT_L);
        int16_t accel_y = (i2c_read(MPU6050_ADDRESS, ACCEL_YOUT_H) << 8) | i2c_read(MPU6050_ADDRESS, ACCEL_YOUT_L);

        // Scale to g
        float fx = (float)accel_x * 0.000061f;
        float fy = (float)accel_y * 0.000061f;

        printf("Accel X: %.2f g, Y: %.2f g\n", fx, fy);

        // Draw on OLED
        ssd1306_clear();
        draw_arrow(C * fx, C * fy);
        drawmytext(2, 2, "Acceleration Arrow");
        ssd1306_update();

        sleep_ms(20);
    }
return 0;
}
