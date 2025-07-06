// =====================================================
// Pins.DDScopeX.h
//
// Pin map for DDScope Direct Drive Telescope (Teensy4.1)
//
#pragma once

#if defined(ARDUINO_TEENSY41)

#if SERIAL_A_BAUD_DEFAULT != OFF
  #define SERIAL_A         Serial
#endif

#if SERIAL_B_BAUD_DEFAULT != OFF
  #define SERIAL_B        Serial1
#endif
// SERIAL_B_BAUD_DEFAULT is defined in Config.h

#define ODRIVE_SERIAL     Serial3 
#define ODRIVE_SERIAL_BAUD 115200

#define SERIAL_GPS        Serial4
//#define SERIAL_GPS_BAUD    115200 

// ODrive Pins
#define ODRIVE_RST_PIN          3     // ODrive Reset Pin

// DDScopeX Specific pins
#define AZ_ENABLED_LED_PIN     20     // AZM Motor ON/OFF LED output active low
#define ALT_ENABLED_LED_PIN    21     // ALT Motor ON/OFF LED output active low

#define ALT_THERMISTOR_PIN     23     // Analog input ALT motor temperature
#define AZ_THERMISTOR_PIN      24     // Analog input AZ motor temperature

#define FAN_ON_PIN             25     // Fan enable output active high
#define BATTERY_LOW_LED_PIN    22     // 24V battery LED output active low

// SPI Display pins 
// . these are assumed by TeensyArduino driver as SPI port 1
// ....therefore they are reserved here
#define TFT_DC                  9     // Data/Command
#define TFT_CS                 10     // Chip Select active low
#define TFT_MOSI               11     // Master out, Slave in
#define TFT_MISO               12     // Master in, Slave out
#define TFT_SCLK               13     // SPI clock

// Touchscreen pins
#define TS_CS                   8     // Chip Select output active low
#define TS_IRQ                  6     // Interrupt input active low
#define TFT_RST                 5     // RESET Display

// DDScopeX DC Focuser pins
#define FOCUSER_STEP_PIN       28     // Focuser Step output
#define FOCUSER_DIR_PIN        29     // Focuser Direction output
#define FOCUSER_EN_PIN         27     // Focuser Enable output
#define FOCUSER_SLEEP_PIN      26     // Sleep Motor Controller output

// OnStepX specific pins
#define STATUS_BUZZER_PIN       4     // Output Piezo Buzzer
#define STATUS_TRACK_LED_PIN    7     // Output, active low, flash when tracking

// Using Beitian BN-220 without the PPS pinned out. The BN-280 has PPS pinned out
#define PPS_SENSE_PIN           2     // Input Pin, Pulse Per Second from GPS

// ESP32C3 LX200 Command Processor 
#define ESP32C3_RST_PIN        36     // ESP32C3 Reset Pin - Active Low

#else
#error "Wrong processor for this configuration!"

#endif
