#include "Region.h"
#include "CCBot.h"
#include "Util.h"

Region::Region(const std::string & name, const float width, const float height, const float xstart, const float ystart, CCBot & bot)
    : m_name(name)
    , m_bot(bot)
    , m_width(width)
    , m_height(height)
    , m_xstart(xstart)
    , m_ystart(ystart)   
{    
}

void Region::setAllUnits()
{
	// clean up the _units vector just in case one of them died
	std::vector<Unit> goodUnits;
	for (auto & unit : m_units)
	{
		if (!unit.isValid()) { continue; }
		if (unit.isBeingConstructed()) { continue; }
		if (unit.getHitPoints() <= 0) { continue; }
		if (!unit.isAlive()) { continue; }
		goodUnits.push_back(unit);
	}

	m_units = goodUnits;
}

bool Region::containsUnit(const Unit & unit) const
{
	return std::find(m_units.begin(), m_units.end(), unit) != m_units.end();
}

const std::vector<Unit> & Region::getUnits() const
{
    return m_units;
}

void Region::addUnit(const Unit & unit)
{
    m_units.push_back(unit);
}

void Region::removeUnit(const Unit & unit)
{
    // this is O(n) but whatever
    for (size_t i = 0; i < m_units.size(); ++i)
    {
        if (unit.getUnitPtr()->tag == m_units[i].getUnitPtr()->tag)
        {
            m_units.erase(m_units.begin() + i);
            break;
        }
    }
}

const std::string & Region::getName() const
{
    return m_name;
}

float Region::getWidth(){
    return m_width;
}

float Region::getHeight(){
    return m_height;
}

float Region::getXstart(){
    return m_xstart;
}

float Region::getYstart(){
    return m_ystart;
}
