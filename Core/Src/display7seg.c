/*
 * display7seg.c
 *
 *  Created on: Mar 16, 2025
 *      Author: DevMachine
 *
 *  Przykładowa biblioteka obsługująca 6-cyfrowy wyświetlacz
 *  przez rejestry przesuwne (np. 74HC595) + SPI2 + DMA + LATCH.
 */

#include "display7seg.h"

// W razie potrzeby zamień na:
#include "main.h"   // jeśli tam masz m.in. #include "stm32f4xx_hal.h" i definicje portu/pinu latch
#include "stm32f4xx_hal.h" // lub wprost
#include "gpio.h"    // gdzie masz zdefiniowane SPI2_LA_Pin i SPI2_LA_GPIO_Port
#include <string.h>  // dla memcpy, jeśli używamy
#include <stdio.h>
// -----------------------------------------------------------
// Definicje segmentów:
// np. bit 7 => A, bit 6 => B, bit 5 => C, bit 4 => D, ...
// Przykład: 0b11111100 => cyfra '0' (bez kropki, dp=0)
// Dopasuj do swojego układu (74HC595) i kolejności wyprowadzeń.
static const uint8_t segmentDigits[20] = {
    0b00111111, // [0] => 0
    0b00000110, // [1] => 1
    0b01011011, // [2] => 2
    0b01001111, // [3] => 3
    0b01100110, // [4] => 4
    0b01101101, // [5] => 5
    0b01111101, // [6] => 6
    0b00000111, // [7] => 7
    0b01111111, // [8] => 8
    0b01101111, // [9] => 9
    0b00000000, // [10] => blank (wszystkie segmenty off)
    0b01000000  // [11] => minus

};

/*  Dodaj raz – najlepiej obok tablicy segmentDigits[]  */
#ifndef SEG_DP
#define SEG_DP  0x80          // bit, którym zapalasz kropkę
#endif

//static const uint8_t segmentDigits[16] = {
//    0b00111111, // 0
//    0b00000110, // 1
//    0b01011011, // 2
//    0b01001111, // 3
//    0b01100110, // 4
//    0b01101101, // 5
//    0b01111101, // 6
//    0b00000111, // 7
//    0b01111111, // 8
//    0b01101111, // 9
//    0b01110111, // A (10)
//    0b01111100, // b (11)
//    0b00111001, // C (12)
//    0b01011110, // d (13)
//    0b01111001, // E (14)
//    0b01110001  // F (15)
//};
// frontBuffer: to, co aktualnie wyświetlamy / wysyłamy przez SPI
static volatile uint8_t frontBuffer[DISPLAY_DIGITS];

// backBuffer: to, co ustawiamy w Display_SetNumber() (nowa wartość)
static volatile uint8_t backBuffer[DISPLAY_DIGITS];

// Flaga informująca, że backBuffer zawiera nową wartość, gotową do przesłania
static volatile uint8_t display_needs_update = 0;

//
// Funkcja inicjująca
//
void Display_Init(void)
{
    // Latch w stan niski (niekoniecznie wymagane, ale można tak zacząć)
    HAL_GPIO_WritePin(SPI2_LA_GPIO_Port, SPI2_LA_Pin, GPIO_PIN_RESET);

    // Wyzerowanie obu buforów (blank)
    for (int i = 0; i < DISPLAY_DIGITS; i++)
    {
        frontBuffer[i] = segmentDigits[10]; // blank
        backBuffer[i]  = segmentDigits[10]; // blank
    }

    display_needs_update = 1;  // bo mamy gotowy "obraz" do wyświetlenia
}

//
// Funkcja ustawiająca liczbę (może być dodatnia, ujemna)
//
void Display_SetNumber(int32_t number)
{
    // Najpierw wypełniamy backBuffer "pustymi" segmentami
    for(int i = 0; i < DISPLAY_DIGITS; i++)
    {
        backBuffer[i] = segmentDigits[10]; // blank
    }

    // Obsługa znaku
    int negative = 0;
    if (number < 0)
    {
        negative = 1;
        number = -number; // wartość bezwzględna
    }

    // Rozbijanie cyfry po cyfrze (LSB -> MSB)
    int idx = 0;
    if (number == 0)
    {
        // Wyświetl '0'
        backBuffer[idx++] = segmentDigits[0];
    }
    else
    {
        while (number != 0 && idx < DISPLAY_DIGITS)
        {
            uint8_t d = number % 10;
            number /= 10;
            backBuffer[idx++] = segmentDigits[d];
        }
    }

    // Jeśli liczba była ujemna i jest jeszcze miejsce w buforze
    if (negative && idx < DISPLAY_DIGITS)
    {
        backBuffer[DISPLAY_DIGITS - 1] = segmentDigits[11]; // minus w najbardziej znaczącej pozycji
    }

    // Ustawiamy flagę, że jest nowa wartość w backBuffer
    display_needs_update = 1;
}

//
// Funkcja sprawdzająca, czy w backBuffer jest nowa wartość
// i jeśli tak – kopiuje do frontBuffer.
// Po tej operacji frontBuffer jest gotowy do wysłania przez SPI.
//
void Display_RefreshIfNeeded(void)
{
    if (display_needs_update)
    {
        // Kopiowanie back->front
        memcpy((uint8_t*)frontBuffer, (const uint8_t*)backBuffer, DISPLAY_DIGITS);

        // Można wyczyścić flagę (bufor skopiowany)
        display_needs_update = 0;
    }
}

//
// Funkcja zwraca wskaźnik na frontBuffer
// (zawiera aktualne dane do wysyłki).
//
uint8_t* Display_GetFrontBuffer(void)
{
    return (uint8_t*)frontBuffer;
}

//
// Sprawdza, czy jest potrzeba aktualizacji (czy backBuffer był zmieniony).
//
uint8_t Display_NeedsUpdate(void)
{
    return display_needs_update;
}


void Display_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    for(int i = 0; i < DISPLAY_DIGITS; i++)
    {
        backBuffer[i] = segmentDigits[10]; // blank
    }

    backBuffer[5] = segmentDigits[ hours  / 10 ];           // H tens
    backBuffer[4] = segmentDigits[ hours  % 10 ] | SEG_DP;  // H units + kropka
    backBuffer[3] = segmentDigits[ minutes/ 10 ];           // M tens
    backBuffer[2] = segmentDigits[ minutes% 10 ] | SEG_DP;  // M units + kropka
    backBuffer[1] = segmentDigits[ seconds/ 10 ];           // S tens
    backBuffer[0] = segmentDigits[ seconds% 10 ]| SEG_DP;;           // S units (bez dp, albo z dp jeśli chcesz)

    display_needs_update = 1;
}




#include <stdio.h>  // dodaj jeśli jeszcze nie ma
#include <stdbool.h>
/* =========================================================================
 *  Wyświetl liczbę w µm jako  x.xxx  /  xx.xx  /  xxx.x  /  xxxx   (±999.99 mm)
 *  Układ bufora: LSB-first – backBuffer[0] = 0.01 mm,  …,  backBuffer[5] = MSB
 *  segmentDigits[10] = blank ,  segmentDigits[11] = '-' ,  SEG_DP = 0x80
 * ======================================================================== */
void Display_ShowMicrometers(int32_t value_um)
{
    /* 1. wyczyść bufor */
    for (int i = 0; i < DISPLAY_DIGITS; ++i)
        backBuffer[i] = segmentDigits[10];           /* blank */

    /* 2. znak */
    bool negative = (value_um < 0);
    if (negative) value_um = -value_um;              /* wartość dodatnia */

    /* 3. setne mm (0,01 mm) */
    uint32_t cmm = (uint32_t)value_um / 10;          /* 1 krok = 0,01 mm */
    int idx = 0;

    /* 0,01 mm i 0,1 mm */
    backBuffer[idx++] = segmentDigits[cmm % 10]; cmm /= 10;    /* 0,01 */
    backBuffer[idx++] = segmentDigits[cmm % 10]; cmm /= 10;    /* 0,1  */

    /* 1 mm – z kropką */
    backBuffer[idx++] = segmentDigits[cmm % 10] | SEG_DP; cmm /= 10;

    /* dziesiątki, setki… mm */
    while (idx < DISPLAY_DIGITS && cmm) {
        backBuffer[idx++] = segmentDigits[cmm % 10];
        cmm /= 10;
    }

    /* 4. minus „przy liczbie”, nie na skraju */
    if (negative) {
        if (idx < DISPLAY_DIGITS)                    /* mamy wolną pozycję */
            backBuffer[idx] = segmentDigits[11];     /* wstaw '-' tuż obok */
        else                                         /* brak miejsca → nadpisz MSB */
            backBuffer[idx - 1] = segmentDigits[11];
    }

    /* 5. overflow – jeśli po zużyciu 6 pozycji zostały jeszcze cyfry */
    if (cmm) {
        for (int i = 0; i < DISPLAY_DIGITS; ++i)
            backBuffer[i] = segmentDigits[11];       /* same kreseczki */
    }

    display_needs_update = 1;                        /* odśwież wyświetlacz */
}




void Display_ShowRawHex(const uint8_t caliper_rawBits[3])
{
    // Złap surowe 24 bity
    uint32_t raw = ((uint32_t)caliper_rawBits[0] << 16) |
                   ((uint32_t)caliper_rawBits[1] << 8)  |
                    (uint32_t)caliper_rawBits[2];

    // Pokaż górne 4 bity (bit23..20)
    uint8_t hi = (raw >> 20) & 0x0F;
    // Pokaż środkowe 4 bity (bit19..16)
    uint8_t h2 = (raw >> 16) & 0x0F;
    // Pokaż kolejne 4 bity (bit15..12)
    uint8_t h3 = (raw >> 12) & 0x0F;
    // Pokaż dolne 12 bitów jako 3 hexdcyfry:
    uint8_t b2 = (raw >> 8) & 0x0F;
    uint8_t b1 = (raw >> 4) & 0x0F;
    uint8_t b0 = raw & 0x0F;

    // Wypisz te 6 hexdcyfr na 6-pozycyjnym wyświetlaczu
    backBuffer[0] = segmentDigits[b0];
    backBuffer[1] = segmentDigits[b1];
    backBuffer[2] = segmentDigits[b2];
    backBuffer[3] = segmentDigits[h3];
    backBuffer[4] = segmentDigits[h2];
    backBuffer[5] = segmentDigits[hi];
    display_needs_update = 1;
}

