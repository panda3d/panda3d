/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_event.cxx
 * @author drose
 * @date 1999-12-14
 */

#include "config_event.h"
#include "asyncFuture.h"
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

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_EVENT)
  #error Buildsystem error: BUILDING_PANDA_EVENT not defined
#endif

Configure(config_event);
NotifyCategoryDef(event, "");
NotifyCategoryDef(task, "");

ConfigureFn(config_event) {
  AsyncFuture::init_type();
  AsyncGatheringFuture::init_type();
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
