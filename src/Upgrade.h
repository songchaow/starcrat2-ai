#pragma once

#include "Common.h"

class CCBot;

class Upgrade{
    CCBot  &m_bot;
    CCUpgrade m_upgradeID;
    int m_startLoop;
    int m_finishLoop;

public:
    Upgrade(CCBot & bot, CCUpgrade upgradeID);

	int getUpgradeID();
	int getStartTime();
    int getFinishTime();
    void setStartTime(int startTime);
	void setFinishTime(int finishTime);
};