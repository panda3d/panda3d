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
  init_libdevice();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libdevice
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdevice() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  MouseAndKeyboard::init_type();
  TrackerNode::init_type();
  ADInputNode::init_type();
  ClientBase::init_type();
}
