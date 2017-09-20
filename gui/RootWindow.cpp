/*
 * RootWindow.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "RootWindow.h"
#include "CTextLabel.h"
#include "CCheckbox.h"
#include "CBaseButton.h"
#include "CTextInput.h"
#include "CSplitContainer.h"
#include "CSlider.h"
#include "CTooltip.h"
#include "CBaseContainer.h"
#include "CDropdown.h"
#include "CMenuContainer.h"
#include "CCVarContainer.h"
#include "CMenuWindow.h"

#include "../common.h"
#include "CCTitleBar.h"

void B1Callback(CBaseButton* thisptr) {
	CBaseContainer* container = dynamic_cast<CBaseContainer*>(thisptr->GetParent());
	CTextLabel* label = dynamic_cast<CTextLabel*>(container->ChildByName("td"));
	if (label) {
		char* text = strfmt("wow! this[\"%s\"] = %i", "test_value", thisptr->Props()->GetInt("test_value"));
		label->SetText(text);
		delete [] text;
	}
}

void TICallback(CTextInput* thisptr, std::string olds, std::string news) {
	CBaseContainer* container = dynamic_cast<CBaseContainer*>(thisptr->GetParent());
	CTextLabel* tl = dynamic_cast<CTextLabel*>(container->ChildByName("td"));
	if (tl) {
		char* text = strfmt("wow! text: %s", news.c_str());
		tl->SetText(text);
		delete [] text;
	}
}

RootWindow::RootWindow() : CBaseWindow("root") {}

void RootWindow::Setup() {
	g_pGUI->m_pTooltip = new CTooltip();
	AddChild(g_pGUI->m_pTooltip);
	SetMaxSize(draw::width, draw::height);
	CBaseWindow* ws = new CBaseWindow("splitwindow");
	ws->SetPositionMode(ABSOLUTE);
	CTitleBar* wst = new CTitleBar(ws, "Window Layout Test");
	ws->AddChild(wst);
	ws->SetMaxSize(500, 0);
	CMenuWindow* win = new CMenuWindow("menu_window", this);
	win->SetMaxSize(912, 410);
	auto ms = GetMaxSize();
	win->AddElements();
	win->SetOffset((draw::width - 912) / 2, (draw::height - 410) / 2);
	AddChild(win);
}
