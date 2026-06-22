/*
 * DS18B20 OneWire temperature sensor driver for STM32F103C8T6
 * PB10 = DQ (data line, requires external 4.7kΩ pull-up to 3.3V)
 *
 * Non-blocking: call DS18B20_Process() periodically (every 100ms),
 * read last result via DS18B20_GetTempTenths().
 */
#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f1xx_hal.h"

/* ── Public API ─────────────────────────────────────────── */

/* Initialize DWT cycle counter & release DQ line.
   Returns 0 = ok, 1 = no device detected */
uint8_t DS18B20_Init(void);

/* Non-blocking state machine — call every ~100ms from main loop.
   Automatically starts conversion and reads result when ready. */
void DS18B20_Process(void);

/* Return last valid temperature in 0.1 °C units.
   e.g. 250 = 25.0°C, -55 = -5.5°C.
   Returns DS18B20_ERROR_VAL (-999) on error / no reading yet. */
#define DS18B20_ERROR_VAL  (-999)
int16_t DS18B20_GetTempTenths(void);

#endif /* __DS18B20_H */
