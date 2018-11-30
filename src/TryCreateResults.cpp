#include "TryCreateResults.h"
#include "CCBot.h"

TryCreateResults::TryCreateResults(CCBot &bot, bool result, bool busy, bool hasUnit, bool hasTech, bool hasProducer, bool enoughResource)
    : m_bot(bot)
	, m_result(result)
	, m_busy(busy)
    , m_hasUnit(hasUnit)
    , m_hasTech(hasTech)
    , m_hasProducer(hasProducer)
	, m_hasEnoughResource(enoughResource)
{
}

void TryCreateResults::addUnitRequired(const UnitType & unit_type)
{
    m_units_required.push_back(unit_type);
}

void TryCreateResults::addTechRequired(const UnitType & tech_type)
{
    m_tech_required.push_back(tech_type);
}

void TryCreateResults::addProducerRequired(const UnitType & producer_type)
{
    m_producers_required.push_back(producer_type);
}

bool TryCreateResults::getResult(){
    return m_result;
}

bool TryCreateResults::IsBusy(){
    return m_busy;
}

bool TryCreateResults::hasUnitRequired(){
    return m_hasUnit;
}

bool TryCreateResults::hasTechRequired(){
    return m_hasTech;
}

bool TryCreateResults::hasProducerRequired(){
    return m_hasProducer;
}

bool TryCreateResults::hasEnoughResource(){
    return m_hasEnoughResource;
}

std::vector<UnitType> TryCreateResults::getUnitsRequired(){
    return m_units_required;
}

std::vector<UnitType> TryCreateResults::getTechRequired(){
    return m_tech_required;
}

std::vector<UnitType> TryCreateResults::getProducersRequired(){
    return m_producers_required;
}
