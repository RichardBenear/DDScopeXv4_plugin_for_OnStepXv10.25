// cmdDirect.h
#pragma once

class CmdDirect {
    public:
        CmdDirect();
        
        bool commandBlind(const char *command);
        bool commandBool(const char *command);
        bool commandWithReply(const char *command, char *response);
        bool processCommand(const char *cmd, char *response);
        char lastCmd[8];
        const char *getLastCommandErrorString();
        int getLastCmdError() const;

    private:
    
};

extern CmdDirect cmdDirect;

inline bool commandBool(const char *command) {
    return cmdDirect.commandBool(command);
}

inline bool commandWithReply(const char *command, char *response) {
    return cmdDirect.commandWithReply(command, response);
}

inline bool commandBlind(const char *command) {
    return cmdDirect.commandBlind(command);
}
