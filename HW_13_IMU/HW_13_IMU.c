#include <stdio.h>
#include "pico/stdlib.h"

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
#define SDA_PIN 4
#define SCL_PIN 5
#define MPU6050_ADDRESS 0x68

// Led pin
#define LED_BUILTIN 25


// to turn on the chip write 0x00 to PWR_MGMT_1 register to turn on the chip
// to turn off the chip write 0x40 to PWR_MGMT_1 register to turn off the chip

// to enable the accelerometer write to the ACCEL_CONFIG register.
// Set the sensitivity to 2g by writing 0x00 to the ACCEL_CONFIG register.

// to enable the gyroscope write to the GYRO_CONFIG register.
// Set the sensitivity to 2000 dps by writing 0x18 to the GYRO_CONFIG register.

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
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
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


int main()
{
    stdio_init_all();
    initialize_pins();
    uint8_t who_am_i = readPin(MPU6050_ADDRESS, WHO_AM_I);
    printf("WHO_AM_I: 0x%02X\n", who_am_i);
    if (who_am_i != 0x68 && who_am_i != 0x98) {
        printf("Error: MPU6050 not found\n");
        error_loop();
    }
    printf("MPU6050 found\n");
    initialize_MPU6050();

    printf("Reading sensor data...\n");
    while(1){
        // Read accelerometer data
        int16_t accel_x = (i2c_read(MPU6050_ADDRESS, ACCEL_XOUT_H) << 8) | i2c_read(MPU6050_ADDRESS, ACCEL_XOUT_L);
        int16_t accel_y = (i2c_read(MPU6050_ADDRESS, ACCEL_YOUT_H) << 8) | i2c_read(MPU6050_ADDRESS, ACCEL_YOUT_L);
        printf("Accel X: %d, Y: %d\n", accel_x, accel_y);
        sleep_ms(100); // Delay for 1 second
    }
return 0;
}
