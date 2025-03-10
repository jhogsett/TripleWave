#include "hardware.h"
#include "led_handler.h"
#include "leds.h"

const int led_pins[] = {GREEN_PANEL_LED, AMBER_PANEL_LED, BLUE_PANEL_LED};
const int intensities[] = {LED_INTENSITY1, LED_INTENSITY2, LED_INTENSITY3};

LEDHandler panel_leds(3, led_pins, intensities);
