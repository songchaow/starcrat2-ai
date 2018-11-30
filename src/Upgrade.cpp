#include "Upgrade.h"
#include "CCBot.h"

Upgrade::Upgrade(CCBot &bot,CCUpgrade upgradeID)
   : m_bot(bot)
   , m_upgradeID(upgradeID)
   , m_startLoop(0)
   , m_finishLoop(0)
{
}

int Upgrade::getUpgradeID() {
	return m_upgradeID;
}

int Upgrade::getStartTime() {
	return m_startLoop;
}

int Upgrade::getFinishTime(){
    return m_finishLoop;
}

void Upgrade::setStartTime(int startTime){
    m_startLoop = startTime;
}

void Upgrade::setFinishTime(int finishTime) {
	m_finishLoop = finishTime;
}