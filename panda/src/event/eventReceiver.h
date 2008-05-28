// Filename: eventReceiver.h
// Created by:  drose (14Dec99)
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

#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include "pandabase.h"
#include "typedObject.h"

////////////////////////////////////////////////////////////////////
//       Class : EventReceiver
// Description : An abstract base class for anything that might care
//               about receiving events.  An object that might receive
//               an event should inherit from this class; each event
//               may be sent with an optional EventReceiver pointer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_EVENT EventReceiver {

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


