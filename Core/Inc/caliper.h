/* caliper.h – public API for the digital-caliper decoder
 * ------------------------------------------------------
 *  – The caliper sends a single 24-bit frame (LSB-first) every ~8 ms.
 *  – This header exposes just the functions the rest of your project needs:
 *      • Caliper_ResetCapture()   – clear shift-register & restart capture
 *      • Caliper_ProcessBit()     – feed ONE bit on each CLK edge
 *      • Caliper_FrameReady()     – returns true after 24 bits collected
 *      • Caliper_ExtractValues()  – convert frame → µm (REL/ABS)
 *      • Caliper_GetRawBytes()    – raw 3-byte buffer for debugging
 *  – Implementation lives in caliper.c
 */

#ifndef CALIPER_H
#define CALIPER_H

#include <stdint.h>
#include <stdbool.h>

/* total bits in one complete frame */
#define CALIPER_TOTAL_BITS   24u        /* Shahe-compatible: only REL frame */

/* reset capture state – call once at power-up or after long pause */
void Caliper_ResetCapture(void);

/* push a single DATA bit (0|1); call from EXTI on every CLK rising edge */
void Caliper_ProcessBit(uint8_t bit);

/* true → exactly CALIPER_TOTAL_BITS bits have been captured */
bool Caliper_FrameReady(void);

/* decode frame → micrometres; resets internal state automatically */
void Caliper_ExtractValues(int32_t *rel_um, int32_t *abs_um);

/* access raw 24-bit frame (3 bytes, LSB-first) – handy for hex display */
const uint8_t* Caliper_GetRawBytes(void);

#endif /* CALIPER_H */
