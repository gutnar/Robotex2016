#ifndef RGBLED_H
#define RGBLED_H

#include <mbed.h>

class RGBLed
{
public:
    enum Color
    {
        Black = 0,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White
    };

    RGBLed(PinName rPin, PinName gPin, PinName bPin);

    bool getRed();
    bool getGreen();
    bool getBlue();
    Color get();

    RGBLed& setRed(bool value);
    RGBLed& setGreen(bool value);
    RGBLed& setBlue(bool value);
    RGBLed& set(bool rValue, bool gValue, bool bValue);
    RGBLed& set(Color color);

private:
    DigitalOut r, g, b;
};

#endif // RGBLED_H
