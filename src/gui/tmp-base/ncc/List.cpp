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

void List::OnKeyPress(int key, bool repeat) {
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

// abc def, ghj, [, fdg sgf saqw rter, ], gs
void FillFromTokens(List* list, CatMenuTree* in) {
	list->title = in->name;
	list->AddChild(new ItemTitle(in->name));
	for (auto& i : in->children) {
		List* tmp = new List();
		FillFromTokens(tmp, &i);
		list->AddChild(new ItemSublist(i.name, tmp));
	}
	for (auto i: in->cat_children) {
		list->AddChild(new ItemVariable(*i));
	}
}

List* List::GetMainList() {
	static List* lmao;
	if (!lmao) {
		lmao = new List();
		FillFromTokens(lmao, &CatMenuRoot);
		//logging::Info("done making list %s - has %i children.", result->title.c_str(), result->ChildCount());
	}
	return lmao;
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
	draw::Rect(x, y, 2 + Item::size_x, Props()->GetInt("items") * Item::size_y + 2, GUIColor());
	for (int i = 1; i < Props()->GetInt("items"); i++) {
		draw::Line(x + 1, y + Item::size_y * i, Item::size_x, 0, GUIColor());
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
		SetSize(Item::size_x + 2 + size.first, std::max(accy, offset.second + size.second));
	} else {
		SetSize(Item::size_x + 2, accy);
	}
}

}}
