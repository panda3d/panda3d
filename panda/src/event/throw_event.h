/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file throw_event.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef THROW_EVENT_H
#define THROW_EVENT_H

#include "pandabase.h"

#include "eventQueue.h"
#include "pt_Event.h"
#include "eventParameter.h"

// A handful of convenience functions to throw events.
INLINE void throw_event(const CPT_Event &event);
INLINE void throw_event(const std::string &event_name);
INLINE void throw_event(const std::string &event_name,
                        const EventParameter &p1);
INLINE void throw_event(const std::string &event_name,
                        const EventParameter &p1,
                        const EventParameter &p2);
INLINE void throw_event(const std::string &event_name,
                        const EventParameter &p1,
                        const EventParameter &p2,
                        const EventParameter &p3);
INLINE void throw_event(const std::string &event_name,
                        const EventParameter &p1,
                        const EventParameter &p2,
                        const EventParameter &p3,
                        const EventParameter &p4);

#include "eventHandler.h"

INLINE void throw_event_directly(EventHandler& handler,
                                 const CPT_Event &event);
INLINE void throw_event_directly(EventHandler& handler,
                                 const std::string &event_name);
INLINE void throw_event_directly(EventHandler& handler,
                                 const std::string &event_name,
                                 const EventParameter &p1);
INLINE void throw_event_directly(EventHandler& handler,
                                 const std::string &event_name,
                                 const EventParameter &p1,
                                 const EventParameter &p2);
INLINE void throw_event_directly(EventHandler& handler,
                                 const std::string &event_name,
                                 const EventParameter &p1,
                                 const EventParameter &p2,
                                 const EventParameter &p3);

#include "throw_event.I"

#endif
