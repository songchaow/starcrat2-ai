#pragma once
/*  Communicate.h 
    Defines all communication data structures sent from and to Python.
    Includes "Commands" and "Messages".
    Commands are sent to C++-end. They are categorized as `BuildCommand` and `CombatCommand`
    Messages are sent back to Python-end. They are categorized as ...
 */

#include "Region.h"
#include "sc2api/sc2_typeenums.h"

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

struct ProductionCommand : Command
{
    enum ProductType {Unit, Upgrade};
    virtual const ProductType getProductType() const = 0;
};

struct CreateCommand : ProductionCommand
{
    virtual const ProductType getProductType() const override {return ProductType::Unit;}
    CreateCommand(sc2::UNIT_TYPEID unit_type) : unit_type(unit_type) {}
    sc2::UNIT_TYPEID unit_type;
};

struct UpgradeCommand : ProductionCommand
{
    virtual const ProductType getProductType() const override {return ProductType::Upgrade;}
    UpgradeCommand(sc2::UPGRADE_ID upgrade_type) : upgrade_type(upgrade_type) {}
    sc2::UPGRADE_ID upgrade_type;
};
