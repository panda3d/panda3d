// Filename: config_device.cxx
// Created by:  drose (04May00)
// 
////////////////////////////////////////////////////////////////////

#include "config_device.h"
#include "mouse.h"
#include "clientBase.h"
#include "trackerNode.h"
#include "adinputNode.h"

#include <dconfig.h>

Configure(config_device);
NotifyCategoryDef(device, "");

const bool asynchronous_clients = config_device.GetBool("asynchronous-clients", false);

ConfigureFn(config_device) {
  MouseAndKeyboard::init_type();
  TrackerNode::init_type();
  ADInputNode::init_type();
  ClientBase::init_type();
}
