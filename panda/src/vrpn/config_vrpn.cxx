// Filename: config_vrpn.cxx
// Created by:  jason (07Aug00)
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

#include "config_vrpn.h"
#include "vrpnAnalogDevice.h"
#include "vrpnButtonDevice.h"
#include "vrpnClient.h"
#include "vrpnDialDevice.h"
#include "vrpnTrackerDevice.h"

#include "dconfig.h"

Configure(config_vrpn);
NotifyCategoryDef(vrpn, "");


ConfigureFn(config_vrpn) {
  VrpnAnalogDevice::init_type();
  VrpnButtonDevice::init_type();
  VrpnClient::init_type();
  VrpnDialDevice::init_type();
  VrpnTrackerDevice::init_type();
}
