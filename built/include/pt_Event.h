/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pt_Event.h
 * @author drose
 * @date 2000-05-26
 */

#ifndef PT_EVENT_H
#define PT_EVENT_H

#include "pandabase.h"

#include "event.h"

#include "pointerTo.h"

/**
 * A PointerTo<Event>.  This is defined here solely we can explicitly export
 * the template class.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT, PointerToBase<Event>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT, PointerTo<Event>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT, ConstPointerTo<Event>)

typedef PointerTo<Event> PT_Event;
typedef ConstPointerTo<Event> CPT_Event;

#endif
