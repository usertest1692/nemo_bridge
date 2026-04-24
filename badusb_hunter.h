/*
 * BadUSB Hunter - USB Device Profiler & BadUSB Detector
 *
 * Hardware: M5Stack Cardputer (ESP32-S3) ONLY
 * Purpose: Detect and profile USB devices to identify suspicious BadUSB behavior
 *
 * This is a defensive security tool for detecting malicious USB devices.
 *
 * IMPORTANT: Only works on CARDPUTER due to ESP32-S3's USB OTG support
 */

#ifndef BADUSB_HUNTER_H
#define BADUSB_HUNTER_H

#if defined(CARDPUTER)

#include <FastLED.h>
#include <usb/usb_host.h>

// Device tracking structure
struct USBDeviceInfo {
  uint16_t vid;
  uint16_t pid;
  uint8_t deviceClass;
  uint8_t numInterfaces;
  uint8_t interfaceClasses[8]; // Support up to 8 interfaces
  bool isSuspicious;
  String suspicionReason;
};

// WS2812 RGB LED control (Cardputer has LED on pin 21)
#define BADUSB_LED_PIN 21
#define BADUSB_NUM_LEDS 1

// Function declarations
void badusb_hunter_setup();
void badusb_hunter_loop();

#endif // CARDPUTER
#endif // BADUSB_HUNTER_H
