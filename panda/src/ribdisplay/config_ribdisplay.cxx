// Filename: config_ribdisplay.cxx
// Created by:  cary (07Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "config_ribdisplay.h"
#include "ribGraphicsPipe.h"
#include "ribGraphicsWindow.h"

#include "dconfig.h"

Configure(config_ribdisplay);
NotifyCategoryDef(ribdisplay, "display");

ConfigureFn(config_ribdisplay) {
  RIBGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(RIBGraphicsPipe::get_class_type(),
                                          RIBGraphicsPipe::make_RIBGraphicsPipe);
  RIBGraphicsWindow::init_type();
  GraphicsWindow::get_factory().register_factory(RIBGraphicsWindow::get_class_type(),
                                             RIBGraphicsWindow::make_RibGraphicsWindow);
}
