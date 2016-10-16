#include "RGBLed.hpp"

RGBLed::RGBLed(PinName rPin, PinName gPin, PinName bPin) :
    r(rPin),
    g(gPin),
    b(bPin)
{
    set(false, false, false);
}

bool RGBLed::getRed()
{
    return !r.read();
}

bool RGBLed::getGreen()
{
    return !g.read();
}

bool RGBLed::getBlue()
{
    return !b.read();
}

RGBLed::Color RGBLed::get()
{
    return Color(getRed() | (getGreen() << 1) | (getBlue() << 2));
}

RGBLed& RGBLed::setRed(bool value)
{
    r = !value;
    return *this;
}

RGBLed& RGBLed::setGreen(bool value)
{
    g = !value;
    return *this;
}

RGBLed& RGBLed::setBlue(bool value)
{
    b = !value;
    return *this;
}

RGBLed& RGBLed::set(bool rValue, bool gValue, bool bValue)
{
    return setRed(rValue).setGreen(gValue).setBlue(bValue);
}

RGBLed& RGBLed::set(RGBLed::Color color)
{
    return set(color & 1, color & 2, color & 4);
}
