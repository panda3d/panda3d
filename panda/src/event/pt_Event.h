// Filename: pt_Event.h
// Created by:  drose (26May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<Event>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerTo<Event>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerTo<Event>)

typedef PointerTo<Event> PT_Event;
typedef ConstPointerTo<Event> CPT_Event;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
