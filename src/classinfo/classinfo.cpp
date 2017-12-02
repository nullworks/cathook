/*
 * classinfo.cpp
 *
 *  Created on: May 13, 2017
 *      Author: nullifiedcat
 */

#include "classinfo/classinfo.hpp"

client_classes::dummy* client_class_list = nullptr;

#define INST_CLIENT_CLASS_LIST(x) \
	client_classes::x client_classes::x##_list

INST_CLIENT_CLASS_LIST(tf2);
INST_CLIENT_CLASS_LIST(tf2c);
INST_CLIENT_CLASS_LIST(hl2dm);
INST_CLIENT_CLASS_LIST(css);

void InitClassTable() {
	if (IsTF2()) {
		client_class_list = (client_classes::dummy*)&client_classes::tf2_list;
	}
	if (IsTF2C()) {
		client_class_list = (client_classes::dummy*)&client_classes::tf2c_list;
	}
	if (IsHL2DM()) {
		client_class_list = (client_classes::dummy*)&client_classes::hl2dm_list;
	}
	if (IsCSS()) {
		client_class_list = (client_classes::dummy*)&client_classes::css_list;
	}
	if (IsDynamic()) {
		client_class_list = (client_classes::dummy*)&client_classes::dynamic_list;
	}
	if (!client_class_list) {
		logging::Info("FATAL: Cannot initialize class list! Game will crash if cathook is enabled.");
		//cathook = false;
	}
}
