// Filename: config_vrpn.cxx
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "config_vrpn.h"
#include "vrpnClient.h"

#include <dconfig.h>

Configure(config_vrpn);
NotifyCategoryDef(vrpn, "");


ConfigureFn(config_vrpn) {
  VrpnClient::init_type();
}
