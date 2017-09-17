/*
 * conditions.cpp
 *
 *  Created on: Jan 15, 2017
 *      Author: nullifiedcat
 */

#include "conditions.h"
#include "common.h"

static CatCommand dump_conditions("debug_conditions", "Shows conditions for entity #", [](const CCommand& args) {
	int id = atoi(args.Arg(1));
	if (!id) id = LOCAL_E->m_IDX;
	CachedEntity* ent = ENTITY(id);
	if (CE_BAD(ent)) return;
	condition_data_s& old_data = CE_VAR(ent, netvar.iCond, condition_data_s);
	logging::Info(format("old conds: ", std::bitset<32>(old_data.cond_0), ' ', std::bitset<32>(old_data.cond_1), ' ', std::bitset<32>(old_data.cond_2), ' ', std::bitset<32>(old_data.cond_3)).c_str());
	condition_data_s& new_data = CE_VAR(ent, netvar._condition_bits, condition_data_s);
	logging::Info(format("new conds: ", std::bitset<32>(new_data.cond_0), ' ', std::bitset<32>(new_data.cond_1), ' ', std::bitset<32>(new_data.cond_2), ' ', std::bitset<32>(new_data.cond_3)).c_str());
});
