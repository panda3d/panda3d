// Filename: config_event.cxx
// Created by:  drose (14Dec99)
// 
////////////////////////////////////////////////////////////////////

#include "config_event.h"
#include "event.h"
#include "eventHandler.h"
#include "eventParameter.h"

#include <dconfig.h>

Configure(config_event);
NotifyCategoryDef(event, "");

ConfigureFn(config_event) {
  Event::init_type();
  EventHandler::init_type();
  EventStoreInt::init_type("EventStoreInt");
  EventStoreDouble::init_type("EventStoreDouble");
  EventStoreString::init_type("EventStoreString");
//  EventStoreVec3::init_type("EventStoreVec3");
}

