#ifndef KSULED_h
#define KSULED_h
#include <iostream>
#include <vector>
#include <utility>
#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
using namespace std;

#define NUMPIXELS 2
#define PIN 2



class LED
{
public:
    LED();
    void setColor(byte r, byte g, byte b,byte select);
    //void setColor();
    void clear();
   // void error();
   // void errorTrap(int code);
    void blink(byte r, byte g, byte b, byte select);
    void startUp(byte brightness);
private:


void tick();








/*  byte _r;
    byte _b;
    byte _g;

    void set_blink_sequence(vector<pair<int, RGB>> sequence);
    bool _blinking;
    vector<pair<int, RGB>> _blink_sequence;
    unsigned int _blink_index;
    int _next_tick;
    */
};

#endif
