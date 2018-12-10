#pragma once

#include "Common.h"
#include "Unit.h"

class CCBot;

/*
一，依赖单位：
1.
依赖单位已建造：
m_hasUnit = true,
m_units_required为空

2.
依赖单位正在建造：
m_hasUnit = false,
m_units_required为空

3.
依赖单位尚未建造：
m_hasUnit = false,
m_units_required中保存依赖单位

二，生产者：
1.
生产者已建造且不忙：
m_hasProducer = true;
m_busy = false;
m_producers_required为空

2.
生产者已建造但忙：
m_hasProducer = true;
m_busy = true;
m_producers_required为空

3.
生产者正在建造：
m_hasProducer = false;
m_busy = false;
m_producers_required为空

4.
生产者尚未建造：
m_hasProducer = false;
m_busy = false;
m_producers_required中保存生产单位

三，资源：
1.
资源充足（考虑矿物，天然气，供给）：
m_hasEnoughResource = true;

2.
资源不充足（一项或几项资源匮乏）：
m_hasEnoughResource = false;

四，依赖升级（目前还没实现）：
1.
依赖升级已建造：
m_hasTech = true,
m_tech_required为空

2.
依赖升级正在建造：
m_hasTech = false,
m_tech_required为空

3.
依赖升级尚未建造：
m_hasTech = false,
m_tech_required中保存依赖单位

五，结果：
当且仅当!busy && m_hasProducer && m_hasUnit && m_hasTech && m_hasEnoughResource时：
m_result = true;
否则：
m_result = false;
*/

class TryCreateResults {
    CCBot &m_bot;
	bool m_result; 
	bool m_busy;
	bool m_hasUnit;
	std::vector<MetaType> m_units_required;
	bool m_hasTech;
	std::vector<MetaType> m_tech_required;
	std::vector<MetaType> m_tech_required_researching;
	bool m_hasProducer;
	std::vector<MetaType> m_producers_required;
	bool m_hasEnergy;
	bool m_hasValidPlace;
	bool m_hasMineral;
	bool m_hasGas;
	bool m_hasSupply;
public:
	TryCreateResults(CCBot &m_bot, bool result, bool busy, bool hasUnit, bool hasTech, bool hasProducer, bool hasMineral, bool hasGas, bool hasSupply, bool hasValidPlace, bool hasEnergy);

    void addUnitRequired(const UnitType & unit_type);
    void addTechRequired(const CCUpgrade & unit_type);
	void addTechRequiredResearching(const CCUpgrade & unit_type);
    void addProducerRequired(const UnitType & unit_type);

    bool getResult();
    bool IsBusy();
    bool hasUnitRequired();
    bool hasTechRequired();
    bool hasProducerRequired();  
    bool hasEnoughMineral();
	bool hasEnoughGas();
	bool hasEnoughSupply();
	bool hasValidPlace();
	bool hasEnergy();

    std::vector<MetaType> getUnitsRequired();
    std::vector<MetaType> getTechRequired();
	std::vector<MetaType> getTechRequiredResearching();
    std::vector<MetaType> getProducersRequired();
};