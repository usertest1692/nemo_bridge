/*
 * IR SNIPER v4.0 - OFFICIAL RECONSTRUCT
 * Uses tvbg.h for bit-reading logic.
 */
#include <M5Unified.h>
#include <IRremote.hpp>
#include "tvbg.h"
#include "WORLD_IR_CODES.h"

#define IRLED 19

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  
  IrSender.begin(IRLED, DISABLE_LED_FEEDBACK);
  
  M5.Display.setRotation(3);
  M5.Display.fillScreen(BLACK);
  M5.Display.println("V4.0 RESTORED");
}

void loop() {
  if (Serial.available() > 0) {
    int idx = Serial.parseInt();
    if (idx >= 0) {
      // Logic from tvbg.h / v4
      if (idx < num_NAcodes) {
        powerCode = NApowerCodes[idx];
      } else {
        powerCode = EUpowerCodes[idx - num_NAcodes];
      }

      uint8_t khz = powerCode->timer_val;
      uint8_t numpairs = powerCode->numpairs;
      uint8_t bitcompression = powerCode->bitcompression;
      
      code_ptr = 0;
      bitsleft_r = 0;

      for (uint8_t k = 0; k < numpairs; k++) {
        uint8_t ti = read_bits(bitcompression);
        rawData[k * 2] = pgm_read_word(&powerCode->times[ti * 2]) * 10;
        rawData[k * 2 + 1] = pgm_read_word(&powerCode->times[ti * 2 + 1]) * 10;
      }

      IrSender.sendRaw(rawData, numpairs * 2, khz);
      Serial.println("OK");
    }
    while(Serial.available() > 0) Serial.read(); 
  }
}
