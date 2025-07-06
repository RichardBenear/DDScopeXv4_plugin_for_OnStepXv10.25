// =====================================================
// ODriveScreen.cpp
//
// Author: Richard Benear 2022
#include "ODriveScreen.h"
#include "../../../telescope/mount/Mount.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/lib/tasks/OnTask.h"
#include <ODriveArduino.h> // https://github.com/odriverobotics/ODrive/tree/master/Arduino/ODriveArduino

#define OD_ERR_OFFSET_X 4
#define OD_ERR_OFFSET_Y 190
#define OD_ERR_SPACING 11
#define OD_BUTTONS_OFFSET 45

// Buttons for actions that are not page selections
#define OD_ACT_BOXSIZE_X 100
#define OD_ACT_BOXSIZE_Y 36
#define OD_ACT_COL_1_X 3
#define OD_ACT_COL_1_Y 324
#define OD_ACT_COL_2_X OD_ACT_COL_1_X + OD_ACT_BOXSIZE_X + 4
#define OD_ACT_COL_2_Y OD_ACT_COL_1_Y
#define OD_ACT_COL_3_X OD_ACT_COL_2_X + OD_ACT_BOXSIZE_X + 4
#define OD_ACT_COL_3_Y OD_ACT_COL_1_Y
#define OD_ACT_X_SPACING 7
#define OD_ACT_Y_SPACING 3

// Printing with stream operator helper functions
template <class T> inline Print &operator<<(Print &obj, T arg) {
  obj.print(arg);
  return obj;
}
template <> inline Print &operator<<(Print &obj, float arg) {
  obj.print(arg, 4);
  return obj;
}

// ODrive Screen Button object
Button odriveButton(OD_ACT_COL_1_X, OD_ACT_COL_1_Y, OD_ACT_BOXSIZE_X,
                    OD_ACT_BOXSIZE_Y, butOnBackground, butBackground,
                    butOutline, mainFontWidth, mainFontHeight, "");

// Demo Mode Wrapper
void demoWrapper() { oDriveExt.demoMode(); }

typedef struct ODriveVersion {
  uint16_t hwMajor;
  uint16_t hwMinor;
  uint16_t hwVar;
  uint16_t fwMajor;
  uint16_t fwMinor;
  uint16_t fwRev;
} ODriveVersion;

ODriveVersion oDversion;

//****** Draw ODrive Screen ******
void ODriveScreen::draw() {
  setCurrentScreen(ODRIVE_SCREEN);
  odriveButton.setColors(butOnBackground, butBackground, butOutline);

#ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(true);
#endif
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);

  drawMenuButtons();
  drawTitle(120, TITLE_TEXT_Y, "ODrive");
  tft.setFont(&Inconsolata_Bold8pt7b);

#if ODRIVE_COMM_MODE == OD_UART
  ODRIVE_SERIAL << "r hw_version_major\n";
  oDversion.hwMajor = _oDriveDriver->readInt();

  ODRIVE_SERIAL << "r hw_version_minor\n";
  oDversion.hwMinor = _oDriveDriver->readInt();

  ODRIVE_SERIAL << "r hw_version_variant\n";
  oDversion.hwVar = _oDriveDriver->readInt();

  ODRIVE_SERIAL << "r fw_version_major\n";
  oDversion.fwMajor = _oDriveDriver->readInt();

  ODRIVE_SERIAL << "r fw_version_minor\n";
  oDversion.fwMinor = _oDriveDriver->readInt();

  ODRIVE_SERIAL << "r fw_version_revision\n";
  oDversion.fwRev = _oDriveDriver->readInt();
#elif ODRIVE_COMM_MODE == OD_CAN
  // needs implemented
  oDversion.hwMajor = 3;
  oDversion.hwMinor = 6;
  oDversion.hwVar = 24;
  oDversion.fwMajor = 0;
  oDversion.fwMinor = 5;
  oDversion.fwRev = 4;
#endif

  // Entering the ODrive screen will always set the gains back to default
  oDriveExt.AZgainHigh = false;
  oDriveExt.AZgainDefault = true;
  oDriveExt.ALTgainHigh = false;
  oDriveExt.ALTgainDefault = true;

  tft.setFont(0); // default
  tft.setCursor(92, 164);
  tft.print("HW Version:");
  tft.print(oDversion.hwMajor);
  tft.print(".");
  tft.print(oDversion.hwMinor);
  tft.print(".");
  tft.print(oDversion.hwVar);
  tft.setCursor(92, 175);
  tft.print("FW Version:");
  tft.print(oDversion.fwMajor);
  tft.print(".");
  tft.print(oDversion.fwMinor);
  tft.print(".");
  tft.print(oDversion.fwRev);

  drawCommonStatusLabels();
  updateCommonStatus();
  updateOdriveButtons();
  updateOdriveStatus();
  showGains();
  showODriveErrors();
  showGpsStatus();
#ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(false);
  wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
#endif
#ifdef ENABLE_TFT_CAPTURE
  tft.saveBufferToSD("ODrive");
#endif
}

// status update for this screen
void ODriveScreen::updateOdriveStatus() {
  // showODriveErrors();
}

// ====== Show the Gains ======
void ODriveScreen::showGains() {
  // Show AZM Velocity Gain - AZ is motor=1, ALT is motor=0
  tft.setFont(0);
  float temp = oDriveExt.getODriveVelGain(AZM_MOTOR);
  tft.setCursor(206, 282);
  tft.print("AZM Vel  Gain:");
  tft.fillRect(288, 282, 30, 10, pgBackground);
  tft.setCursor(288, 282);
  tft.print(temp);

  // Show AZM Velocity Integrator Gain
  temp = oDriveExt.getODriveVelIntGain(AZM_MOTOR);
  tft.setCursor(206, 292);
  tft.print("AZM VelI Gain:");
  tft.fillRect(288, 292, 30, 10, pgBackground);
  tft.setCursor(288, 292);
  tft.print(temp);

  // Show ALT Velocity Gain - ALT is motor 0
  temp = oDriveExt.getODriveVelGain(ALT_MOTOR);
  tft.setCursor(206, 302);
  tft.print("ALT Vel  Gain:");
  tft.fillRect(288, 302, 30, 10, pgBackground);
  tft.setCursor(288, 302);
  tft.print(temp);

  // Show ALT Velocity Integrator Gain
  temp = oDriveExt.getODriveVelIntGain(ALT_MOTOR);
  tft.setCursor(206, 312);
  tft.print("ALT VelI Gain:");
  tft.fillRect(288, 312, 30, 10, pgBackground);
  tft.setCursor(288, 312);
  tft.print(temp);
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// ===== Decode all ODrive Errors =====
uint8_t ODriveScreen::decodeODriveTopErrors(int axis, uint32_t odErrorCode, int y_offset) {
  uint8_t errorCount = 0; 
                          
  tft.setFont(0);
  tft.setCursor(OD_ERR_OFFSET_X, y_offset);

  if (axis == -1) { // ODrive top-level errors, not axis-specific
    if (odErrorCode & ODRIVE_ERROR_CONTROL_ITERATION_MISSED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("ERROR_CONTROL_ITERATION_MISSED");
      errorCount++;
    }
    if (odErrorCode & ODRIVE_ERROR_DC_BUS_UNDER_VOLTAGE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("ERROR_DC_BUS_UNDER_VOLTAGE");
      errorCount++;
    }
    if (odErrorCode & ODRIVE_ERROR_DC_BUS_OVER_VOLTAGE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("ERROR_DC_BUS_OVER_VOLTAGE");
      errorCount++;
    }
    if (odErrorCode & ODRIVE_ERROR_DC_BUS_OVER_REGEN_CURRENT) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("ERROR_DC_BUS_OVER_REGEN_CURRENT");
      errorCount++;
    }
    if (odErrorCode & ODRIVE_ERROR_DC_BUS_OVER_CURRENT) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("ERROR_DC_BUS_OVER_CURRENT");
      errorCount++;
    }
    if (odErrorCode & ODRIVE_ERROR_BRAKE_DEADTIME_VIOLATION) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("ERROR_BRAKE_DEADTIME_VIOLATION");
      errorCount++;
    }
    if (odErrorCode & ODRIVE_ERROR_BRAKE_DUTY_CYCLE_NAN) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("ERROR_BRAKE_DUTY_CYCLE_NAN");
      errorCount++;
    }
    if (odErrorCode & ODRIVE_ERROR_INVALID_BRAKE_RESISTANCE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("ERROR_INVALID_BRAKE_RESISTANCE");
      errorCount++;
    }
    if (odErrorCode == 0x88) { // Special case: "Not Implemented"
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("NOT_IMPLEMENTED");
      errorCount++;
    }
    if (errorCount == 0) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("TOP_LEVEL_ERR_NONE"); // No errors found
      errorCount++;
    }
  }
  return errorCount;
}
  
uint8_t ODriveScreen::decodeODriveAxisErrors(int axis, uint32_t odErrorCode, int y_offset) {
  uint8_t errorCount = 0;
  tft.setFont(0);
  tft.setCursor(OD_ERR_OFFSET_X, y_offset);
  // Axis Errors
  if (axis == 1 || axis == 0) {
    if (odErrorCode & AXIS_ERROR_INVALID_STATE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("AX_ERROR_INVALID_STATE");
      errorCount++;
    }
    if (odErrorCode & AXIS_ERROR_WATCHDOG_TIMER_EXPIRED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("AX_ERROR_WATCHDOG_TIMER_EXPIRED");
      errorCount++;
    }
    if (odErrorCode & AXIS_ERROR_MIN_ENDSTOP_PRESSED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("AX_ERROR_MIN_ENDSTOP_PRESSED");
      errorCount++;
    }
    if (odErrorCode & AXIS_ERROR_MAX_ENDSTOP_PRESSED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("AX_ERROR_MAX_ENDSTOP_PRESSED");
      errorCount++;
    }
    if (odErrorCode & AXIS_ERROR_ESTOP_REQUESTED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("AX_ERROR_ESTOP_REQUESTED");
      errorCount++;
    }
    if (odErrorCode & AXIS_ERROR_HOMING_WITHOUT_ENDSTOP) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("AX_ERROR_HOMING_WITHOUT_ENDSTOP");
      errorCount++;
    }
    if (odErrorCode & AXIS_ERROR_OVER_TEMP) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("AX_ERROR_OVER_TEMP");
      errorCount++;
    }
    if (odErrorCode & AXIS_ERROR_UNKNOWN_POSITION) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("AX_ERROR_UNKNOWN_POSITION");
      errorCount++;
    }
    if (errorCount == 0) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("AXIS_ERROR_NONE");
      errorCount++;
    }
  }
  return errorCount;
}
  
uint8_t ODriveScreen::decodeODriveMotorErrors(int axis, uint32_t odErrorCode, int y_offset) {
  uint8_t errorCount = 0;
  tft.setFont(0);
  tft.setCursor(OD_ERR_OFFSET_X, y_offset);
  // Motor Errors
  if (axis == 1 || axis == 0) {
    if (odErrorCode & MOTOR_ERROR_PHASE_RESISTANCE_OUT_OF_RANGE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_PHASE_RESISTANCE_OUT_OF_RANGE");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_PHASE_INDUCTANCE_OUT_OF_RANGE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_PHASE_INDUCTANCE_OUT_OF_RANGE");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_DRV_FAULT) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_DRV_FAULT");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_CONTROL_DEADLINE_MISSED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_CONTROL_DEADLINE_MISSED");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_MODULATION_MAGNITUDE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_MODULATION_MAGNITUDE");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_CURRENT_SENSE_SATURATION) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_CURRENT_SENSE_SATURATION");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_CURRENT_LIMIT_VIOLATION) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_CURRENT_LIMIT_VIOLATION");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_MODULATION_IS_NAN) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_MODULATION_IS_NAN");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_MOTOR_THERMISTOR_OVER_TEMP) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_MOTOR_THERMISTOR_OVER_TEMP");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_FET_THERMISTOR_OVER_TEMP) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_FET_THERMISTOR_OVER_TEMP");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_TIMER_UPDATE_MISSED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_TIMER_UPDATE_MISSED");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_CURRENT_MEASUREMENT_UNAVAILABLE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_CURRENT_MEASUREMENT_UNAVAIL");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_CONTROLLER_FAILED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_CONTROLLER_FAILED");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_I_BUS_OUT_OF_RANGE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_I_BUS_OUT_OF_RANGE");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_BRAKE_RESISTOR_DISARMED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_BRAKE_RESISTOR_DISARMED");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_SYSTEM_LEVEL) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_SYSTEM_LEVEL");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_BAD_TIMING) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_BAD_TIMING");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_UNKNOWN_PHASE_ESTIMATE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_UNKNOWN_PHASE_ESTIMATE");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_UNKNOWN_PHASE_VEL) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_UNKNOWN_PHASE_VEL");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_UNKNOWN_TORQUE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_UNKNOWN_TORQUE");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_UNKNOWN_CURRENT_COMMAND) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_UNKNOWN_CURRENT_COMMAND");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_UNKNOWN_CURRENT_MEASUREMENT) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_UNKNOWN_CURRENT_MEASUREMENT");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_UNKNOWN_VBUS_VOLTAGE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_UNKNOWN_VBUS_VOLTAGE");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_UNKNOWN_VOLTAGE_COMMAND) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_UNKNOWN_VOLTAGE_COMMAND");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_UNKNOWN_GAINS) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_UNKNOWN_GAINS");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_CONTROLLER_INITIALIZING) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_CONTROLLER_INITIALIZING");
      errorCount++;
    }
    if (odErrorCode & MOTOR_ERROR_UNBALANCED_PHASES) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("M_ERR_UNBALANCED_PHASES");
      errorCount++;
    }
    if (errorCount == 0) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("MOTOR_ERR_NONE");
      errorCount++;
    }
  }
  return errorCount;
}

uint8_t ODriveScreen::decodeODriveContErrors(int axis, uint32_t odErrorCode, int y_offset) {
  uint8_t errorCount = 0;
  tft.setFont(0);
  tft.setCursor(OD_ERR_OFFSET_X, y_offset);
#if ODRIVE_COMM_MODE == OD_UART
  if (axis != -1) {
    if (odErrorCode == CONTROLLER_ERROR_NONE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_NONE");
      errorCount++;
    } else if (odErrorCode == CONTROLLER_ERROR_OVERSPEED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_OVERSPEED");
      errorCount++;
    } else if (odErrorCode == CONTROLLER_ERROR_INVALID_INPUT_MODE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_INVALID_INPUT_MODE");
      errorCount++;
    } else if (odErrorCode == CONTROLLER_ERROR_UNSTABLE_GAIN) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_UNSTABLE_GAIN");
      errorCount++;
    } else if (odErrorCode == CONTROLLER_ERROR_INVALID_MIRROR_AXIS) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_INVALID_MIRROR_AXIS");
      errorCount++;
    } else if (odErrorCode == CONTROLLER_ERROR_INVALID_LOAD_ENCODER) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_INVALID_LOAD_ENCODER");
      errorCount++;
    } else if (odErrorCode == CONTROLLER_ERROR_INVALID_ESTIMATE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_INVALID_ESTIMATE");
      errorCount++;
    } else if (odErrorCode == CONTROLLER_ERROR_INVALID_CIRCULAR_RANGE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_INVALID_CIRCULAR_RANGE");
      errorCount++;
    } else if (odErrorCode == CONTROLLER_ERROR_SPINOUT_DETECTED) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_SPINOUT_DETECTED");
      errorCount++;
    } else {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("C_ERR_UNKNOWN");
      errorCount++;
    }
    return errorCount;
  }
#elif ODRIVE_COMM_MODE == OD_CAN
  if (axis == 1 || axis == 0) {
    if (odErrorCode == 128) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("Controller Trajectory Done");
      errorCount++;
    }
    if (errorCount == 0) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("CONTROLLER_ERR_NONE");
      errorCount++;
    }
  }
#endif
  return errorCount;
}

uint8_t ODriveScreen::decodeODriveEncErrors(int axis, uint32_t odErrorCode, int y_offset) {
  uint8_t errorCount = 0;
  tft.setFont(0);
  tft.setCursor(OD_ERR_OFFSET_X, y_offset);
  // Encoder Errors
  if (axis == 1 || axis == 0) {
    if (odErrorCode & ENCODER_ERROR_UNSTABLE_GAIN) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("E_ERR_UNSTABLE_GAIN");
      errorCount++;
    }
    if (odErrorCode & ENCODER_ERROR_CPR_POLEPAIRS_MISMATCH) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("E_ERR_CPR_POLEPAIRS_MISMATCH");
      errorCount++;
    }
    if (odErrorCode & ENCODER_ERROR_NO_RESPONSE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("E_ERR_NO_RESPONSE");
      errorCount++;
    }
    if (odErrorCode & ENCODER_ERROR_UNSUPPORTED_ENCODER_MODE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("E_ERR_UNSUPPORTED_ENCODER_MODE");
      errorCount++;
    }
    if (odErrorCode & ENCODER_ERROR_ILLEGAL_HALL_STATE) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("E_ERR_ILLEGAL_HALL_STATE");
      errorCount++;
    }
    if (odErrorCode & ENCODER_ERROR_INDEX_NOT_FOUND_YET) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("E_ERR_INDEX_NOT_FOUND_YET");
      errorCount++;
    }
    if (odErrorCode & ENCODER_ERROR_ABS_SPI_TIMEOUT) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("E_ERR_ABS_SPI_TIMEOUT");
      errorCount++;
    }
    if (odErrorCode & ENCODER_ERROR_ABS_SPI_COM_FAIL) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("E_ERR_ABS_SPI_COM_FAIL");
      errorCount++;
    }
    if (odErrorCode & ENCODER_ERROR_ABS_SPI_NOT_READY) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("E_ERR_ABS_SPI_NOT_READY");
      errorCount++;
    }
    if (errorCount == 0) {
      tft.setCursor(OD_ERR_OFFSET_X, tft.getCursorY());
      tft.println("ENC_ERR_NONE");
      errorCount++;
    }
  }
  return errorCount; // Return the number of detected errors
}

// ======== Show the ODRIVE errors ========
void ODriveScreen::showODriveErrors() {
  int y_offset = 0;
  uint8_t err = 0;
  uint8_t errCnt = 0;

  // **** enum ordering: AXIS=2, CONTROLLER=3, MOTOR=4, ENCODER=5 *****//
  // 
  // clear background
  tft.fillRect(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y, 197, 15 * OD_ERR_SPACING, pgBackground);
  tft.setFont(0);
  
  // ODrive top level errors
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y);
  tft.println("-------Top Level Errors-------");

  y_offset = OD_ERR_OFFSET_Y + OD_ERR_SPACING;
  err = oDriveExt.getODriveErrors(-1, Component::NO_COMP); // no axis number since Top Level
  //sprintf(tempString, "Top Level err=%08lX", err); VL(tempString);
  errCnt = oDriveScreen.decodeODriveTopErrors(-1, err, y_offset);

  // AZM Errors
  y_offset += (errCnt * OD_ERR_SPACING+3);

  tft.setCursor(OD_ERR_OFFSET_X, y_offset);
  tft.println("----------AZM Errors----------");
  
  y_offset += (OD_ERR_SPACING);
  err = oDriveExt.getODriveErrors(AZM_MOTOR, AXIS);
  //sprintf(tempString, "AZM AXIS=%c err=%08lX", AXIS+48, err); VL(tempString);
  errCnt = oDriveScreen.decodeODriveAxisErrors(AZM_MOTOR, err, y_offset);
  y_offset += (errCnt * OD_ERR_SPACING);

  err = oDriveExt.getODriveErrors(AZM_MOTOR, CONTROLLER);
  //sprintf(tempString, "AZM CONTROLLER=%c err=%08lX", CONTROLLER+48, err); VL(tempString);
  errCnt = oDriveScreen.decodeODriveContErrors(AZM_MOTOR, err, y_offset);
  y_offset += (errCnt * OD_ERR_SPACING);

  err = oDriveExt.getODriveErrors(AZM_MOTOR, MOTOR);
  //sprintf(tempString, "AZM MOTOR=%c err=%08lX", MOTOR+48, err); VL(tempString);
  errCnt = oDriveScreen.decodeODriveMotorErrors(AZM_MOTOR, err, y_offset);
  y_offset += (errCnt * OD_ERR_SPACING);

  err = oDriveExt.getODriveErrors(AZM_MOTOR, ENCODER);
  //sprintf(tempString, "AZM ENCODER=%c err=%08lX", ENCODER+48, err); VL(tempString);
  errCnt = oDriveScreen.decodeODriveEncErrors(AZM_MOTOR, err, y_offset);
  y_offset += (errCnt * OD_ERR_SPACING+3);

  tft.setCursor(OD_ERR_OFFSET_X, y_offset);
  tft.println("----------ALT Errors----------");

  y_offset += OD_ERR_SPACING;
  err = oDriveExt.getODriveErrors(ALT_MOTOR, AXIS);
  //sprintf(tempString, "ALT AXIS=%c err=%08lX", AXIS+48, err); VL(tempString);
  errCnt = oDriveScreen.decodeODriveAxisErrors(ALT_MOTOR, err, y_offset);
  y_offset += (errCnt * OD_ERR_SPACING);

  err = oDriveExt.getODriveErrors(ALT_MOTOR, CONTROLLER);
  //sprintf(tempString, "ALT CONTROLLER=%c err=%08lX", CONTROLLER+48, err); VL(tempString);
  errCnt = oDriveScreen.decodeODriveContErrors(ALT_MOTOR, err, y_offset);
  y_offset += (errCnt * OD_ERR_SPACING);

  err = oDriveExt.getODriveErrors(ALT_MOTOR, MOTOR);
  //sprintf(tempString, "ALT MOTOR=%c err=%08lX", MOTOR+48, err); VL(tempString);
  errCnt = oDriveScreen.decodeODriveMotorErrors(AZM_MOTOR, err, y_offset);
  y_offset += (errCnt * OD_ERR_SPACING);

  err = oDriveExt.getODriveErrors(ALT_MOTOR, ENCODER);
  //sprintf(tempString, "ALT ENCODER=%c err=%08lX", ENCODER+48, err); VL(tempString);
  errCnt = oDriveScreen.decodeODriveEncErrors(ALT_MOTOR, err, y_offset);
}

bool ODriveScreen::odriveButStateChange() {
  bool changed = false;

  if (preAzmState != axis1.isEnabled()) {
    preAzmState = axis1.isEnabled();
    changed = true;
  }

  if (preAltState != axis2.isEnabled()) {
    preAltState = axis2.isEnabled();
    changed = true;
  }

  if (display.buttonTouched) {
    display.buttonTouched = false;
    if (resetODriveFlag || OdStopButton || clearODriveErrs || demoActive) {
      changed = true;
    }
    if (oDriveExt.AZgainHigh || oDriveExt.AZgainDefault || oDriveExt.ALTgainHigh || oDriveExt.ALTgainDefault) {
      changed = true;
    }
  }

  if (display._redrawBut) {
    display._redrawBut = false;
    changed = true;
  }
  return changed;
}

// ========  Update ODrive Page Buttons ========
void ODriveScreen::updateOdriveButtons() {

  tft.setFont(&Inconsolata_Bold8pt7b);

  int x_offset = 0;
  int y_offset = 0;
  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

  // ----- Column 1 -----
  // Enable / Disable Azimuth Motor
  if (axis1.isEnabled()) {
    odriveButton.draw(OD_ACT_COL_1_X, OD_ACT_COL_1_Y + y_offset, "AZM Enabled",
                      BUT_ON);
  } else { // motor off
    odriveButton.draw(OD_ACT_COL_1_X, OD_ACT_COL_1_Y + y_offset, "EN AZM",
                      BUT_OFF);
  }

  // Enable / Disable Altitude Motor
  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (axis2.isEnabled()) {
    odriveButton.draw(OD_ACT_COL_1_X, OD_ACT_COL_1_Y + y_offset, "ALT Enabled",
                      BUT_ON);
  } else { // motor off
    odriveButton.draw(OD_ACT_COL_1_X, OD_ACT_COL_1_Y + y_offset, "EN ALT",
                      BUT_OFF);
  }

  // ----- Column 2 -----
  // Stop all movement
  y_offset = 0;
  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (OdStopButton) {
    odriveButton.draw(OD_ACT_COL_2_X, OD_ACT_COL_2_Y + y_offset, "All Stop'd", BUT_ON);
    OdStopButton = false;
    display._redrawBut = true;
  } else {
    odriveButton.draw(OD_ACT_COL_2_X, OD_ACT_COL_2_Y + y_offset, "Motors OFF", BUT_OFF);
  }

  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Clear Errors
  if (clearODriveErrs) {
    odriveButton.draw(OD_ACT_COL_2_X, OD_ACT_COL_2_Y + y_offset, "Errs Cleared", BUT_ON);
    clearODriveErrs = false;
    display._redrawBut = true;
  } else {
    odriveButton.draw(OD_ACT_COL_2_X, OD_ACT_COL_2_Y + y_offset, "Clr Errors", BUT_OFF);
  }

  // ----- 3rd Column -----
  y_offset = -160;

  // AZ Gains High
  if (!oDriveExt.AZgainHigh) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "AZ Gain Hi", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "AZ Gain Hi", BUT_ON);
  }

  // AZ Gains Default
  y_offset += OD_ACT_BOXSIZE_Y - box_height_adj + OD_ACT_Y_SPACING;
  if (!oDriveExt.AZgainDefault) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "AZ Gain Def", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "AZ Gain Def", BUT_ON);
  }

  // ALT Velocity Gain High
  y_offset += OD_ACT_BOXSIZE_Y - box_height_adj + OD_ACT_Y_SPACING;
  if (!oDriveExt.ALTgainHigh) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "ALT Gain Hi", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "ALT Gain Hi", BUT_ON);
  }

  // ALT Velocity Gain Default
  y_offset += OD_ACT_BOXSIZE_Y - box_height_adj + OD_ACT_Y_SPACING;
  if (!oDriveExt.ALTgainDefault) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "ALT Gain Def", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "ALT Gain Def", BUT_ON);
  }

  //----------------------------------------
  y_offset = 0;
  // Demo Button
  if (demoActive) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "Demo Active", BUT_ON);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y - box_height_adj,
                      "Demo ODrive", BUT_OFF);
  }

  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Reset ODrive Button
  if (!resetODriveFlag) {
    odriveButton.draw(OD_ACT_COL_3_X, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, "Rst ODrive",
                      BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, "Resetting", BUT_ON);
    resetODriveFlag = false;
    display._redrawBut = true;
  }

  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Enable or Disable the ODrive position update via SERIAL
  if (ODpositionUpdateEnabled) {
    odriveButton.draw(OD_ACT_COL_3_X, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, "Mtr Loop On",
                      BUT_ON);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X, OD_ACT_COL_3_Y + y_offset,
                      OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, "Mtr Loop Off",
                      BUT_OFF);
  }
}

// =========== ODrive button update ===========
bool ODriveScreen::touchPoll(uint16_t px, uint16_t py) {
  int x_offset = 0;
  int y_offset = 0;
  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

  // ===== Column 1 - Leftmost ======
  // Enable Azimuth motor
  if (px > OD_ACT_COL_1_X + x_offset &&
      px < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_1_Y + y_offset &&
      py < OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!axis1.isEnabled()) {                // if not On, toggle ON
      digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
      axis1.enable(true);
    } else {                                  // since already ON, toggle OFF
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      axis1.enable(false);
    }
    return true;
  }

  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Enable Altitude motor
  if (px > OD_ACT_COL_1_X + x_offset &&
      px < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_1_Y + y_offset &&
      py < OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!axis2.isEnabled()) {                 // toggle ON
      digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
      axis2.enable(true);
    } else {                                   // toggle OFF
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
      axis2.enable(false);                     // turn off ODrive motor
    }
    return true;
  }

  // ----- Column 2 -----
  // STOP everthing requested
  y_offset = 0;
  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_2_X + x_offset &&
      px < OD_ACT_COL_2_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_2_Y + y_offset &&
      py < OD_ACT_COL_2_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!OdStopButton) {
      commandBool(":Q#"); // stops move
      axis1.enable(false);
      axis2.enable(false);
      OdStopButton = true;
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH);  // Turn Off AZ LED
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
      commandBool(":Td#");                     // Disable Tracking
    }
    return true;
  }

  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Clear ODrive Errors
  if (px > OD_ACT_COL_2_X + x_offset &&
      px < OD_ACT_COL_2_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_2_Y + y_offset &&
      py < OD_ACT_COL_2_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    VLF("MSG: Clearing ODrive Errors");
    oDriveExt.clearAllODriveErrors();
    clearODriveErrs = true;
    showODriveErrors();
    return true;
  }

  // Column 3
  y_offset = -165;
  // AZ Gain HIGH
  if (px > OD_ACT_COL_3_X + x_offset &&
      px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_3_Y + y_offset &&
      py < OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y - box_height_adj) {
    BEEP;
    oDriveExt.AZgainHigh = true;
    oDriveExt.AZgainDefault = false;
    oDriveExt.setODriveVelGains(AZM_MOTOR, AZM_VEL_GAIN_HI,
                                AZM_VEL_INT_GAIN_HI); // Set Velocity Gain
    delay(1); // wait for ODrive to process change
    showGains();
    return true;
  }

  // AZ Gain DEFault
  y_offset += OD_ACT_BOXSIZE_Y - box_height_adj + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset &&
      px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_3_Y + y_offset &&
      py < OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y - box_height_adj) {
    BEEP;
    oDriveExt.AZgainHigh = false;
    oDriveExt.AZgainDefault = true;
    oDriveExt.setODriveVelGains(AZM_MOTOR, AZM_VEL_GAIN_DEF,
                                AZM_VEL_INT_GAIN_DEF);
    delay(1);
    showGains();
    return true;
  }

  // ALT Gain HIGH
  y_offset += OD_ACT_BOXSIZE_Y - box_height_adj + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset &&
      px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_3_Y + y_offset &&
      py < OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y - box_height_adj) {
    BEEP;
    oDriveExt.ALTgainHigh = true;
    oDriveExt.ALTgainDefault = false;
    oDriveExt.setODriveVelGains(
        ALT_MOTOR, ALT_VEL_GAIN_HI,
        ALT_VEL_INT_GAIN_HI); // Set Velocity Gain, Integrator gain
    delay(1);
    showGains();
    return true;
  }

  // ALT Gain DEFault
  y_offset += OD_ACT_BOXSIZE_Y - box_height_adj + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset &&
      px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_3_Y + y_offset &&
      py < OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y - box_height_adj) {
    BEEP;
    oDriveExt.ALTgainHigh = false;
    oDriveExt.ALTgainDefault = true;
    oDriveExt.setODriveVelGains(ALT_MOTOR, ALT_VEL_GAIN_DEF,
                                ALT_VEL_INT_GAIN_DEF);
    delay(1);
    showGains();
    return true;
  }

  y_offset = 0;
  // Demo Mode for ODrive
  // Toggle on Demo Mode if button pressed, toggle off if pressed and already on
  // Demo Mode bypasses OnStep control to repeat a sequence of moves
  if (px > OD_ACT_COL_3_X + x_offset &&
      px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_3_Y + y_offset &&
      py < OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!demoActive) {
      commandBool(":Q#"); // does not turn off Motor power
      demoActive = true;

      // Start demo task
      VF("MSG: Setup, Demo Mode (rate 10 sec priority 6)... ");
      uint8_t demoHandle = tasks.add(10000, 0, true, 6, demoWrapper, "Demo");
      if (demoHandle) {
        VLF("success");
      } else {
        VLF("FAILED!");
      }
    } else {
      demoActive = false;
      VLF("MSG: Demo OFF ODrive");
      tasks.setDurationComplete(demoHandle);
    }
    return true;
  }

  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Reset ODRIVE
  if (px > OD_ACT_COL_3_X + x_offset &&
      px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_3_Y + y_offset &&
      py < OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    ALERT;
    VLF("MSG: Resetting ODrive");
    digitalWrite(ODRIVE_RST_PIN, LOW);
    delay(1);
    digitalWrite(ODRIVE_RST_PIN, HIGH);
    delay(1000); // Wait for ODrive to boot
    // oDriveScreen.draw();
    // ODRIVE_SERIAL.flush();
    // oDriveExt.clearAllODriveErrors();
    resetODriveFlag = true;
    oDriveExt.AZgainHigh = false;
    oDriveExt.AZgainDefault = true;
    oDriveExt.ALTgainHigh = false;
    oDriveExt.ALTgainDefault = true;
    return true;
  }

  // Disable / Enable ODrive motor position update which uses SERIAL or CAN
  // between ODrive and Teensy The purpose of Disable-position-updates so that
  // they don't override the ODrive motor positions while tuning the ODrive and
  // motors when using the ODrive USB serial port
  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset &&
      px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X &&
      py > OD_ACT_COL_3_Y + y_offset &&
      py < OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    ALERT;
    if (ODpositionUpdateEnabled) { // on, then toggle off
      axis1.enable(false);
      axis2.enable(false);
      ODpositionUpdateEnabled = false;
    } else { // off, then toggle on
      axis1.enable(true);
      axis2.enable(true);
      ODpositionUpdateEnabled = true;
    }
    return false;
  }

  // Check emergeyncy ABORT button area
  display.motorsOff(px, py);

  return true;
}

ODriveScreen oDriveScreen;