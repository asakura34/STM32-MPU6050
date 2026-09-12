/**
 ******************************************************************************
 * @file           : mpu6050.c
 * @brief          : MPU6050 IMU driver implementation for STM32 HAL
 * @author         : Cem Ozan Yilmaz
 * @date           : 12.09.2026 (dd/mm/yyyy)
 ******************************************************************************
 * @details
 *
 * This file implements a custom MPU6050 driver using the STM32 HAL I2C
 * interface. The driver communicates directly with the MPU6050 register map
 * without using an external sensor library.
 *
 * The implementation handles:
 * - WHO_AM_I device identification
 * - Power and clock configuration
 * - Digital low-pass filter and sample rate configuration
 * - Accelerometer and gyroscope full-scale configuration
 * - Raw sensor data acquisition over I2C
 * - Conversion of raw accelerometer and gyroscope data to physical units
 * - Conversion of the internal temperature sensor data to degrees Celsius
 *
 ******************************************************************************
 */


#include "mpu6050.h"
#include <stdint.h>


/* Default sensitivity factors for ±250 dps and ±2 g ranges. */
static float gyro_scale_factor = 131.0f;
static float accel_scale_factor = 16384.0f;


/* Read the device identification register. */
HAL_StatusTypeDef mpu6050_read_who_am_i(I2C_HandleTypeDef *hi2c, uint8_t *who_am_i)
{
    return HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR_L << 1, WHO_AM_I, I2C_MEMADD_SIZE_8BIT, who_am_i, 1, 100);
}

/* Power Management */
HAL_StatusTypeDef mpu6050_wake_up(I2C_HandleTypeDef *hi2c)
{
    uint8_t data = 0x01; // Wake the sensor and select the X-axis gyroscope PLL clock.

    return HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR_L << 1, PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

/* Configuration */
HAL_StatusTypeDef mpu6050_configuration(I2C_HandleTypeDef *hi2c)
{
    uint8_t data = 0x03; // DLPF_CFG = 3: 44 Hz accel / 42 Hz gyro bandwidth.

    return HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR_L << 1, CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

/* Sample Rate Division */
HAL_StatusTypeDef mpu6050_sample_rate_division(I2C_HandleTypeDef *hi2c)
{
    uint8_t data = 9; // 1 kHz / (1 + 9) = 100 Hz sample rate.

    return HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR_L << 1, SMPLRT_DIV, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

/* Gyroscope Configuration */
HAL_StatusTypeDef mpu6050_gyro_configuration(I2C_HandleTypeDef *hi2c, uint8_t fs_sel)
{
    uint8_t data;

    /* Select gyroscope full-scale range and corresponding sensitivity. */
    switch (fs_sel)
    {
        case 0:
            data = 0x00;                    // ±250 dps
            gyro_scale_factor = 131.0f;
            break;

        case 1:
            data = 0x08;                    // ±500 dps
            gyro_scale_factor = 65.5f;
            break;

        case 2:
            data = 0x10;                    // ±1000 dps
            gyro_scale_factor = 32.8f;
            break;

        case 3:
            data = 0x18;                    // ±2000 dps
            gyro_scale_factor = 16.4f;
            break;

        default:
            return HAL_ERROR;
    }

    return HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR_L << 1, GYRO_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

/* Accelerometer Configuration */
HAL_StatusTypeDef mpu6050_accel_configuration(I2C_HandleTypeDef *hi2c, uint8_t afs_sel)
{
    uint8_t data;

    /* Select accelerometer full-scale range and corresponding sensitivity. */
    switch (afs_sel)
    {
        case 0:
            data = 0x00;                    // ±2g
            accel_scale_factor = 16384.0f;
            break;

        case 1:
            data = 0x08;                    // ±4g
            accel_scale_factor = 8192.0f;
            break;

        case 2:
            data = 0x10;                    // ±8g
            accel_scale_factor = 4096.0f;
            break;

        case 3:
            data = 0x18;                    // ±16g
            accel_scale_factor = 2048.0f;
            break;

        default:
            return HAL_ERROR;
    }

    return HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR_L << 1, ACCEL_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

/* Read Accelerometer */
HAL_StatusTypeDef mpu6050_read_accel(I2C_HandleTypeDef *hi2c, float *accel_x, float *accel_y, float *accel_z)
{
    uint8_t data[6];

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR_L << 1, ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT, data, 6, 100);

    if (status != HAL_OK)
    {
        return status;
    }

    /* Combine high and low bytes into signed 16-bit samples. */
    int16_t raw_x = (int16_t)((data[0] << 8) | data[1]);
    int16_t raw_y = (int16_t)((data[2] << 8) | data[3]);
    int16_t raw_z = (int16_t)((data[4] << 8) | data[5]);

    /* Convert raw values using the selected full-scale sensitivity. */
    *accel_x = (float)raw_x / accel_scale_factor;
    *accel_y = (float)raw_y / accel_scale_factor;
    *accel_z = (float)raw_z / accel_scale_factor;

    return HAL_OK;
}


/* Read Gyroscope */
HAL_StatusTypeDef mpu6050_read_gyro(I2C_HandleTypeDef *hi2c, float *gyro_x, float *gyro_y, float *gyro_z)
{
    uint8_t data[6];

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR_L << 1, GYRO_XOUT_H, I2C_MEMADD_SIZE_8BIT, data, 6, 100);

    if (status != HAL_OK)
    {
        return status;
    }

    /* Combine high and low bytes into signed 16-bit samples. */
    int16_t raw_x = (int16_t)((data[0] << 8) | data[1]);
    int16_t raw_y = (int16_t)((data[2] << 8) | data[3]);
    int16_t raw_z = (int16_t)((data[4] << 8) | data[5]);

    /* Convert raw values using the selected full-scale sensitivity. */
    *gyro_x = (float)raw_x / gyro_scale_factor;
    *gyro_y = (float)raw_y / gyro_scale_factor;
    *gyro_z = (float)raw_z / gyro_scale_factor;

    return HAL_OK;
}


/* Read Temperature */
HAL_StatusTypeDef mpu6050_read_temp(I2C_HandleTypeDef *hi2c, float *temperature)
{
    uint8_t data[2];

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR_L << 1, TEMP_OUT_H, I2C_MEMADD_SIZE_8BIT, data, 2, 100);

    if (status != HAL_OK)
    {
        return status;
    }

    /* Combine temperature registers into a signed 16-bit value. */
    int16_t temp_raw = (int16_t)((data[0] << 8) | data[1]);

    /* Convert the raw internal temperature value to degrees Celsius. */
    *temperature = ((float)temp_raw / 340.0f) + 36.53f;

    return HAL_OK;
}