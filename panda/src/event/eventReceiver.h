// Filename: eventReceiver.h
// Created by:  drose (14Dec99)
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

#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include "pandabase.h"

#include <typedObject.h>

////////////////////////////////////////////////////////////////////
//       Class : EventReceiver
// Description : An abstract base class for anything that might care
//               about receiving events.  An object that might receive
//               an event should inherit from this class; each event
//               may be sent with an optional EventReceiver pointer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS EventReceiver {

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "EventReceiver");
  }

private:
  static TypeHandle _type_handle;
};

#endif


