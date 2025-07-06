
// utils.cpp
// Supporting methods

#include "utils.h"

Utils utils;

void Utils::strncpyex(char *result, const char *source, size_t length) {
  strncpy(result, source, length);
  result[length - 1] = 0;
}

void Utils::formatDegreesStr(char *s)
{
  char *tail;

  // degrees part
  tail = strchr(s, '*');
  if (tail)
  {
    char head[80];
    tail[0] = 0;
    strcpy(head, s);
    strcat(head, "&deg;");

    tail++;
    strcat(head, tail);
    strcpy(s, head);
  } else return;

  // minutes part
  tail = strchr(s, ':');
  if (tail) // indicates there is a seconds part
  {
    tail[0] = '\'';
    strcat(s, "\"");
  }
  else // there is no seconds part
  {
    strcat(s, "\'");
  }
}

void Utils::formatHoursStr(char *s)
{
  char *tail;

  // hours part
  tail = strchr(s, ':');
  if (tail)
  {
    tail[0] = 'h';
  } else return;

  // minutes part
  tail = strchr(s, ':');
  if (tail) // indicates there is a seconds part
  {
    tail[0] = 'm';
    strcat(s, "s");
  }
  else // there is no seconds part
  {
    strcat(s, "m");
  }
}
