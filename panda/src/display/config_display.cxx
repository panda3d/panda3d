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
#include "interactiveGraphicsPipe.h"
#include "noninteractiveGraphicsPipe.h"
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
static Config::ConfigTable::Symbol *guard;
static Config::ConfigTable::Symbol *preferred_pipe;
static Config::ConfigTable::Symbol *preferred_window;
static Config::ConfigTable::Symbol *preferred_gsg;

ConfigureFn(config_display) {
  init_libdisplay();
}

const string pipe_spec_machine = config_display.GetString("pipe-machine", "");
const string pipe_spec_filename = config_display.GetString("pipe-filename",
                                                           "outfile-%03f.rib");
const int pipe_spec_pipe_number = config_display.GetInt("pipe-number", -1);
const bool pipe_spec_is_file = config_display.Defined("pipe-filename")
                               || config_display.GetBool("pipe-file", false);
const bool pipe_spec_is_remote = config_display.Defined("pipe-machine")
                                 || config_display.GetBool("pipe-remote",
                                                           false);

const bool compare_state_by_pointer =
config_display.GetBool("compare-state-by-pointer", true);

// This is normally true to enable the cull traversal to perform
// state-sorting and alpha-sorting.  Turn this false to disable these
// features and likely improve cull performance at the expense of draw
// (and at the expense of correct alpha).
const bool cull_sorting = config_display.GetBool("cull-sorting", true);

// This is normally true; set it false to disable view-frustum culling
// (primarily useful for debugging).
const bool view_frustum_cull = config_display.GetBool("view-frustum-cull", true);

// Set this true to show the number of unused states in the pstats
// graph for TransformState and RenderState counts.  This adds a bit
// of per-frame overhead to count these things up.
const bool pstats_unused_states = config_display.GetBool("pstats-unused-states", false);


Config::ConfigTable::Symbol::iterator pipe_modules_begin(void) {
  return disp->begin();
}

Config::ConfigTable::Symbol::iterator pipe_modules_end(void) {
  return disp->end();
}

Config::ConfigTable::Symbol::iterator gsg_modules_begin(void) {
  return guard->begin();
}

Config::ConfigTable::Symbol::iterator gsg_modules_end(void) {
  return guard->end();
}


Config::ConfigTable::Symbol::iterator preferred_pipe_begin(void) {
  return preferred_pipe->begin();
}

Config::ConfigTable::Symbol::iterator preferred_pipe_end(void) {
  return preferred_pipe->end();
}

Config::ConfigTable::Symbol::iterator preferred_window_begin(void) {
  return preferred_window->begin();
}

Config::ConfigTable::Symbol::iterator preferred_window_end(void) {
  return preferred_window->end();
}

Config::ConfigTable::Symbol::iterator preferred_gsg_begin(void) {
  return preferred_gsg->begin();
}

Config::ConfigTable::Symbol::iterator preferred_gsg_end(void) {
  return preferred_gsg->end();
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
  GraphicsPipe::PipeParam::init_type();
  GraphicsPipe::PipeSpec::init_type();
  InteractiveGraphicsPipe::init_type();
  NoninteractiveGraphicsPipe::init_type();
  GraphicsWindow::init_type();
  GraphicsWindow::WindowParam::init_type();
  GraphicsWindow::WindowProps::init_type();
  GraphicsWindow::WindowPipe::init_type();
  GraphicsChannel::init_type();
  GraphicsLayer::init_type();
  HardwareChannel::init_type();
  TextureContext::init_type();
  GeomNodeContext::init_type();
  GeomContext::init_type();

  disp = new Config::ConfigTable::Symbol;
  guard = new Config::ConfigTable::Symbol;
  preferred_pipe = new Config::ConfigTable::Symbol;
  preferred_window = new Config::ConfigTable::Symbol;
  preferred_gsg = new Config::ConfigTable::Symbol;

  config_display.GetAll("load-display", *disp);
  config_display.GetAll("load-gsg", *guard);

  config_display.GetAll("preferred-pipe", *preferred_pipe);
  config_display.GetAll("preferred-window", *preferred_window);
  config_display.GetAll("preferred-gsg", *preferred_gsg);
}
