// Filename: config_display.cxx
// Created by:  drose (06Oct99)
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


#include "config_display.h"
#include "graphicsStateGuardian.h"
#include "savedFrameBuffer.h"
#include "graphicsPipe.h"
#include "graphicsWindow.h"
#include "graphicsChannel.h"
#include "graphicsLayer.h"
#include "hardwareChannel.h"
#include "textureContext.h"
#include "geomNodeContext.h"
#include "geomContext.h"

Configure(config_display);
NotifyCategoryDef(display, "");
NotifyCategoryDef(gsg, display_cat);

static Config::ConfigTable::Symbol *disp;

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

// Use this to specify a particular pipe you prefer to use, when more
// than one GraphicsPipe is available (for instance, if both GL and DX
// are linked in).  This should be the name of the type preferred, or
// a substring unique to the type name.
const string preferred_pipe = config_display.GetString("preferred-pipe", "");


Config::ConfigTable::Symbol::iterator display_modules_begin(void) {
  return disp->begin();
}

Config::ConfigTable::Symbol::iterator display_modules_end(void) {
  return disp->end();
}

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
  GraphicsStateGuardian::GsgParam::init_type();
  GraphicsStateGuardian::GsgWindow::init_type();
  SavedFrameBuffer::init_type();
  GraphicsPipe::init_type();
  GraphicsWindow::init_type();
  GraphicsChannel::init_type();
  GraphicsLayer::init_type();
  HardwareChannel::init_type();
  TextureContext::init_type();
  GeomNodeContext::init_type();
  GeomContext::init_type();

  disp = new Config::ConfigTable::Symbol;
  config_display.GetAll("load-display", *disp);
}
