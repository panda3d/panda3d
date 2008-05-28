// Filename: pt_Event.h
// Created by:  drose (26May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PT_EVENT_H
#define PT_EVENT_H

#include "pandabase.h"

#include "event.h"

#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PT_Event
// Description : A PointerTo<Event>.  This is defined here solely we
//               can explicitly export the template class.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT, PointerToBase<Event>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT, PointerTo<Event>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT, ConstPointerTo<Event>)

typedef PointerTo<Event> PT_Event;
typedef ConstPointerTo<Event> CPT_Event;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
