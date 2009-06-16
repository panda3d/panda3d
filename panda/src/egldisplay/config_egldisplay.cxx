// Filename: config_egldisplay.cxx
// Created by:  cary (07Oct99)
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

#include "config_egldisplay.h"
#include "eglGraphicsPipe.h"
#include "eglGraphicsWindow.h"
#include "eglGraphicsStateGuardian.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

Configure(config_egldisplay);
NotifyCategoryDef(egldisplay, "display");

ConfigureFn(config_egldisplay) {
  init_libegldisplay();
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

ConfigVariableInt x_wheel_left_button
("x-wheel-left-button", 6,
 PRC_DESC("This is the mouse button index of the wheel_left event: which "
          "mouse button number does the system report when one scrolls "
          "to the left?"));

ConfigVariableInt x_wheel_right_button
("x-wheel-right-button", 7,
 PRC_DESC("This is the mouse button index of the wheel_right event: which "
          "mouse button number does the system report when one scrolls "
          "to the right?"));

////////////////////////////////////////////////////////////////////
//     Function: init_libegldisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libegldisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  eglGraphicsPipe::init_type();
  eglGraphicsWindow::init_type();
  eglGraphicsStateGuardian::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(eglGraphicsPipe::get_class_type(),
                           eglGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
#ifdef OPENGLES_2
  ps->set_system_tag("OpenGL ES 2", "window_system", "EGL");
#else
  ps->set_system_tag("OpenGL ES", "window_system", "EGL");
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: get_egl_error_string
//  Description: Returns the given EGL error as string.
////////////////////////////////////////////////////////////////////
const string get_egl_error_string(int error) {
  switch (error) {
    case 0x3000: return "EGL_SUCCESS"; break;
    case 0x3001: return "EGL_NOT_INITIALIZED"; break;
    case 0x3002: return "EGL_BAD_ACCESS"; break;
    case 0x3003: return "EGL_BAD_ALLOC"; break;
    case 0x3004: return "EGL_BAD_ATTRIBUTE"; break;
    case 0x3005: return "EGL_BAD_CONFIG"; break;
    case 0x3006: return "EGL_BAD_CONTEXT"; break;
    case 0x3007: return "EGL_BAD_CURRENT_SURFACE"; break;
    case 0x3008: return "EGL_BAD_DISPLAY"; break;
    case 0x3009: return "EGL_BAD_MATCH"; break;
    case 0x300A: return "EGL_BAD_NATIVE_PIXMAP"; break;
    case 0x300B: return "EGL_BAD_NATIVE_WINDOW"; break;
    case 0x300C: return "EGL_BAD_PARAMETER"; break;
    case 0x300D: return "EGL_BAD_SURFACE"; break;
    case 0x300E: return "EGL_CONTEXT_LOST"; break;
    default: return "Unknown error";
  }
}
