// Filename: config_event.cxx
// Created by:  drose (14Dec99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "config_event.h"
#include "buttonEventList.h"
#include "event.h"
#include "eventHandler.h"
#include "eventParameter.h"

#include "dconfig.h"

Configure(config_event);
NotifyCategoryDef(event, "");

ConfigureFn(config_event) {
  ButtonEventList::init_type();
  Event::init_type();
  EventHandler::init_type();
  EventStoreValueBase::init_type();
  EventStoreInt::init_type("EventStoreInt");
  EventStoreDouble::init_type("EventStoreDouble");
  EventStoreString::init_type("EventStoreString");

  ButtonEventList::register_with_read_factory();
  EventStoreInt::register_with_read_factory();
  EventStoreDouble::register_with_read_factory();
  EventStoreString::register_with_read_factory();
}

