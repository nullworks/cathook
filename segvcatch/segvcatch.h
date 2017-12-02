/***************************************************************************
 *   Copyright (C) 2009 by VisualData                                      *
 *                                                                         *
 *   Redistributed under LGPL license terms.                               *
 ***************************************************************************/

#ifndef _SEGVCATCH_H
#define	_SEGVCATCH_H

/*! \brief segvcatch namespace


*/
namespace segvcatch
{


/*! Signal handler, used to redefine standart exception throwing. */
typedef void (*handler)();


extern segvcatch::handler handler_segv;
extern segvcatch::handler handler_fpe;

/*! Initialize segmentation violation handler.
    \param h (optional) - optional user's signal handler. By default used an internal signal handler to throw
 std::runtime_error.
   */
void init_segv(handler h = 0);

/*! Initialize floating point error handler.
    \param h - optional user's signal handler. By default used an internal signal handler to throw
 std::runtime_error.*/
void init_fpe(handler h = 0);

}

#endif	/* _SEGVCATCH_H */
