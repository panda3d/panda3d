// Filename: pt_Event.h
// Created by:  drose (26May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PT_EVENT_H
#define PT_EVENT_H

#include <pandabase.h>

#include "event.h"

#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : PT_Event
// Description : A PointerTo<Event>.  This is defined here solely we
//               can explicitly export the template class.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerToBase<Event>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerTo<Event>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, ConstPointerTo<Event>)

typedef PointerTo<Event> PT_Event;
typedef ConstPointerTo<Event> CPT_Event;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
