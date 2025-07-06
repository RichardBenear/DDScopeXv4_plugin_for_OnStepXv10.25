// =====================================================
// DDScope.h
//
// DDScope (Direct Drive TeleScope) plugin for OnStepX

#pragma once

class DDScope {
  public:
    void init();
    bool command(char *reply, char *command, char *parameter, bool *supressFrame, bool *numericReply, CommandError *commandError);
    
  private:
    
};

extern DDScope dDScope;
