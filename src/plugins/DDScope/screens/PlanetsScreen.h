// =====================================================
// PlanetsScreen.h

#ifndef PLANETS_S_H
#define PLANETS_S_H

#include <Arduino.h>
#include <Ephemeris.h>

class Display;

class PlanetsScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updatePlanetsStatus();
    void updatePlanetsButtons();
    bool planetsButStateChange();

  private:
    uint8_t mapPlanetIndex(uint8_t planetIndex);
    void char2RA(char* txt, unsigned int& hour, unsigned int& minute, unsigned int& second);
    void GetTime(unsigned int &hour, unsigned int &minute, unsigned int &second, bool ut);
    void GetDate(unsigned int &day, unsigned int &month, unsigned int &year, bool ut);
    void GetLatitude(int &degree, int &minute, int &second);
    void GetLongitude(int &degree, int &minute, int &second);
    void equatorialCoordinatesToString(EquatorialCoordinates coord, char raCoord[14] , char decCoord[14]);
    void getPlanet(unsigned short planetNum);

    // SolarSystemObjectIndex from Ephemeris.hpp
    //Sun        = 0,
    //Mercury    = 1,
    //Venus      = 2,
    //Earth      = 3,
    //Mars       = 4,
    //Jupiter    = 5,
    //Saturn     = 6,
    //Uranus     = 7,
    //Neptune    = 8,
    //EarthsMoon = 9
    char planetSelectionStr[9];
    bool planetButDetected = false;
    int planetButSelPos = 4; // default to Mars
    int planetPrevSel;
    int utc;
};

extern PlanetsScreen planetsScreen;

#endif