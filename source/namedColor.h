#pragma once
#include "color.h"

class NamedColor: public Color
{
public:
    NamedColor():m_name(""){};
    NamedColor(Color col, String name):Color(col),m_name(name){};
    String m_name;
};
