#include "Digit.h"

Digit::Digit(int value)
{
    m_value = value;
    m_newValue = value;
    m_frame = 0;
}

int Digit::Value()
{
    return m_value;
}

int Digit::Value(int value)
{
    m_value = value;
    m_newValue = value;
    return m_value;
}

int Digit::NewValue()
{
    return m_newValue;
}

int Digit::NewValue(int newValue)
{
    m_newValue = newValue;
    return m_newValue;
}

int Digit::Frame()
{
    return m_frame;
}

int Digit::Frame(int frame)
{
    m_frame = frame;
    return m_frame;
}

int Digit::Height()
{
    return m_height;
}

int Digit::Height(int height)
{
    m_height = height;
    return m_height;
}

void Digit::SetXY(int x, int y)
{
    m_x = x;
    m_y = y;
}

int Digit::X() { return m_x; };
int Digit::Y() { return m_y; };

Digit::~Digit()
{
}