/*
 * Bandai Namco Protocol
 * WHowe <github.com/whowechina>
 */

#ifndef BANA_H
#define BANA_H

#include <stdint.h>
#include <stdbool.h>

/* return true if accepts a byte, false if rejects */
typedef void (*bana_putc_func)(uint8_t byte);

void bana_init(bana_putc_func putc_func);
void bana_debug(bool enable);

bool bana_feed(int c);

/* if bana is currently active */
bool bana_is_active();

uint32_t bana_led_color();


#endif