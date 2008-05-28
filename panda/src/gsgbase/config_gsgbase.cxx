// Filename: config_gsgbase.cxx
// Created by:  drose (06Oct99)
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

#include "config_gsgbase.h"
#include "graphicsStateGuardianBase.h"

#include "dconfig.h"

Configure(config_gsgbase);

ConfigureFn(config_gsgbase) {
  GraphicsStateGuardianBase::init_type();
}
