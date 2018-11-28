#pragma once
/*  Communicate.h 
    Defines all communication data structures sent from and to Python.
    Includes "Commands" and "Messages".
    Commands are categorized as `BuildCommand` and `CombatCommand`
    Messages are categorized as ...
 */

#include "Region.h"

struct Command {};
struct CombatCommand : Command
{
    enum CombatType {RegionMove};
    virtual const CombatType getCombatType() const = 0;
};

struct RegionMoveCommand : CombatCommand
{
    virtual const CombatType getCombatType() const override {return CombatType::RegionMove;}
    RegionMoveCommand(RegionID source, RegionID target) : source(source), target(target) {}
    RegionID source;
    RegionID target;
};
