// Filename: throw_event.h
// Created by:  drose (19Feb99)
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

#ifndef THROW_EVENT_H
#define THROW_EVENT_H

#include "pandabase.h"

#include "eventQueue.h"
#include "pt_Event.h"

// A handful of convenience functions to throw events.
INLINE void throw_event(const CPT_Event &event);
INLINE void throw_event(const string &event_name);
INLINE void throw_event(const string &event_name,
                        const EventParameter &p1);
INLINE void throw_event(const string &event_name,
                        const EventParameter &p1,
                        const EventParameter &p2);
INLINE void throw_event(const string &event_name,
                        const EventParameter &p1,
                        const EventParameter &p2,
                        const EventParameter &p3);

#include "eventHandler.h"

INLINE void throw_event_directly(EventHandler& handler,
                                 const CPT_Event &event);
INLINE void throw_event_directly(EventHandler& handler,
                                 const string &event_name);
INLINE void throw_event_directly(EventHandler& handler,
                                 const string &event_name,
                                 const EventParameter &p1);
INLINE void throw_event_directly(EventHandler& handler,
                                 const string &event_name,
                                 const EventParameter &p1,
                                 const EventParameter &p2);
INLINE void throw_event_directly(EventHandler& handler,
                                 const string &event_name,
                                 const EventParameter &p1,
                                 const EventParameter &p2,
                                 const EventParameter &p3);

#include "throw_event.I"

#endif
