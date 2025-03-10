#ifndef __LEDS_H__
#define __LEDS_H__

#include "hardware.h"
#include "led_handler.h"

// default on/off time when animating panel LEDs
#define DEFAULT_PANEL_LEDS_SHOW_TIME	 750
#define DEFAULT_PANEL_LEDS_BLANK_TIME	350

// adjustments to match the panels LEDs to the display
#define LED_INTENSITY1 32 // Amber panel LED matched to Green
#define LED_INTENSITY2 32 // Green panel LED is a bit brighter than the Amber LED
#define LED_INTENSITY3 52 // Blue panel LED?

extern const int led_pins[];

// array used to pass the LED intensities to LED handlers
extern const int led_intensities[];

// LED handler for just the panel LEDs
extern LEDHandler panel_leds;

#endif
