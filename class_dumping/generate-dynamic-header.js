const fs = require("fs");

var fullClassTable = {};
try {
	fullClassTable = JSON.parse(fs.readFileSync("full-class-table.json").toString());
} catch (e) {}

console.log("Generating dummy class header");

var header = `/*
	AUTO-GENERATED HEADER - DO NOT MODIFY
	HEADER FOR UNIVERSAL BUILD
*/

#ifndef DYNAMIC_AUTOGEN_HPP
#define DYNAMIC_AUTOGEN_HPP

namespace client_classes {
	
	class dynamic {
	public:
		dynamic();
		void Populate();
`;

for (var clz in fullClassTable) {
	header += "\t\tint " + clz + " { 0 };\n";
}

header += `
	};
	
	extern dynamic dynamic_list;
}

#endif /* DYNAMIC_AUTOGEN_HPP */`;

var POPULATED_MAP = "";

for (var clz in fullClassTable) {
	POPULATED_MAP += `\t\tclassid_mapping["${clz}"] = &${clz};\n`;
}

var source = `

#include "classinfo/dynamic.gen.hpp"
#include "common.hpp"

namespace client_classes {
	
std::unordered_map<std::string, int*> classid_mapping {};
	
dynamic::dynamic() {
${POPULATED_MAP}
}

void dynamic::Populate() {
	ClientClass* cc = g_IBaseClient->GetAllClasses();
	while (cc) {
		std::string name(cc->GetName());
		if (classid_mapping.find(name) != classid_mapping.end())
			*classid_mapping[name] = cc->m_ClassID;
		cc = cc->m_pNext;
	}
}
	
dynamic dynamic_list;

}`;

console.log(header)
console.log(source)
fs.writeFileSync("include/classinfo/dynamic.gen.hpp", header);
fs.writeFileSync("src/classinfo/dynamic.gen.cpp", source);
