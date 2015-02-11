// Filename: config_event.cxx
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

#include "config_event.h"
#include "asyncTask.h"
#include "asyncTaskChain.h"
#include "asyncTaskManager.h"
#include "asyncTaskPause.h"
#include "asyncTaskSequence.h"
#include "buttonEventList.h"
#include "event.h"
#include "eventHandler.h"
#include "eventParameter.h"
#include "genericAsyncTask.h"
#include "pointerEventList.h"

#include "dconfig.h"

Configure(config_event);
NotifyCategoryDef(event, "");
NotifyCategoryDef(task, "");

ConfigureFn(config_event) {
  AsyncTask::init_type();
  AsyncTaskChain::init_type();
  AsyncTaskManager::init_type();
  AsyncTaskPause::init_type();
  AsyncTaskSequence::init_type();
  ButtonEventList::init_type();
  PointerEventList::init_type();
  Event::init_type();
  EventHandler::init_type();
  EventStoreInt::init_type("EventStoreInt");
  EventStoreDouble::init_type("EventStoreDouble");
  GenericAsyncTask::init_type();

  ButtonEventList::register_with_read_factory();
  EventStoreInt::register_with_read_factory();
  EventStoreDouble::register_with_read_factory();
}
