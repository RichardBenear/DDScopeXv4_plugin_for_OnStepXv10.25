// =====================================================
// LX200Handler.cpp
//
// Handle LX200 commands
// External to Teensy4.1, an ESP32-C3 is used to handle the
// transactions between the Client (SkySafari or Stellarium) 
// and the OnStep command channels. Not tested on any other
// planetarium programs.
//
// **** by Richard Benear 5/21/2025 ****
//
// See rights and use declaration in License.h
//
#include "LX200Handler.h"
#include "../display/Display.h"
#include "src/lib/serial/Serial_Local.h"

void lxWrapper() { lx200Handler.lxPoll(); }

#define SERIAL_ESP32  SERIAL_ESP32C3
#define SERIAL_ESP_BAUD 230400//115200

// It's possible to use the already installed ESP32 D1 Mini
// instead of the ESP32-C3, but this is not fully tested
// ESP32 D1 Mini takes more power than the ESP32-C3. Also,
// it is reserved for use with the SmartWebServer.
//#define SERIAL_ESP32  SERIAL_B
//#define SERIAL_ESP_BAUD SERIAL_B_BAUD_DEFAULT

void LX200Handler::init() {
  pinMode(34, INPUT_PULLUP);  // Serial8 RX pin
   
  SERIAL_ESP32.begin(SERIAL_ESP_BAUD);
  SERIAL_DEBUG.println("MSG: LX200, Starting ESP32C3 Serial");
  delay(10);
  
  // clear any junk in RX buffers
  while (SERIAL_ESP32.available()) SERIAL_ESP32.read(); 

  // start LX200 poll task
  VF("MSG: Setup, start LX200 polling task (rate 10 ms priority 2)... ");
  uint8_t lx_handle = tasks.add(20, 0, true, 3, lxWrapper, "LX200 task");
  if (lx_handle) {
    VLF("success");
  } else {
    VLF("FAILED!");
  }
}

// =====================================================
void LX200Handler::lxPoll() {

  if (SERIAL_ESP32.available()) {
    char peekChar = SERIAL_ESP32.peek();

    //SERIAL_DEBUG.print("0x"); SERIAL_DEBUG.println(peekChar, HEX);
    if (peekChar == 'L') {
      //unsigned long tStart = micros();  // Start timing
      SERIAL_ESP32.read();
      SERIAL_ESP32.write('K'); // ACK
      SERIAL_ESP32.flush();

      String lxCmd = "";

      // Read command from Teensy until '#' or timeout
      unsigned long start = micros();
      const unsigned long timeoutMicros = 10000;
      while ((micros() - start) < timeoutMicros) {
        if (SERIAL_ESP32.available()) {
          char c = SERIAL_ESP32.read();
          lxCmd += c;
          if (c == '#') break;
        }
      }

      //SERIAL_DEBUG.print("LX200 Cmd = "); SERIAL_DEBUG.println(lxCmd);

      if (lxCmd.length() == 0 || lxCmd.charAt(0) != ':' || lxCmd.charAt(lxCmd.length() - 1) != '#') {
        SERIAL_DEBUG.printf("MSG: LX200, Timeout or missing '#' terminator. Received so far: \"%s\"\n", lxCmd.c_str());
        return;
      }

      const char* cmd = lxCmd.c_str();
      char lxResp[32] = "";

      // Determine whether command is a setter (e.g., :Sr) or a getter (e.g., :GD)
      bool isSetter = false;
      if (strlen(cmd) > 3) {
        // Setter commands typically begin with :S (Set) and have parameters
        if (cmd[1] == 'S' && isalpha(cmd[2]) && cmd[strlen(cmd) - 1] == '#') {
          isSetter = true;
        }
      }

      if (isSetter) {
        // Setter: Use commandBool
        bool result = commandBool((char*)cmd);
        snprintf(lxResp, sizeof(lxResp), "%d#", result ? 1 : 0);
      } else {
        // Getter: Use commandWithReply
        commandWithReply(cmd, lxResp);
        // Make sure '#' is appended
        size_t len = strlen(lxResp);
        if (len == 0 || lxResp[len - 1] != '#') {
          if (len < sizeof(lxResp) - 1) {
            lxResp[len] = '#';
            lxResp[len + 1] = '\0';
          }
        }
      }

      // Send response back over serial
      SERIAL_ESP32.print(lxResp);
      SERIAL_ESP32.flush();

      //SERIAL_DEBUG.printf("MSG: LX200, Cmd: %-14s  Resp: %s\n", lxCmd.c_str(), lxResp);

      //unsigned long tEnd = micros();  // End timing
      ///SERIAL_DEBUG.printf("MSG: LX200, lxPoll() duration: %lu µs\n", tEnd - tStart);
    }
  }
}

LX200Handler lx200Handler;