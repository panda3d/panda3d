// Filename: config_glutdisplay.cxx
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

#include "config_glutdisplay.h"
#include "glutGraphicsPipe.h"
#include "glutGraphicsWindow.h"

#include <dconfig.h>

Configure(config_glutdisplay);
NotifyCategoryDef(glutdisplay, "display");

ConfigureFn(config_glutdisplay) {
  glutGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(glutGraphicsPipe::get_class_type(),
                                          glutGraphicsPipe::make_glutGraphicsPipe);
  glutGraphicsWindow::init_type();
  GraphicsWindow::get_factory().register_factory(glutGraphicsWindow::get_class_type(),
                                            glutGraphicsWindow::make_GlutGraphicsWindow);
}
