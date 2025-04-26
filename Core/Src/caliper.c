/* caliper.c – runtime code for Shahe-type 24-bit digital caliper
 * --------------------------------------------------------------
 * Interface:
 *   • Caliper_ResetCapture()        – clear internal shift-register
 *   • Caliper_ProcessBit(bit)       – feed ONE DATA bit on every CLK edge
 *   • Caliper_FrameReady()          – true → 24 bits collected
 *   • Caliper_ExtractValues()       – convert frame → µm (REL)
 * Helpers:
 *   • Caliper_GetRawBytes()         – raw 3-byte buffer for debugging
 *
 * NOTE:  Frame format (LSB-first)
 *          bits  0..15 : counter, step = 0.01 mm  (=10 µm)
 *          bit      20 : sign (1 = negative)
 *          all others  : 0
 */

#include "caliper.h"

/* ───────── internal state ─────────────────────────────────────────── */
static uint64_t shift_reg; /* 64-bit bucket for incoming bits */
static uint8_t bit_index; /* 0 … 23                              */
static bool frame_ready; /* set when 24 bits captured          */
static uint8_t raw_bits[6]; /* debug copy (0..2 used)             */

/* clear state – call at power-up or after pause >3 ms */
void Caliper_ResetCapture(void) {
	shift_reg = 0;
	bit_index = 0;
	frame_ready = false;
}

/* feed one bit (LSB-first) – call from EXTI on each RISING CLK */
void Caliper_ProcessBit(uint8_t bit) {
	if (frame_ready)
		return; /* previous frame not read yet */

	shift_reg |= ((uint64_t) (bit & 1u)) << bit_index++; /* accumulate */

	if (bit_index >= CALIPER_TOTAL_BITS) /* 24 bits done */
	{
		raw_bits[0] = shift_reg & 0xFFu; /* keep copy for hex display   */
		raw_bits[1] = (shift_reg >> 8) & 0xFFu;
		raw_bits[2] = (shift_reg >> 16) & 0xFFu;
		frame_ready = true;
	}
}

bool Caliper_FrameReady(void) {
	return frame_ready;
}
const uint8_t* Caliper_GetRawBytes(void) {
	return raw_bits;
}

/* ───────── 24-bit frame → micrometres ─────────────────────────────── */
static int32_t Raw24_To_Um(uint32_t raw24) {
	const uint32_t SIGN_MASK = 0x100000u; /* bit-20 = sign */

	bool negative = (raw24 & SIGN_MASK) != 0;
	uint32_t ticks = raw24 & 0xFFFFu; /* counter in 0.01 mm */

	int32_t um = (int32_t) ticks * 10; /* 0.01 mm ⇒ 10 µm */
	return negative ? -um : um;
}

/* read REL value (ABS unused), reset capture for next frame */
void Caliper_ExtractValues(int32_t *rel_um, int32_t *abs_um) {
	uint32_t raw24 = (uint32_t) raw_bits[0] | ((uint32_t) raw_bits[1] << 8)
			| ((uint32_t) raw_bits[2] << 16);

	*rel_um = Raw24_To_Um(raw24);
	*abs_um = 0;

	Caliper_ResetCapture();
}
