// Filename: config_helix.h
// Created by:  jjtaylor (27Feb04)
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

#include "config_helix.h"
#include "helixClient.h"
#include "pandaSystem.h"

#include "dconfig.h"

DLLAccessPath statClnt;
DLLAccessPath* GetDLLAccessPath()
{
    return &statClnt;
}

Configure(config_helix);
NotifyCategoryDef(helix, "");

ConfigureFn(config_helix) {
  HelixClient::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("Helix");
}

