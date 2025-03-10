/* Encoder Library - Basic Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */

// #define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Arduino.h>
#include <limits.h>
#include <Encoder.h>
#include "encoder_handler.h"

#define _IDA 1

#define CLKA 2
#define DTA 3
#define SWA 4

#define CLKB 5
#define DTB 6
#define SWB 7

#define CLKC 8
#define DTC 9
#define SWC 10

#define CLKD 0
#define DTD 0
#define SWD 11

#define PULSES_PER_DETENT 2

EncoderHandler encoder_handlerA(0, CLKA, DTA, SWA, PULSES_PER_DETENT);
EncoderHandler encoder_handlerB(1, CLKB, DTB, SWB, PULSES_PER_DETENT);
EncoderHandler encoder_handlerC(2, CLKC, DTC, SWC, PULSES_PER_DETENT);
EncoderHandler encoder_handlerD(3, CLKD, DTD, SWD, PULSES_PER_DETENT);

void setup(){
  Serial.begin(115200);
}

void loop() {
  encoder_handlerA.step();
  encoder_handlerB.step();
  encoder_handlerC.step();
  encoder_handlerD.step();
}
