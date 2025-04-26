/*
 * sht31_int.c
 *
 *  Created on: Apr 12, 2025
 *      Author: DevMachine
 */


#include "sht31_int.h"

uint8_t SHT31_CheckCrc(uint8_t *data, uint8_t len, uint8_t crc)
{
    // Algorytm CRC-8 / polinom 0x31 wg dokumentacji Sensirion.
    // Zwraca 1, jeśli OK, 0 jeśli błąd.
    uint8_t bit;
    uint8_t crcCalc = 0xFF;

    for (uint8_t i = 0; i < len; i++)
    {
        crcCalc ^= data[i];
        for (bit = 0; bit < 8; bit++)
        {
            if (crcCalc & 0x80)
                crcCalc = (crcCalc << 1) ^ 0x31;
            else
                crcCalc = (crcCalc << 1);
        }
    }
    return (crcCalc == crc) ? 1 : 0;
}


// Prototyp

