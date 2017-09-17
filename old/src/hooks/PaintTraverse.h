/*
 * PaintTraverse.h
 *
 *  Created on: Jan 8, 2017
 *      Author: nullifiedcat
 */

#ifndef PAINTTRAVERSE_H_
#define PAINTTRAVERSE_H_

class CatVar;

extern CatVar no_zoom;
extern CatVar clean_screenshots;
extern CatVar disable_visuals;
void PaintTraverse_hook(void*, unsigned int, bool, bool);

#endif /* PAINTTRAVERSE_H_ */
