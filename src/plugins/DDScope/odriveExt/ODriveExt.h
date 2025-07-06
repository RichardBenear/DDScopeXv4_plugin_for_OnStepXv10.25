// =====================================================
// ODriveExt.h

//#if ODRIVE_COMM_MODE == OD_UART
  //#include <ODriveArduino.h>
//#endif
 
//#include "../display/Display.h"
// Forward declaration to avoid circular dependency
//class Display;

#ifndef ODRIVEEXT_H
#define ODRIVEEXT_H

#define AZM_VEL_GAIN_DEF      1.5 // do not change unless changed in ODrive setup, default value
#define AZM_VEL_GAIN_HI       1.8
#define ALT_VEL_GAIN_DEF      0.3 // do not change unless changed in ODrive setup, default value
#define ALT_VEL_GAIN_HI       0.5 //
#define AZM_VEL_INT_GAIN_DEF  2.0 // do not change unless changed in ODrive setup, default value
#define AZM_VEL_INT_GAIN_HI   2.3
#define ALT_VEL_INT_GAIN_DEF  0.4 // do not change unless changed in ODrive setup, default value
#define ALT_VEL_INT_GAIN_HI   0.2 // 0.7

enum Component
{
  COMP_FIRST,
  NO_COMP,
  AXIS,
  CONTROLLER,
  MOTOR,
  ENCODER,
  COMP_LAST
};

class ODriveExt {
  public:
    
    // getters
    int getMotorPositionCounts(int axis);
    uint8_t getODriveCurrentState(int axis);

    float getEncoderPositionDeg(int axis);
    float getMotorPositionTurns(int axis);
    float getMotorPositionDelta(int axis);
    float getMotorCurrent(int axis);
    float getMotorTemp(int axis);
    float getODriveVelGain(int axis);
    float getODriveVelIntGain(int axis);
    float getODrivePosGain(int axis);
    float getODriveBusVoltage(int axis);

    uint32_t getODriveErrors(int axis, Component component);
    void demoMode();
    
    // other actions
    void setODriveVelGains(int axis, float level, float intLevel);
    void setODrivePosGain(int axis, float level);
    //void updateODriveMotorPositions();
    void MotorEncoderDelta();
    //void clearODriveErrors(int axis, int comp);
    //void setHigherBaud();
    void clearAllODriveErrors();

    bool oDserialAvail = false;

    Component component = COMP_FIRST;
   
    bool AZgainHigh;
    bool ALTgainHigh;
    bool AZgainDefault;   
    bool ALTgainDefault;
    
  private:
    bool batLowLED = false;
    bool oDriveRXoff = false;
};

extern ODriveExt oDriveExt;

#endif
