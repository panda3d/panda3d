// Filename: config_display.cxx
// Created by:  drose (06Oct99)
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


#include "config_display.h"
#include "graphicsStateGuardian.h"
#include "savedFrameBuffer.h"
#include "graphicsPipe.h"
#include "graphicsOutput.h"
#include "graphicsBuffer.h"
#include "graphicsWindow.h"
#include "graphicsChannel.h"
#include "graphicsLayer.h"
#include "parasiteBuffer.h"
#include "hardwareChannel.h"
#include "textureContext.h"
#include "geomNodeContext.h"
#include "geomContext.h"

ConfigureDef(config_display);
NotifyCategoryDef(display, "");
NotifyCategoryDef(gsg, display_cat);

ConfigureFn(config_display) {
  init_libdisplay();
}

// This is normally true; set it false to disable view-frustum culling
// (primarily useful for debugging).
const bool view_frustum_cull = config_display.GetBool("view-frustum-cull", true);

// Set this true to show the number of unused states in the pstats
// graph for TransformState and RenderState counts.  This adds a bit
// of per-frame overhead to count these things up.
const bool pstats_unused_states = config_display.GetBool("pstats-unused-states", false);

// This is the default threading model to use for new windows.  Use
// empty string for single-threaded, or something like "cull/draw" for
// a 3-stage pipeline.  See GraphicsEngine::set_threading_model().

// Warning!  The code that uses this is currently experimental and
// incomplete, and will almost certainly crash!  Do not set
// threading-model to anything other than its default of a
// single-threaded model unless you are developing Panda's threading
// system!
const string threading_model = config_display.GetString("threading-model", "");

// This indicates the initial setting of the auto-flip flag.  Set it
// true (the default) to cause render_frame() to flip all the windows
// before it returns (in single-threaded mode only), or false to wait
// until an explicit call to flip_frame() or the next render_frame().
const bool auto_flip = config_display.GetBool("auto-flip", true);

// This indicates if you want multiple window support for same GSG
const bool multiple_windows = config_display.GetBool("multiple-windows", false);

// Set this true to yield the timeslice at the end of the frame to be
// more polite to other applications that are trying to run.
const bool yield_timeslice = config_display.GetBool("yield-timeslice", false);

const string screenshot_filename = config_display.GetString("screenshot-filename", "%~p-%a-%b-%d-%H-%M-%S-%Y-%~f.%~e");
const string screenshot_extension = config_display.GetString("screenshot-extension", "jpg");

// Set this true to cause offscreen GraphicsBuffers to be created as
// GraphicsWindows, if possible, so that their contents may be viewed
// interactively.  Handy during development of multipass algorithms.
const bool show_buffers = config_display.GetBool("show-buffers", false);

// Use the variable load-display to specify the name of the default
// graphics display library or GraphicsPipe to load.  It is the name
// of a shared library (or * for all libraries named in aux-display),
// optionally followed by the name of the particular GraphicsPipe
// class to create.

// Also use the variable aux-display to name each of the graphics
// display libraries that are available on a particular platform.
// This variable may be repeated several times.


////////////////////////////////////////////////////////////////////
//     Function: init_libdisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  GraphicsStateGuardian::init_type();
  SavedFrameBuffer::init_type();
  GraphicsPipe::init_type();
  GraphicsOutput::init_type();
  GraphicsWindow::init_type();
  GraphicsBuffer::init_type();
  GraphicsChannel::init_type();
  GraphicsLayer::init_type();
  HardwareChannel::init_type();
  ParasiteBuffer::init_type();
  TextureContext::init_type();
  GeomNodeContext::init_type();
  GeomContext::init_type();
}
