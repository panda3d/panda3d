// Filename: config_wgldisplay.cxx
// Created by:  drose (20Dec02)
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

#include "config_wgldisplay.h"
#include "wglGraphicsBuffer.h"
#include "wglGraphicsPipe.h"
#include "wglGraphicsStateGuardian.h"
#include "wglGraphicsWindow.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"

Configure(config_wgldisplay);
NotifyCategoryDef(wgldisplay, "windisplay");

ConfigureFn(config_wgldisplay) {
  init_libwgldisplay();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libwgldisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libwgldisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  wglGraphicsBuffer::init_type();
  wglGraphicsPipe::init_type();
  wglGraphicsStateGuardian::init_type();
  wglGraphicsWindow::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wglGraphicsPipe::get_class_type(),
                           wglGraphicsPipe::pipe_constructor);
}

int gl_force_pixfmt = config_wgldisplay.GetInt("gl-force-pixfmt", 0);

// Set this true to force all GL windows to fail to open correctly.
bool gl_force_invalid = config_wgldisplay.GetBool("gl-force-invalid", false);

// This is true to insist that low-memory cards open only 640x480
// fullscreen windows, no matter what resolution of window was
// requested.  It only affects fullscreen windows.
bool gl_do_vidmemsize_check = config_wgldisplay.GetBool("gl-do-vidmemsize-check", true);

// For now, we fake offscreen rendering without using the
// poorly-supported pbuffer extension; we simply render into an
// invisible window.  Some drivers don't like that, so for these
// drivers set show-pbuffers to true to force the window to be visible
// so you can at least get some work done.
bool show_pbuffers = config_wgldisplay.GetBool("show-pbuffers", false);
