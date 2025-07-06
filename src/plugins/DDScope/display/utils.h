#pragma once

#include "display.h"

class Utils {
  public:

    void strncpyex(char *result, const char *source, size_t length);
    void formatDegreesStr(char *s);
    void formatHoursStr(char *s);

    private:
};

extern Utils utils;

