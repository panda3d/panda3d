// Filename: config_ribgsg.cxx
// Created by:  cary (08Oct99)
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

#include "config_ribgsg.h"
#include "ribGraphicsStateGuardian.h"

#include "dconfig.h"

Configure(config_ribgsg);
NotifyCategoryDef(ribgsg, ":display:gsg");

ConfigureFn(config_ribgsg) {
  RIBGraphicsStateGuardian::init_type();
  GraphicsStateGuardian::get_factory().
    register_factory(RIBGraphicsStateGuardian::get_class_type(),
                     RIBGraphicsStateGuardian::make_RIBGraphicsStateGuardian);
}
