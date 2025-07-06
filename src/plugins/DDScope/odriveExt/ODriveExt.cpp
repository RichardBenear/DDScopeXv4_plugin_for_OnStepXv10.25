// ============================================
// ODriveExt.cpp
//
// ODrive Extended Support functions
// Author: Richard Benear 2022
// 
// ODrive communication via Teensy 4.0 serial
// Uses GitHub ODrive Arduino library

#include "../display/Display.h"
#include "ODriveExt.h"
#include "src/lib/axis/motor/oDrive/ODrive.h"
#include "../../../telescope/mount/Mount.h"
#include "../../../lib/tasks/OnTask.h"

const char* ODriveComponentsStr[4] = {
            "None",   
            "Controller",
            "Motor",
            "Encoder"        
};

// Printing with stream operator helper functions
template<class T> inline Print& operator <<(Print& obj,     T arg) { obj.print(arg   ); return obj; }
template<>        inline Print& operator <<(Print& obj, float arg) { obj.print(arg, 4); return obj; }

// ==============================================================================
// NOTE: A change to the HardwareSerial.cpp library was made.
// In HardwareSerial.cpp, this line was changed: PUS(3) was changed to PUS(2)
// PUS() is Pullup Strength for the Teensy RX PAD. 3 = 22K ohm, 2 = 100K ohm
// When set to 22K ohm, the ODrive TX signal would only have a low of .4 volt above ground.
// When set to 100K ohm, the ODrive TX signal would be about 20mv above ground.
// Apparently, the ODrive TX drive strength isn't very high
// But, it's not clear if this has any functional benefit; maybe better noise immunity.
//	*(portControlRegister(hardware->rx_pins[rx_pin_index_].pin)) = IOMUXC_PAD_DSE(7) | IOMUXC_PAD_PKE | IOMUXC_PAD_PUE | IOMUXC_PAD_PUS(2) | IOMUXC_PAD_HYS;
//===============================================================================

// ================ ODrive "writes" ======================
// ODrive clear ALL errors
void ODriveExt::clearAllODriveErrors() {
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "w sc\n";
  #elif ODRIVE_COMM_MODE == OD_CAN
    _oDriveDriver->ClearErrors(0);
    _oDriveDriver->ClearErrors(1);
  #else
  #endif 
} 

// Set ODrive Gains
void ODriveExt::setODriveVelGains(int axis, float level, float intLevel) {
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_gain "<<level<<'\n';
    ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_integrator_gain "<<intLevel<<'\n';
  #elif ODRIVE_COMM_MODE == OD_CAN
    _oDriveDriver->SetVelocityGains(axis, level, intLevel);
  #endif
}

void ODriveExt::setODrivePosGain(int axis, float level) {
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.pos_gain "<<level<<'\n';
  #elif ODRIVE_COMM_MODE == OD_CAN
    _oDriveDriver->SetPositionGain(axis, level);
  #endif
}

// NOTE: Since the ODriveArduino library has up to 1000ms timeout waiting for a RX character,
// a tasks.yield() is added after every read command.
// NOTE: if the RX data from ODrive drops out or the ODrive is off during debug, then just return
// from any of the follow "read" routines with -9.

// Read ODrive bus voltage which is approx. the battery voltage
// Battery Low LED is only on when battery is below low threashold
float ODriveExt::getODriveBusVoltage(int axis) {
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r vbus_voltage\n";
    float battery_voltage = _oDriveDriver->readFloat();
  #elif ODRIVE_COMM_MODE == OD_CAN
    float battery_voltage = _oDriveDriver->GetVbusVoltage(axis);  //Can be sent to either axis
  #endif

  // Handle timeout condition:
  // ----Post Note: seems that even when ODrive is off that CAN returns something once a 
  // ----shorter timeout of 10 msec was included in the sendMessage so the following is commented out.
  // if (battery_voltage == 0.0F) {
  //   digitalWrite(BATTERY_LOW_LED_PIN, LOW); // LED on
  //   batLowLED = true;
  //   oDriveRXoff = true; 
  //   return battery_voltage = 0.0F;  // Set to a valid value like 0
  // } else if (battery_voltage < BATTERY_LOW_VOLTAGE) { 
  if (battery_voltage < BATTERY_LOW_VOLTAGE) { 
    digitalWrite(BATTERY_LOW_LED_PIN, LOW); // LED on
    batLowLED = true;
    oDriveRXoff = false; 
    return battery_voltage;
  } else {
    digitalWrite(BATTERY_LOW_LED_PIN, HIGH); // LED off
    oDriveRXoff = false;
    batLowLED = false;
    return battery_voltage;
  } 
}

// get absolute Encoder positions in degrees
float ODriveExt::getEncoderPositionDeg(int axis) {
  if (oDriveRXoff) return -9.9; // arbitrary number
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; Y;
    float turns = _oDriveDriver->readFloat();
  #elif ODRIVE_COMM_MODE == OD_CAN
    float turns = _oDriveDriver->GetPosition(axis);
  #endif
  return turns*360;
}  

// get motor positions in turns
float ODriveExt::getMotorPositionTurns(int axis) {
  if (oDriveRXoff) return -9.9;
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; Y;
    float turns = _oDriveDriver->readFloat();
  #elif ODRIVE_COMM_MODE == OD_CAN
    float turns = _oDriveDriver->GetPosition(axis);
  #endif
  return turns;
}  

// get motor position in counts
int ODriveExt::getMotorPositionCounts(int axis) {
  if (oDriveRXoff) return -9.9;
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate_counts\n"; Y;
    int counts = _oDriveDriver->readInt();
  #elif ODRIVE_COMM_MODE == OD_CAN
    int counts = (int)_oDriveDriver->GetEncoderCountInCPR(axis);
  #endif
  return counts;
} 

// get Motor Current
float ODriveExt::getMotorCurrent(int axis) {
  if (oDriveRXoff) return -9.9;
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r axis" << axis << ".motor.I_bus\n"; Y;
    float Iq = _oDriveDriver->readFloat();
  #elif ODRIVE_COMM_MODE == OD_CAN
    float Iq = _oDriveDriver->GetIqMeasured(axis);
  #endif
  return Iq;
}  

// read current state
uint8_t ODriveExt::getODriveCurrentState(int axis) {
  #if ODRIVE_COMM_MODE == OD_UART
  ODRIVE_SERIAL << "r axis" << axis << ".current_state\n";
    uint8_t cState = _oDriveDriver->readInt();
  #elif ODRIVE_COMM_MODE == OD_CAN
    uint8_t cState = _oDriveDriver->GetCurrentState(axis);
  #endif
  return cState;
}

// Get the difference between ODrive setpoint and the encoder
float ODriveExt::getMotorPositionDelta(int axis) {
  
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r axis" << axis << ".controller.pos_setpoint\n";
    float reqPos = _oDriveDriver->readFloat();   
    ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n";
    float posEst = _oDriveDriver->readFloat(); 
    float deltaPos = fabs(reqPos - posEst);
  #elif ODRIVE_COMM_MODE == OD_CAN
    double currentTarget = 0.0000;
    float target = 0.0000;
    float actual = 0.0000;
    float deltaPos = 0.0000;
    if (ODRIVE_SWAP_AXES == ON) {
      if (axis == ALT_MOTOR) currentTarget = axis2.getTargetCoordinate();
      if (axis == AZM_MOTOR) currentTarget = axis1.getTargetCoordinate();
    } else {
      if (axis == ALT_MOTOR) currentTarget = axis1.getTargetCoordinate();
      if (axis == AZM_MOTOR) currentTarget = axis2.getTargetCoordinate();
    }
    // the following variables are in fractional "turns"
    target = ((float)currentTarget*RAD_DEG_RATIO)/360;
    //int32_t enc_actual = _oDriveDriver->GetEncoderShadowCount(axis); // 2^14 = 16384 counts per revolution
    //actual = (float)enc_actual/16384;
    actual = _oDriveDriver->GetPosition(axis);
    deltaPos = (target - actual); 
    //char deltaPosS[9]="";
    //char actualS[9]="";
    //char targetS[9]="";
    //if (axis == 1) {
    //  sprintf(actualS, "%0.5f", actual);
    //  sprintf(targetS, "%0.5f", target);
    //  sprintf(deltaPosS, "%0.5f", deltaPos);
    //  VF("act="); VL(actualS);
    //  VF("targ="); VL(targetS);
    //  VF("delta="); VL(deltaPosS);
    //} 
  #endif  
  return fabs(deltaPos);
}

// Check encoders to see if positions are too far outside range of requested position inferring that there are interfering forces
// This will warn that the motors may be getting too hot since more current is required trying to move them to the requested position
void ODriveExt::MotorEncoderDelta() {
  if (oDriveRXoff) return;
  if (axis1.isEnabled()) {
    float AZposDelta = getMotorPositionDelta(AZM_MOTOR);
    if (AZposDelta > 0.0020 && AZposDelta < 0.03) 
      display.soundFreq(1700, 65);
    else if (AZposDelta > 0.03) display.soundFreq(1800, 65); // saturated
  }
  
  if (axis2.isEnabled()) {
    float ALTposDelta = getMotorPositionDelta(ALT_MOTOR);
    if (ALTposDelta > .0050 && ALTposDelta < 0.03) {
      display.soundFreq(1500, 35);
    } else if (ALTposDelta > 0.03) display.soundFreq(1600, 35); 
  }
}

// Get ODrive gains
float ODriveExt::getODriveVelGain(int axis) {
  if (oDriveRXoff) return -9.9;
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_gain\n"; Y;
    return _oDriveDriver->readFloat();
  #elif ODRIVE_COMM_MODE == OD_CAN
    // not a commmand that is implemented with "CAN Simple" on ODrive so just return built-in constants
    if (axis == 1) { // 1=AZM motor, 0=ALT Motor
      if (!AZgainHigh) return AZM_VEL_GAIN_DEF; else return AZM_VEL_GAIN_HI;    
    } else {
      if (!ALTgainHigh) return ALT_VEL_GAIN_DEF; else return ALT_VEL_GAIN_HI;    
    }
  #endif
}

float ODriveExt::getODriveVelIntGain(int axis) {
  if (oDriveRXoff) return -9.9;
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_integrator_gain\n"; Y;
    return _oDriveDriver->readFloat();
  #elif ODRIVE_COMM_MODE == OD_CAN
    // not a commmand that is implemented with "CAN Simple" on ODrive so just return built-in constants
    if (axis == 1) { //1=AZM Motor, 0=ALT Motor
      if (!AZgainHigh) return AZM_VEL_INT_GAIN_DEF; else return AZM_VEL_INT_GAIN_HI;    
    } else {
      if (!ALTgainHigh) return ALT_VEL_INT_GAIN_DEF; else return ALT_VEL_INT_GAIN_HI;    
    }
  #endif
}

float ODriveExt::getODrivePosGain(int axis) {
  if (oDriveRXoff) return -9.9;
  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.pos_gain\n"; Y;
    return _oDriveDriver->readFloat();
  #elif ODRIVE_COMM_MODE == OD_CAN
    return 99; // not implemented
  #endif
}

// ========  Get the ODRIVE errors ========
uint32_t ODriveExt::getODriveErrors(int axis, Component component) {
  if (oDriveRXoff) return 99;
  uint32_t axisErr = 99;

  // ODrive Top Level Error
  if (axis == -1) {
    #if ODRIVE_COMM_MODE == OD_UART
      ODRIVE_SERIAL << "r odrive.error\n"; Y;
      return axisErr = (uint32_t)_oDriveDriver->readInt();
    #elif ODRIVE_COMM_MODE == OD_CAN
      return axisErr = 88; // No implementation for this in CAN bus
    #endif
  } else {
    // ODrive Errors (Axis, Controller, Motor, or Encoder)
    #if ODRIVE_COMM_MODE == OD_UART
      if (component == NO_COMP) { // axis error
        ODRIVE_SERIAL << "r odrive.axis"<<axis<<".error\n"; Y;
        return axisErr = (uint32_t)_oDriveDriver->readInt();
      } else { // component of the Axis error
        ODRIVE_SERIAL << "r odrive.axis"<<axis<<"."<<ODriveComponentsStr[component]<<".error\n"; Y;
        return axisErr = (uint32_t)_oDriveDriver->readInt();
      }
    #elif ODRIVE_COMM_MODE == OD_CAN
      if (component == AXIS) {
        return axisErr = _oDriveDriver->GetAxisError(axis);
      } else if (component == MOTOR) {
        return axisErr = _oDriveDriver->GetMotorError(axis);
      } else if (component == ENCODER) {
        return axisErr = _oDriveDriver->GetEncoderError(axis);
      } else if (component == CONTROLLER) {
        return axisErr = _oDriveDriver->GetControllerFlags(axis);
      } else {
        return axisErr = 99;
      }
    #endif 
  }
}

// =========== Motor Thermistor Support =============
float ODriveExt::getMotorTemp(int axis) {
  int Ro = 9, B =  3950; //Nominal resistance 10K, Beta constant, 9k at 68 deg
  int Rseries = 10.0;// Series resistor 10K
  float To = 293; // Nominal Temperature 68 deg calibration point
  float Vi = 0;

  /*Read analog output of NTC module,
    i.e the voltage across the thermistor */
  // IMPORTANT: USE 3.3V for Thermistor!! Teensy pins are NOT 5V tolerant!
  if (axis == 1) 
    Vi = analogRead(ALT_THERMISTOR_PIN) * (3.3 / 1023.0);
  else
    Vi = analogRead(AZ_THERMISTOR_PIN) * (3.3 / 1023.0);
  //Convert voltage measured to resistance value
  //All Resistance are in kilo ohms.
  float R = (Vi * Rseries) / (3.3 - Vi);
  /*Use R value in steinhart and hart equation
    Calculate temperature value in kelvin*/
  float T =  1 / ((1 / To) + ((log(R / Ro)) / B));
  float Tc = T - 273.15; // Converting kelvin to celsius
  float Tf = Tc * 9.0 / 5.0 + 32.0; // Converting celsius to Fahrenheit
  return Tf;
}

// =================== Demo Mode ====================
// Demo mode requires that the OnStep updates to the Axis be stopped but the Motor power is ON
// Demo mode repeats a sequence of moves
void ODriveExt::demoMode() {
  // choosing some AZM and ALT positions in fractional "Turns"
  // ALT position should never be negative in actual use because of the position of the focuser but it "can" go negative in demo
 
  float pos_one = 0.15;
  float pos_two = 0.3;
  float pos_three = -0.1;
  float pos_four = -0.4;
  float pos_five = 0.4;
  int demo_pos = 0;

  switch(demo_pos) {
    case 0:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_one);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_one);
      ++demo_pos;
      break;
    case 1:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_two);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_two);
      ++demo_pos;
      break;
    case 2:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_three);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_one);
      ++demo_pos;
      break;
    case 3:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_four);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_five);
      demo_pos = 0;
      break;
    default:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_one);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_one);
      demo_pos = 0;
      break;
  }
}

ODriveExt oDriveExt;