// Filename: config_sgiglxdisplay.cxx
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

#include "config_sgiglxdisplay.h"
#include "sgiglxGraphicsPipe.h"

#include <dconfig.h>

Configure(config_sgiglxdisplay);
NotifyCategoryDef(sgiglxdisplay, "display");

ConfigureFn(config_sgiglxdisplay) {
  SgiGlxGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(SgiGlxGraphicsPipe::get_class_type(),
                                          SgiGlxGraphicsPipe::make_sgiglxgraphicspipe);
}
