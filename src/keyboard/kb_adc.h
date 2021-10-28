#include <stdint.h>
#include <stdbool.h>

#define SAADC_CHANNEL_COUNT 1
#define SAADC_SAMPLE_INTERVAL_MS 1000

void kb_adc_init(void);
uint8_t kb_batt_percentage(void);