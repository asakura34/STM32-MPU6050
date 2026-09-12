/**
 ******************************************************************************
 * @file           : mpu6050.h
 * @brief          : MPU6050 IMU driver interface for STM32 HAL
 * @author         : Cem Ozan Yilmaz
 * @date           : 12.09.2026 (dd/mm/yyyy)
 ******************************************************************************
 * @details
 *
 * This file contains the public interface and register definitions for the
 * custom MPU6050 driver used with STM32 HAL.
 *
 * The driver provides functions for:
 * - Device identification
 * - Power and clock configuration
 * - Digital low-pass filter and sample rate configuration
 * - Accelerometer and gyroscope full-scale configuration
 * - Accelerometer, gyroscope, and temperature measurements
 *
 ******************************************************************************
 */

#ifndef MPU6050_H
#define MPU6050_H

#include "stm32l4xx_hal.h"

#define MPU6050_ADDR_L      0x68
#define MPU6050_ADDR_H      0x69
#define WHO_AM_I            0x75
#define PWR_MGMT_1          0x6B
#define CONFIG              0x1A
#define SMPLRT_DIV          0x19
#define GYRO_CONFIG         0x1B
#define ACCEL_CONFIG        0x1C
#define GYRO_XOUT_H         0x43
#define GYRO_XOUT_L         0x44
#define GYRO_YOUT_H         0x45
#define GYRO_YOUT_L         0x46
#define GYRO_ZOUT_H         0x47
#define GYRO_ZOUT_L         0x48
#define ACCEL_XOUT_H        0x3B
#define ACCEL_XOUT_L        0x3C
#define ACCEL_YOUT_H        0x3D
#define ACCEL_YOUT_L        0x3E
#define ACCEL_ZOUT_H        0x3F
#define ACCEL_ZOUT_L        0x40
#define TEMP_OUT_H          0x41
#define TEMP_OUT_L          0x42


HAL_StatusTypeDef mpu6050_read_who_am_i(I2C_HandleTypeDef *hi2c, uint8_t *who_am_i);
HAL_StatusTypeDef mpu6050_wake_up(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef mpu6050_configuration(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef mpu6050_sample_rate_division(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef mpu6050_gyro_configuration(I2C_HandleTypeDef *hi2c, uint8_t fs_sel);
HAL_StatusTypeDef mpu6050_accel_configuration(I2C_HandleTypeDef *hi2c, uint8_t afs_sel);
HAL_StatusTypeDef mpu6050_read_accel(I2C_HandleTypeDef *hi2c, float *accel_x, float *accel_y, float *accel_z);
HAL_StatusTypeDef mpu6050_read_gyro(I2C_HandleTypeDef *hi2c, float *gyro_x, float *gyro_y, float *gyro_z);
HAL_StatusTypeDef mpu6050_read_temp(I2C_HandleTypeDef *hi2c, float *temperature);

#endif