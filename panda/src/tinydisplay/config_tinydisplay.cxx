// Filename: config_tinydisplay.cxx
// Created by:  drose (24Apr08)
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

#include "config_tinydisplay.h"
#include "tinySDLGraphicsPipe.h"
#include "tinySDLGraphicsWindow.h"
#include "tinyXGraphicsPipe.h"
#include "tinyXGraphicsWindow.h"
#include "tinyGraphicsStateGuardian.h"
#include "tinyGeomMunger.h"
#include "tinyTextureContext.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

Configure(config_tinydisplay);
NotifyCategoryDef(tinydisplay, "display");

ConfigureFn(config_tinydisplay) {
  init_libtinydisplay();
}

ConfigVariableString display_cfg
("display", "",
 PRC_DESC("Specify the X display string for the default display.  If this "
          "is not specified, $DISPLAY is used."));

ConfigVariableBool x_error_abort
("x-error-abort", false,
 PRC_DESC("Set this true to trigger and abort (and a stack trace) on receipt "
          "of an error from the X window system.  This can make it easier "
          "to discover where these errors are generated."));

ConfigVariableInt x_wheel_up_button
("x-wheel-up-button", 4,
 PRC_DESC("This is the mouse button index of the wheel_up event: which "
          "mouse button number does the system report when the mouse wheel "
          "is rolled one notch up?"));

ConfigVariableInt x_wheel_down_button
("x-wheel-down-button", 5,
 PRC_DESC("This is the mouse button index of the wheel_down event: which "
          "mouse button number does the system report when the mouse wheel "
          "is rolled one notch down?"));

ConfigVariableInt td_texture_ram
("td-texture-ram", -1,
 PRC_DESC("This specifies the maximum amount of RAM to devote to keeping "
          "textures resident with the tinydisplay software renderer.  When "
          "this limit is exceeded, textures over the limit that have not "
          "been rendered within the current frame will be evicted.  "
          "(Textures will not be evicted while they are still in the "
          "frame, even if this means this limit remains exceeded.)  "
          "Set it to -1 for no limit."));

////////////////////////////////////////////////////////////////////
//     Function: init_libtinydisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libtinydisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  TinyGraphicsStateGuardian::init_type();
  TinyGeomMunger::init_type();
  TinyTextureContext::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();

#ifdef IS_LINUX
  TinyXGraphicsPipe::init_type();
  TinyXGraphicsWindow::init_type();
  selection->add_pipe_type(TinyXGraphicsPipe::get_class_type(),
                           TinyXGraphicsPipe::pipe_constructor);
#endif

  TinySDLGraphicsPipe::init_type();
  TinySDLGraphicsWindow::init_type();
  selection->add_pipe_type(TinySDLGraphicsPipe::get_class_type(),
                           TinySDLGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("TinyGL");
}
