// Filename: config_vrpn.cxx
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "config_vrpn.h"
#include "vrpnAnalogDevice.h"
#include "vrpnButtonDevice.h"
#include "vrpnClient.h"
#include "vrpnTrackerDevice.h"

#include <dconfig.h>

Configure(config_vrpn);
NotifyCategoryDef(vrpn, "");


ConfigureFn(config_vrpn) {
  VrpnAnalogDevice::init_type();
  VrpnButtonDevice::init_type();
  VrpnClient::init_type();
  VrpnTrackerDevice::init_type();
}
