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
#include "parasiteBuffer.h"
#include "textureContext.h"

ConfigureDef(config_display);
NotifyCategoryDef(display, "");
NotifyCategoryDef(gsg, display_cat);

ConfigureFn(config_display) {
  init_libdisplay();
}

ConfigVariableBool view_frustum_cull
("view-frustum-cull", true,
 PRC_DESC("This is normally true; set it false to disable view-frustum culling "
          "(primarily useful for debugging)."));

ConfigVariableBool pstats_unused_states
("pstats-unused-states", false,
 PRC_DESC("Set this true to show the number of unused states in the pstats "
          "graph for TransformState and RenderState counts.  This adds a bit "
          "of per-frame overhead to count these things up."));


// Warning!  The code that uses this is currently experimental and
// incomplete, and will almost certainly crash!  Do not set
// threading-model to anything other than its default of a
// single-threaded model unless you are developing Panda's threading
// system!
ConfigVariableString threading_model
("threading-model", "",
 PRC_DESC("This is the default threading model to use for new windows.  Use "
          "empty string for single-threaded, or something like \"cull/draw\" for "
          "a 3-stage pipeline.  See GraphicsEngine::set_threading_model(). "
          "EXPERIMENTAL and incomplete, do not use this!"));

ConfigVariableBool auto_flip
("auto-flip", true,
 PRC_DESC("This indicates the initial setting of the auto-flip flag.  Set it "
          "true (the default) to cause render_frame() to flip all the windows "
          "before it returns (in single-threaded mode only), or false to wait "
          "until an explicit call to flip_frame() or the next render_frame()."));

ConfigVariableBool yield_timeslice
("yield-timeslice", false,
 PRC_DESC("Set this true to yield the timeslice at the end of the frame to be "
          "more polite to other applications that are trying to run."));

ConfigVariableString screenshot_filename
("screenshot-filename", "%~p-%a-%b-%d-%H-%M-%S-%Y-%~f.%~e",
 PRC_DESC("This specifies the filename pattern to be used to generate "
          "screenshots captured via save_screenshot_default().  See "
          "DisplayRegion::save_screenshot()."));
ConfigVariableString screenshot_extension
("screenshot-extension", "jpg",
 PRC_DESC("This specifies the default filename extension (and therefore the "
          "default image type) to be used for saving screenshots."));

ConfigVariableBool show_buffers
("show-buffers", false,
 PRC_DESC("Set this true to cause offscreen GraphicsBuffers to be created as "
          "GraphicsWindows, if possible, so that their contents may be viewed "
          "interactively.  Handy during development of multipass algorithms."));

ConfigVariableBool prefer_parasite_buffer
("prefer-parasite-buffer", true,
 PRC_DESC("Set this true to make GraphicsOutput::make_render_texture() try to "
          "create a parasite buffer before it tries to create an offscreen "
          "buffer.  This may be desired if you know your graphics API does not "
          "support render-directly-to-texture and you want to minimize "
          "framebuffer memory."));

ConfigVariableBool prefer_single_buffer
("prefer-single-buffer", true,
 PRC_DESC("Set this true to make GraphicsOutput::make_render_texture() first "
          "try to create a single-buffered offscreen buffer, before falling "
          "back to a double-buffered one (or whatever kind the source window "
          "has).  This is true by default to reduce waste of framebuffer "
          "memory, but you may get a performance benefit by setting it to "
          "false (since in that case the buffer can share a graphics context "
          "with the window)."));


ConfigVariableBool copy_texture_inverted
("copy-texture-inverted", false,
 PRC_DESC("Set this true to indicate that the GSG in use will invert textures when "
          "it performs a framebuffer-to-texture copy operation, or false to indicate "
          "that it does the right thing.  If this is not set, the default behavior is "
          "determined by the GSG's internal logic."));

ConfigVariableBool window_inverted
("window-inverted", false,
 PRC_DESC("Set this true to create all windows with the inverted flag set, so that "
          "they will render upside-down and backwards.  Normally this is useful only "
          "for debugging."));


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
  ParasiteBuffer::init_type();
  TextureContext::init_type();
}
