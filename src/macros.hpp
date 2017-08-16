/*
 * macros.hpp
 *
 *  Created on: May 25, 2017
 *      Author: nullifiedcat
 */

#ifndef MACROS_HPP_
#define MACROS_HPP_

#if defined(NOGUI) and NOGUI == 1 or defined(TEXTMODE)
#define ENABLE_GUI false
#else
#define ENABLE_GUI true
#endif

#endif /* MACROS_HPP_ */
