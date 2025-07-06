// =====================================================
// CmdDirect.cpp
//
// Local LX200 Handler
// Instead of using the SERIAL_LOCAL channel built into OnStep this
// directly accesses the OnStep commands. Essentially, it acts as one
// self contained way to get/set commands without relying on the tasking.
// The processCommand() function is a copy of that used in the webpage plugin
// but without the timeouts.
//
// The tasking for SERIAL_LOCAL was set at 3 msec and priority 5 and was
// the initial approach taken but the SERIAL_LOCAL task had to be changed
// to priority 2 and, even then, there were anomalous behaviour for updating
// the Wifi display (which uses USB and compression) and the WiFi LX200 handler. 
//
// Bottom line, I get quicker WiFi screen updates and deterministic behaviour
// for Stellarium and Sky Safari with this. Basically, this is a function call 
// to the many OnStep commands. Since I'm running Direct Drive motors with an
// external ODrive motor driver with it's own processor for the PID loop, the
// stepper motor updates are a non-issue.
// 
// Author: Richard Benear 7/3/2025
//

#include "CmdDirect.h"
#include "src/telescope/Telescope.h"

namespace {
    enum CmdError {
        CD_NONE, CD_0, CD_CMD_UNKNOWN, CD_REPLY_UNKNOWN, CD_PARAM_RANGE, CD_PARAM_FORM,
        CD_ALIGN_FAIL, CD_ALIGN_NOT_ACTIVE, CD_NOT_PARKED_OR_AT_HOME, CD_PARKED,
        CD_PARK_FAILED, CD_NOT_PARKED, CD_NO_PARK_POSITION_SET, CD_GOTO_FAIL, CD_LIBRARY_FULL,
        CD_SLEW_ERR_BELOW_HORIZON, CD_SLEW_ERR_ABOVE_OVERHEAD, CD_SLEW_ERR_IN_STANDBY, 
        CD_SLEW_ERR_IN_PARK, CD_SLEW_IN_SLEW, CD_SLEW_ERR_OUTSIDE_LIMITS, CD_SLEW_ERR_HARDWARE_FAULT,
        CD_SLEW_IN_MOTION, CD_SLEW_ERR_UNSPECIFIED, CD_DATE_TIME_NOT_READY,
        CD_NULL, CD_1
    };

    const char *cmdErrorStr[] = {
        "None", "Zero", "Unknown Cmd", "Unknown Reply", "Param Range", "Param Format",
        "Align Failed", "Align Not Active", "Not Parked or Home", "Parked",
        "Park Failed", "Not Parked", "No Park Set", "Goto Failed", "Lib Full",
        "Below Horizon", "Above Overhead", "In Standby",
        "In Park", "In Slew", "Outside Limits", "HW Fault",
        "In Motion", "Unspecified", "Time Not Ready",
        "Null", "One"
    };
}

CmdError cmdError;
CmdError lastCmdError;

CmdDirect::CmdDirect() {
    lastCmd[0] = '\0';  // empty string
    cmdError = CD_NONE;
    lastCmdError = CD_NONE;
}

const char *CmdDirect::getLastCommandErrorString() {
    return cmdErrorStr[lastCmdError];
}

int CmdDirect::getLastCmdError() const {
    return static_cast<int>(lastCmdError);
}

bool CmdDirect::processCommand(const char* cmd, char* response) {
  response[0] = 0;  // init output
  bool noResponse = false;
  bool shortResponse = false;

  // === Command Type Analysis ===
  if (cmd[0] == 6 && cmd[1] == 0) {
    shortResponse = true;
  }

  if (cmd[0] == ':' || cmd[0] == ';') {
    char c1 = cmd[1], c2 = cmd[2], c3 = cmd[3], c4 = cmd[4];

    if (c1 == 'G') {
      if (c2 == 'X' && ((c3 == 'E' && c4 == 'E') || (c3 == '8' && c4 == '9')))
        shortResponse = true;
    } else if (c1 == 'M') {
      if (strchr("ewnsg", c2)) noResponse = true;
      if (strchr("ADNPS", c2)) shortResponse = true;
    } else if (c1 == 'Q') {
      if (strchr("#ewns", c2)) noResponse = true;
    } else if (c1 == 'A') {
      if (strchr("W123456789+", c2)) {
        shortResponse = true;
      }
    } else if (c1 == 'F' || c1 == 'f') {
      if (strchr("123456", c2) && c3 != '#') {
        if (strchr("+-QZHhF1234", c3)) noResponse = true;
        if (strchr("Aapc", c3)) shortResponse = true;
      } else {
        if (strchr("+-QZHhF1234", c2)) noResponse = true;
        if (strchr("Aapc", c2)) shortResponse = true;
      }
    } else if (c1 == 'r') {
      if (strchr("+-PRFC<>Q1234", c2)) noResponse = true;
      if (strchr("~S", c2)) shortResponse = true;
    } else if (c1 == 'R') {
      if (strchr("AEGCMS0123456789", c2)) noResponse = true;
    } else if (c1 == 'S') {
      if (strchr("CLSGtgMNOPrdhoTBX", c2)) shortResponse = true;
    } else if (c1 == 'L') {
      if (strchr("BNCDL!", c2)) noResponse = true;
      if (strchr("o$W", c2)) {
        shortResponse = true;
      }
    } else if (c1 == 'B') {
      if (strchr("+-", c2)) noResponse = true;
    } else if (c1 == 'C') {
      if (strchr("S", c2)) noResponse = true;
    } else if (c1 == 'h') {
      if (strchr("FC", c2)) {
        noResponse = true;
      }
      if (strchr("QPR", c2)) {
        shortResponse = true;
      }
    } else if (c1 == 'T') {
      if (strchr("QR+-SLK", c2)) noResponse = true;
      if (strchr("edrn", c2)) shortResponse = true;
    } else if (c1 == 'U') {
      noResponse = true;
    } else if (c1 == 'W') {
      if (c2 == 'R') {
        if (strchr("+-", c3)) shortResponse = true;
        else noResponse = true;
      }
      if (c2 == 'S') shortResponse = true;
      if (strchr("0123", c2)) noResponse = true;
    } else if (c1 == '$' && c2 == 'Q' && c3 == 'Z') {
      if (strchr("+-Z/!", c4)) noResponse = true;
    }

    // Checksum override
    if (cmd[0] == ';') {
      noResponse = false;
      shortResponse = false;
    }
  }

  bool supressFrame = false;
  bool numericReply = true;

  char mutableCommand[64];
  strncpy(mutableCommand, cmd, sizeof(mutableCommand) - 1);
  mutableCommand[sizeof(mutableCommand) - 1] = '\0';  // Null-terminate

  char extractedCmd[3] = {0};
  char parameter[32] = {0};

  // Extract command
  extractedCmd[0] = mutableCommand[1];
  if (mutableCommand[2] == '#') {
    extractedCmd[1] = '\0';  // Single-char command, no parameter
  } else {
    extractedCmd[1] = mutableCommand[2];
    extractedCmd[2] = '\0';

    if (mutableCommand[3] == '#') {
      // Two-char command, no parameter
      parameter[0] = '\0';
    } else {
      // Parameter begins at index 3
      size_t paramIndex = 0;
      size_t i = 3;
      while (mutableCommand[i] != '#' && mutableCommand[i] != '\0' && paramIndex < sizeof(parameter) - 1) {
        parameter[paramIndex++] = mutableCommand[i++];
      }
      parameter[paramIndex] = '\0';  // Null-terminate
    }
  }

  CommandError telescopeError = CommandError::CE_NONE;
  telescope.command(response, extractedCmd, parameter, &supressFrame, &numericReply, &telescopeError);
  cmdError = static_cast<CmdError>(telescopeError);

  if (numericReply) {
    if (cmdError != CD_NONE && cmdError != CD_1) strcpy(response,"0"); else strcpy(response,"1");
    supressFrame = true;
  } 

  if (!supressFrame) {
    strcat(response,"#");
  }
  
  VF("MSG: cmdDirect="); V(extractedCmd); V(parameter); VF(", reply = "); V(response);

  if (cmdError > CD_0) {
    if (strstr(extractedCmd, "GX") && strstr(parameter, "89")) {
      cmdError = CD_DATE_TIME_NOT_READY;
    }
    lastCmdError = cmdError;
    V(", "); VL(cmdErrorStr[lastCmdError]);
  } else {
    VL(", None");
  }

  if (noResponse) {
    response[0] = 0;
    return true;
  } else if (shortResponse) {
    return (response[0] != 0);
  } else { // get full response, '#' terminated
    return response[strlen(response) - 1] == '#';
  }
}

bool CmdDirect::commandBool(const char *command) {
  char response[80] = "";
  bool success = processCommand(command, response);
  int l = strlen(response) - 1; if (l >= 0 && response[l] == '#') response[l] = 0;
  if (!success) return false;
  if (response[1] != 0) return false;
  if (response[0] == '0') return false; else return true;
}

bool CmdDirect::commandWithReply(const char *command, char *response) {
  bool success = processCommand(command, response);
  int l = strlen(response) - 1;
  if (l >= 0 && response[l] == '#') response[l] = 0;
  return success;
}

bool CmdDirect::commandBlind(const char *command) {
  char response[80] = "";
  return processCommand(command, response);
}
  
CmdDirect cmdDirect;