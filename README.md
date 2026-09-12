# STM32 NUCLEO-L476RG MPU6050 IMU Library with I2C

This project is an STM32 HAL firmware project for reading an MPU6050
accelerometer and gyroscope sensor with a NUCLEO-L476RG development board.

The library communicates with the MPU6050 over `I2C1`, configures the sensor, reads
acceleration, angular velocity, and temperature values, and the main application
prints the measurements over `USART2`.

## Features

- STM32L476RG based project
- Custom MPU6050 I2C library written for STM32 HAL
- MPU6050 communication over `I2C1`
- Accelerometer readings for X, Y, and Z axes
- Gyroscope readings for X, Y, and Z axes
- Internal MPU6050 temperature reading
- UART serial output over `USART2`

## Hardware

- NUCLEO-L476RG development board
- MPU6050 sensor module
- USB cable for programming and UART serial output
- I2C pull-up resistors, if they are not already included on the sensor module

## Default Configuration at example on main.c

| Item | Value |
| --- | --- |
| MCU | STM32L476RG |
| Board | NUCLEO-L476RG |
| Sensor | MPU6050 |
| Sensor I2C address | `0x68` |
| I2C peripheral | `I2C1` |
| I2C SCL | `PB8` |
| I2C SDA | `PB9` |
| UART peripheral | `USART2` |
| UART TX/RX | `PA2` / `PA3` |
| UART baud rate | `115200` |
| DLPF setting | `DLPF_CFG = 3` |
| Sample rate | `100 Hz` |
| Gyroscope range | `+/-250 dps` |
| Accelerometer range | `+/-2 g` |
| Read interval | `500 ms` |

The MPU6050 sample rate and the application's read/print interval are
independent settings. The sensor updates its output registers at `100 Hz`, while
the main loop reads and prints the latest measurement every `500 ms`. The
`500 ms` delay is used only to keep the serial output readable.

## Wiring

Typical MPU6050 wiring for the default firmware configuration:

| MPU6050 Pin | NUCLEO-L476RG |
| --- | --- |
| VCC | 3.3 V |
| GND | GND |
| SCL | `PB8` / `I2C1_SCL` |
| SDA | `PB9` / `I2C1_SDA` |

Make sure the MPU6050 and the STM32 board share a common ground.

## Project Structure

```text
Core/
  Inc/
    mpu6050.h              MPU6050 register definitions and library interface
    i2c.h                  STM32 I2C declarations
    usart.h                STM32 UART declarations
    gpio.h                 STM32 GPIO declarations
    main.h                 Main application declarations
  Src/
    main.c                 Main MPU6050 read loop and UART output
    mpu6050.c              MPU6050 STM32 HAL library implementation
    i2c.c                  I2C1 initialization
    usart.c                USART2 initialization
    gpio.c                 GPIO initialization
Drivers/                   STM32 HAL and CMSIS drivers
L476RG_MPU6050.ioc         STM32CubeMX project configuration
startup_stm32l476xx.s      STM32L476 startup file
```

## How It Works

1. Initializes STM32 HAL, system clock, GPIO, `USART2`, and `I2C1`.
2. Reads the MPU6050 `WHO_AM_I` register.
3. Wakes the MPU6050 by writing to `PWR_MGMT_1`.
4. Configures the digital low-pass filter with `DLPF_CFG = 3`.
5. Sets the sample rate divider for a `100 Hz` sample rate.
6. Configures the gyroscope range to `+/-250 dps`.
7. Configures the accelerometer range to `+/-2 g`.
8. Reads accelerometer, gyroscope, and temperature data in the main loop.
9. Sends formatted values over UART with `HAL_UART_Transmit()`.
10. Waits `500 ms`, then repeats.

## DLPF and Sample Rate

The MPU6050 digital low-pass filter is configured with the `DLPF_CFG` bits
`[2:0]` in the `CONFIG` register, address `0x1A`.

In the current library, `mpu6050_configuration()` writes:

```c
uint8_t data = 0x03; // DLPF_CFG = 3
```

This means the current setting is `DLPF_CFG = 3`.

Available `DLPF_CFG` values:

| DLPF_CFG | Accel bandwidth | Gyro bandwidth | Gyro output rate |
| --- | --- | --- | --- |
| `0` | `260 Hz` | `256 Hz` | `8 kHz` |
| `1` | `184 Hz` | `188 Hz` | `1 kHz` |
| `2` | `94 Hz` | `98 Hz` | `1 kHz` |
| `3` | `44 Hz` | `42 Hz` | `1 kHz` |
| `4` | `21 Hz` | `20 Hz` | `1 kHz` |
| `5` | `10 Hz` | `10 Hz` | `1 kHz` |
| `6` | `5 Hz` | `5 Hz` | `1 kHz` |
| `7` | Reserved | Reserved | `8 kHz` |

`DLPF_CFG` values `1` through `6` use a `1 kHz` gyroscope output rate.
`DLPF_CFG` values `0` and `7` use an `8 kHz` gyroscope output rate.
`DLPF_CFG = 7` is reserved and should not be used for normal operation.

To manually change the DLPF setting, change the value written to the `CONFIG`
register inside `mpu6050_configuration()` in `Core/Src/mpu6050.c`. These
settings are intentionally fixed in the current driver and are changed manually
from the register values.

`DLPF_CFG` occupies bits `[2:0]` of the `CONFIG` register. In the current
implementation, the other `CONFIG` bits are left at their default values, so a
value such as:

```c
uint8_t data = 0x03; // DLPF_CFG = 3
```

directly represents `DLPF_CFG = 3`. Similarly:

```c
uint8_t data = 0x04; // DLPF_CFG = 4
```

directly represents `DLPF_CFG = 4`. If other `CONFIG` register bits are used in
the future, construct the complete register value carefully instead of assuming
the whole register contains only `DLPF_CFG`.

The sample rate is controlled by the `SMPLRT_DIV` register, address `0x19`:

```text
Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
```

The current project uses `DLPF_CFG = 3`, so the gyroscope output rate is
`1 kHz`. In `mpu6050_sample_rate_division()`, the library writes:

```c
uint8_t data = 9;
```

Therefore:

```text
Sample Rate = 1000 / (1 + 9) = 100 Hz
```

To calculate a new `SMPLRT_DIV` value:

```text
SMPLRT_DIV = (Gyroscope Output Rate / Desired Sample Rate) - 1
```

`SMPLRT_DIV` is an 8-bit integer register, so valid values are `0` through
`255`. Not every desired sample rate can be generated exactly. For example, with
a `1 kHz` gyroscope output rate and a desired rate of `300 Hz`:

```text
SMPLRT_DIV = (1000 / 300) - 1 = 2.33...
```

A fractional divider cannot be written to the register, so choose an integer
divider and calculate the resulting actual sample rate.

Examples when the gyroscope output rate is `1 kHz`:

| Desired sample rate | SMPLRT_DIV |
| --- | --- |
| `1000 Hz` | `0` |
| `500 Hz` | `1` |
| `200 Hz` | `4` |
| `100 Hz` | `9` |
| `50 Hz` | `19` |
| `20 Hz` | `49` |
| `10 Hz` | `99` |

To manually change the sample rate, change the value assigned to `data` inside
`mpu6050_sample_rate_division()` in `Core/Src/mpu6050.c`. This setting is also
intentionally fixed in the current driver and is changed manually from the
register value.

Important: the base gyroscope output rate depends on `DLPF_CFG`. When
`DLPF_CFG` is `1` through `6`, the gyroscope output rate is `1 kHz`. When
`DLPF_CFG` is `0` or `7`, the gyroscope output rate is `8 kHz`. If you change
the DLPF setting, use the matching gyroscope output rate when calculating
`SMPLRT_DIV`.

## Example Serial Output

Open a serial monitor on the NUCLEO board's virtual COM port:

```text
Baud rate: 115200
Data bits: 8
Parity:    None
Stop bits: 1
```

Example output:

```text
ACC: X=12 Y=-36 Z=1015 mg | GYRO: X=122 Y=-61 Z=30 mdps | TEMP=27.45 C
ACC: X=10 Y=-35 Z=1016 mg | GYRO: X=114 Y=-53 Z=22 mdps | TEMP=27.47 C
```

## Important Files

- `Core/Src/main.c`: initializes the board, configures the MPU6050, and prints measurements over UART.
- `Core/Inc/mpu6050.h`: MPU6050 register addresses and function declarations.
- `Core/Src/mpu6050.c`: custom MPU6050 library functions for setup and sensor reads.
- `Core/Src/i2c.c`: `I2C1` setup for `PB8` and `PB9`.
- `Core/Src/usart.c`: `USART2` setup for serial output.

## Notes

- `0x68` is the default MPU6050 address when `AD0` is low.
- `0x69` is used when `AD0` is high.
- STM32 HAL expects the 7-bit I2C address shifted left by one bit when passed
  to `HAL_I2C_Mem_Read()` and `HAL_I2C_Mem_Write()` in this implementation.
  The current library uses `MPU6050_ADDR_L << 1`.
- If the sensor module uses address `0x69`, update the library to use
  `MPU6050_ADDR_H << 1`.
- Accelerometer values are printed in milli-g (`mg`).
- Gyroscope values are printed in milli-degrees per second (`mdps`).
- Temperature is calculated from the MPU6050 raw temperature register using
  the standard formula: `temperature = raw / 340.0 + 36.53`.
