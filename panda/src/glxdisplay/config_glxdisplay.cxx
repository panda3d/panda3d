// Filename: config_glxdisplay.cxx
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

#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"
#include "glxGraphicsWindow.h"
#include "glxDisplay.h"

#include <dconfig.h>

Configure(config_glxdisplay);
NotifyCategoryDef(glxdisplay, "display");

ConfigureFn(config_glxdisplay) {
  glxGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(glxGraphicsPipe::get_class_type(),
                                          glxGraphicsPipe::make_glxGraphicsPipe);
  glxGraphicsWindow::init_type();
  GraphicsWindow::get_factory().register_factory(glxGraphicsWindow::get_class_type(),
                                            glxGraphicsWindow::make_GlxGraphicsWindow);
  glxDisplay::init_type();
}

bool gl_show_fps_meter = config_glxdisplay.GetBool("show-fps-meter", false);
float gl_fps_meter_update_interval = max((float)0.5,config_glxdisplay.GetFloat("fps-meter-update-interval", 1.7));
