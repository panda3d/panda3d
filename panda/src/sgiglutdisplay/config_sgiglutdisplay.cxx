// Filename: config_sgiglutdisplay.cxx
// Created by:  cary (07Oct99)
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

#include "config_sgiglutdisplay.h"
#include "sgiglutGraphicsPipe.h"

#include <dconfig.h>

Configure(config_sgiglutdisplay);
NotifyCategoryDef(sgiglutdisplay, "display");

ConfigureFn(config_sgiglutdisplay) {
  sgiglutGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(sgiglutGraphicsPipe::get_class_type(),
                                          sgiglutGraphicsPipe::make_sgiglutGraphicsPipe);
}
