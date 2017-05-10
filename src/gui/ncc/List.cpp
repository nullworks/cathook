/*
 * List.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "List.hpp"
#include "Item.hpp"
#include "ItemTitle.hpp"
#include "Menu.hpp"
#include "ItemVariable.hpp"
#include "ItemSublist.hpp"

#include "../../common.h"

namespace menu { namespace ncc {

List::List(std::string title) : open_sublist(nullptr), title(title), got_mouse(false), CBaseContainer("ncc_list") {
	AddChild(new ItemTitle(title));
	Hide();
	root_list = this;
}

List::List() : open_sublist(nullptr), title(""), got_mouse(false), CBaseContainer("ncc_list") {
	Hide();
	root_list = this;
}

void List::Show() {
	CBaseContainer::Show();
	got_mouse = false;
}

void List::FillWithCatVars(std::vector<CatVar*> vec) {
	for (auto var : vec) {
		AddChild(new ItemVariable(*var));
	}
}

void List::OnKeyPress(ButtonCode_t key, bool repeat) {
	if (GetHoveredChild()) GetHoveredChild()->OnKeyPress(key, repeat);
}

void List::OpenSublist(List* sublist, int dy) {
	if (open_sublist) open_sublist->Hide();
	open_sublist = sublist;
	if (sublist) {
		sublist->SetOffset(Item::size_x + 1, dy);
		sublist->Show();
	}
}

bool List::ShouldClose() {
	if (open_sublist) {
		if (!open_sublist->ShouldClose()) return false;
	}
	return !IsHovered() && !got_mouse;
}

/*IWidget* List::ChildByPoint(int x, int y) {
	IWidget* rr = CBaseContainer::ChildByPoint(x, y);
	if (rr) return rr;
	List* list = dynamic_cast<List*>(open_sublist);
	if (list) {
		auto co = list->GetOffset();
		auto cs = list->GetSize();
		if (x >= co.first && x <= co.first + cs.first &&
			y >= co.second && y <= co.second + cs.second) {
			return list;
		}
	}
	return nullptr;
}*/

CatVar* FindCatVar(const std::string name) {
	for (auto var : CatVarList()) {
		if (var->name == name) return var;
	}
	logging::Info("can't find %s", name.c_str());
	throw std::runtime_error("can't find catvar " + name);
}
// abc def, ghj, [, fdg sgf saqw rter, ], gs
void FillFromTokens(List* list, const std::vector<std::string>& tokens) {
	list->title = tokens[0];
	list->AddChild(new ItemTitle(tokens[0]));
	for (int i = 1; i < tokens.size(); i++) {
		const std::string& str = tokens.at(i);
		if (i == tokens.size() - 1 || tokens[i + 1] != "[") {
			list->AddChild(new ItemVariable(*FindCatVar(str)));
		} else {
			list->AddChild(new ItemSublist(str, List::FromString(tokens[i + 2])));
			i += 3;
		}
	}
}

List* List::FromString(const std::string& string) {
	List* result = new List();
	bool readingkey = false;
	std::string last_read_key = "";
	std::stringstream readkey("");
	std::vector<std::string> tokens = {};
	int brackets = 0;
	for (const auto& c : string) {
		if (c == '[') {
			brackets++;
			if (brackets == 1) {
				tokens.push_back("[");
				readkey.str("");
				readkey.clear();
				continue;
			}
		} else if (c == ']') {
			brackets--;
			if (!brackets) {
				tokens.push_back(readkey.str());
				tokens.push_back("]");
				readkey.str("");
				readkey.clear();
				continue;
			}
		}
		if (!brackets) {
			if (c == '"') {
				readingkey = !readingkey;
				if (!readingkey) {
					tokens.push_back(readkey.str());
					readkey.str("");
					readkey.clear();
				}
			} else {
				if (readingkey) readkey << c;
			}
		} else {
			readkey << c;
		}
	}
	FillFromTokens(result, tokens);
	logging::Info("done making list %s - has %i children.", result->title.c_str(), result->ChildCount());
	return result;
}

void List::FillWithCatVars(std::vector<std::string> vec) {
	for (const auto& string : vec) {
		AddChild(new ItemVariable(*FindCatVar(string)));
	}
}

void List::OnMouseLeave() {
	CBaseContainer::OnMouseLeave();
	if (ShouldClose()) {
		List* parent_list = dynamic_cast<List*>(GetParent());
		if (parent_list) {
			parent_list->OpenSublist(nullptr, 0);
		}
	}
}

void List::Draw(int x, int y) {
	//const auto& size = GetSize();
	draw::OutlineRect(x, y, 2 + Item::size_x, Props()->GetInt("items") * Item::size_y + 2, GUIColor());
	for (int i = 1; i < Props()->GetInt("items"); i++) {
		draw::DrawLine(x + 1, y + Item::size_y * i, Item::size_x, 0, GUIColor());
	}
	//CBaseContainer::Draw(x, y);
	for (int i = 0; i < ChildCount(); i++) {
		Item* item = dynamic_cast<Item*>(ChildByIndex(i));
		if (!item) {
			if (ChildByIndex(i)->GetName().find("ncc_list") == 0) continue;
			throw std::runtime_error("Invalid cast in NCC-List:Draw!");
		}
		const auto& offset = item->GetOffset();
		item->Draw(x + offset.first, y + offset.second);
	}
	if (dynamic_cast<List*>(open_sublist)) {
		const auto& offset = open_sublist->GetOffset();
		open_sublist->Draw(x + offset.first, y + offset.second);
	}
}

void List::OnMouseEnter() {
	CBaseContainer::OnMouseEnter();
	got_mouse = true;
}

void List::SetParent(IWidget* parent) {
	CBaseContainer::SetParent(parent);
	List* parent_list = dynamic_cast<List*>(parent);
	if (parent_list) {
		root_list = parent_list->root_list;
	} else root_list = this;
}

void List::Update() {
	CBaseContainer::Update();
	if (IsPressed() && root_list == this) {
		const auto& offset = root_list->GetOffset();
		root_list->SetOffset(offset.first + g_pGUI->mouse_dx, offset.second + g_pGUI->mouse_dy);
	}
}

void List::MoveChildren() {
	int accy = 2;
	int j = 0;
	for (int i = 0; i < ChildCount(); i++) {
		Item* item = dynamic_cast<Item*>(ChildByIndex(i));
		if (!item) {
			if (ChildByIndex(i)->GetName().find("ncc_list") == 0) continue;
			throw std::runtime_error("Invalid cast in NCC-List:MoveChildren! Offender " + ChildByIndex(i)->GetName());
		}
		item->SetOffset(1, j * Item::size_y + 1);
		accy += Item::size_y;
		j++;
	}
	Props()->SetInt("items", j);
	List* list = dynamic_cast<List*>(open_sublist);
	if (list) {
		const auto& size = list->GetSize();
		const auto& offset = list->GetOffset();
		SetSize(Item::size_x + 2 + size.first, max(accy, offset.second + size.second));
	} else {
		SetSize(Item::size_x + 2, accy);
	}
}

}}
