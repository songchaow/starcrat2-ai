#include "sc2api/sc2_unit.h"
#include "MicroMachine.h"
#include "boost/python.hpp"
#include "boost/python/class.hpp"
#include <boost/python/enum.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>

#define ADD_ENUM_ENTRY(classname,member) .value(#member,classname::member)
#define ADD_READWRITE_ENTRY(classname,member) .def_readwrite(#member,&classname::member)
#define ADD_METHOD(classname,member) .def(#member,&classname::member)
using namespace boost::python;

class sc2::Unit;


BOOST_PYTHON_MODULE(micromachine)
{
	{
		class_<sc2::Unit> unit("Unit");
		scope in_unit = unit;
		enum_<sc2::Unit::DisplayType>("DisplayType")
			ADD_ENUM_ENTRY(sc2::Unit,Visible)
			ADD_ENUM_ENTRY(sc2::Unit,Snapshot)
			ADD_ENUM_ENTRY(sc2::Unit,Hidden)
			;
		enum_<sc2::Unit::Alliance>("Alliance")
			ADD_ENUM_ENTRY(sc2::Unit,Self)
			ADD_ENUM_ENTRY(sc2::Unit, Ally)
			ADD_ENUM_ENTRY(sc2::Unit, Neutral)
			ADD_ENUM_ENTRY(sc2::Unit, Enemy)
			;
		enum_<sc2::Unit::CloakState>("CloakState")
			ADD_ENUM_ENTRY(sc2::Unit, Cloaked)
			ADD_ENUM_ENTRY(sc2::Unit, CloakedDetected)
			ADD_ENUM_ENTRY(sc2::Unit, NotCloaked)
			ADD_ENUM_ENTRY(sc2::Unit, Unknown)
			;
		unit
			ADD_READWRITE_ENTRY(sc2::Unit, display_type)
			ADD_READWRITE_ENTRY(sc2::Unit, alliance)
			ADD_READWRITE_ENTRY(sc2::Unit, tag)
			//ADD_READWRITE_ENTRY(sc2::Unit, unit_type)
			;
	}
	class_<MicroMachine> mm_class("MicroMachine");
	mm_class
            ADD_METHOD(MicroMachine,Initialize)
            ADD_METHOD(MicroMachine,Update)
	    ;

};
