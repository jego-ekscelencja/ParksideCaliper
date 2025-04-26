#ifndef __DISPLAY7SEG_H
#define __DISPLAY7SEG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Liczba cyfr na wyświetlaczu
#define DISPLAY_DIGITS 6
#define SEG_A  (1<<0)
#define SEG_B  (1<<1)
#define SEG_C  (1<<2)
#define SEG_D  (1<<3)
#define SEG_E  (1<<4)
#define SEG_F  (1<<5)
#define SEG_G  (1<<6)
#define SEG_DP (1<<7)

/**
 * @brief  Inicjalizuje bibliotekę (zeruje bufor, ustawia latch w stan domyślny).
 *         Należy wywołać raz na starcie programu.
 */
void Display_Init(void);

/**
 * @brief  Ustawia nową wartość liczbową do wyświetlenia.
 *         Może być zarówno dodatnia, jak i ujemna.
 * @param  number: liczba (int32_t), np. -123, 45678, itd.
 */
void Display_SetNumber(int32_t number);

/**
 * @brief  Funkcja opcjonalna, umożliwiająca skopiowanie backBuffer -> frontBuffer,
 *         jeśli wystąpiła zmiana. Zostaje ustawiona flaga, że jest nowy obraz do wysłania.
 *         Możesz wywoływać ją w pętli głównej lub w przerwaniu timera.
 */
void Display_RefreshIfNeeded(void);

/**
 * @brief  Zwraca wskaźnik na frontBuffer (gotowy do wysłania przez SPI DMA).
 */
uint8_t* Display_GetFrontBuffer(void);

/**
 * @brief  Zwraca 1 jeśli bufor został zmieniony i oczekuje na wyświetlenie,
 *         0 - jeśli brak zmian do odświeżenia.
 */
uint8_t Display_NeedsUpdate(void);
void Display_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
void Display_ShowMicrometers(int32_t value_um);
void Display_ShowRawValue(uint32_t raw);
void Display_ShowRawHex(const uint8_t caliper_rawBits[3]);


#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY7SEG_H */
