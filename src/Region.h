#pragma once

#include "Common.h"
#include "MeleeManager.h"
#include "RangedManager.h"
#include "SquadOrder.h"
#include <ctime>

class CCBot;

class Region
{
    CCBot &             m_bot;

    std::string         m_name;
    std::vector<Unit>   m_units;
    float               m_width;
    float               m_height;
    float               m_xstart;
    float               m_ystart;


public:

    Region(const std::string & name, const float width, const float height, const float xstart, const float ystart, CCBot & bot);

    //void onFrame();
    void addUnit(const Unit & unit);
    void removeUnit(const Unit & unit);
    float getWidth();
    float getHeight();
    float getXstart();
    float getYstart();
    const std::vector<Unit> & getUnits() const;
	const std::string & getName() const;
	bool containsUnit(const Unit & unit) const;
	void setAllUnits();
};
