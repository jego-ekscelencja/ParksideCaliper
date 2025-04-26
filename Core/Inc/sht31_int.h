/*
 * sht31_int.h
 *
 *  Created on: Apr 12, 2025
 *      Author: DevMachine
 */

#ifndef INC_SHT31_INT_H_
#define INC_SHT31_INT_H_


#include "stm32f4xx_hal.h"  // lub "main.h" – ważne, żeby mieć dostęp do stdint.h, I2C_HandleTypeDef, itp.

// Adres SHT31 (jeśli ADDR=GND, w notach to 0x44; *przesunięte* w HAL o 1 w lewo => 0x88 / 0x44<<1).
// Jeżeli ADDR=VDD, to 0x45<<1. Zmieniaj wg swojego hardware.
#define SHT31_I2C_ADDRESS         (0x44 << 1)

// Komenda pomiaru Single Shot (np. repeatability High, clock stretching disabled).
// Najczęściej spotykane: 0x2C, 0x06
#define SHT31_CMD_MEASURE_HIGHREP { 0x24, 0x00 }

// Bufor na 6 bajtów (2B temp + CRC + 2B wilg + CRC)
#define SHT31_RX_DATA_LEN         6

// W SHT31 temperatura = -45 + 175 * (raw / 65535).
// Nie chcemy używać float, więc wyliczamy na int (setne części stopnia).
//     T * 100 = -45*100 + (175*100) * raw / 65535
//     T * 100 = -4500 + (17500 * raw) / 65535
// Dla pewności używamy int32_t i rzutowań, by uniknąć przepełnień w 16-bit.
#define SHT31_CALC_TEMPERATURE_CENTI(tempVar, signVar, rawVal)        \
  do {                                                                \
    int32_t __tmp = -4500 + ((17500U * (uint32_t)(rawVal)) / 65535U); \
    if (__tmp < 0) {                                                 \
      (signVar) = 1; /* oznaczamy znak ujemny */                      \
      (tempVar) = (int16_t)(-__tmp); /* wartość dodatnia w setnych */ \
    } else {                                                         \
      (signVar) = 0;                                                 \
      (tempVar) = (int16_t)__tmp;                                    \
    }                                                                \
  } while(0)

// Wilgotność (w %) = 100 * (raw / 65535).
// Skoro chcemy setne części %, to
//     RH * 100 = (10000 * rawVal) / 65535
#define SHT31_CALC_HUMIDITY_CENTI(humVar, rawVal)                     \
  do {                                                                \
    (humVar) = (uint16_t)(((uint32_t)10000U * (uint32_t)(rawVal))     \
                           / 65535U);                                 \
  } while(0)

// Ewentualny prototyp funkcji CRC jeżeli chcesz sprawdzać poprawność danych
uint8_t SHT31_CheckCrc(uint8_t *data, uint8_t len, uint8_t crc);
void SHT31_Init(void);
#endif /* __SHT31_INT_H__ */


