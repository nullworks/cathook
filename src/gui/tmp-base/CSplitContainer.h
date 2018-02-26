/*
 * CSplitContainer.h
 *
 *  Created on: Jan 28, 2017
 *      Author: nullifiedcat
 */

#ifndef CSPLITCONTAINER_H_
#define CSPLITCONTAINER_H_

#include "CBaseContainer.h"

class CSplitContainer : public CBaseContainer {
public:
	CSplitContainer(std::string name = "unnamed", IWidget* parent = nullptr);

	virtual void MoveChildren() override;
};



#endif /* CSPLITCONTAINER_H_ */
