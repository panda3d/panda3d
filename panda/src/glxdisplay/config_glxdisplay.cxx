// Filename: config_glxdisplay.cxx
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

#include "config_glxdisplay.h"
#include "glxGraphicsBuffer.h"
#include "glxGraphicsPipe.h"
#include "glxGraphicsWindow.h"
#include "glxGraphicsStateGuardian.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"

Configure(config_glxdisplay);
NotifyCategoryDef(glxdisplay, "display");

ConfigureFn(config_glxdisplay) {
  init_libglxdisplay();
}

ConfigVariableString display_cfg
("display", "");

ConfigVariableBool glx_error_abort
("glx-error-abort", false);

////////////////////////////////////////////////////////////////////
//     Function: init_libglxdisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libglxdisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

#ifdef HAVE_GLXFBCONFIG
  glxGraphicsBuffer::init_type();
#endif  // HAVE_GLXFBCONFIG
  glxGraphicsPipe::init_type();
  glxGraphicsWindow::init_type();
  glxGraphicsStateGuardian::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(glxGraphicsPipe::get_class_type(),
                           glxGraphicsPipe::pipe_constructor);
}
