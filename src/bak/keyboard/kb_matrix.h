#pragma  once
#include "config.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef EXTRA_KEY_ENABLE
void matrix_extra_add_oneshot(uint8_t row, uint8_t col);
void matrix_extra_set(uint8_t row, uint8_t col, bool press);
#endif
