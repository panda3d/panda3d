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

ConfigVariableBool depth_offset_decals
("depth-offset-decals", false,
 PRC_DESC("Set this true to allow decals to be implemented via the advanced "
          "depth offset feature, if supported, instead of via the traditional "
          "(and slower) two-pass approach.  This is false by default "
          "because it appears that many graphics drivers have issues with "
          "their depth offset implementation."));

ConfigVariableInt win_size
("win-size", "640 480",
 PRC_DESC("This is the default size at which to open a new window.  This "
          "replaces the deprecated win-width and win-height variables."));

ConfigVariableInt win_origin
("win-origin", "0 0",
 PRC_DESC("This is the default position at which to open a new window.  This "
          "replaces the deprecated win-origin-x and win-origin-y variables."));

ConfigVariableInt win_width
("win-width", 0);

ConfigVariableInt win_height
("win-height", 0);

ConfigVariableInt win_origin_x
("win-origin-x", 0);

ConfigVariableInt win_origin_y
("win-origin-y", 0);

ConfigVariableBool fullscreen
("fullscreen", false);

ConfigVariableBool undecorated
("undecorated", false);

ConfigVariableBool cursor_hidden
("cursor-hidden", false);

ConfigVariableFilename icon_filename
("icon-filename", "");

ConfigVariableFilename cursor_filename
("cursor-filename", "");

ConfigVariableEnum<WindowProperties::ZOrder> z_order
("z-order", WindowProperties::Z_normal);

ConfigVariableString window_title
("window-title", "Panda");

ConfigVariableString framebuffer_mode
("framebuffer-mode", "rgba double-buffer depth multisample hardware",
 PRC_DESC("A space-separated list of keywords that describe the default "
          "framebuffer properties requested for a window."));
ConfigVariableInt depth_bits
("depth-bits", 1,
 PRC_DESC("The minimum number of depth bits requested if the depth keyword "
          "is present in framebuffer-mode."));
ConfigVariableInt color_bits
("color-bits", 1,
 PRC_DESC("The minimum number of color bits requested in the default "
          "framebuffer properties (sum of all three color channels)."));
ConfigVariableInt alpha_bits
("alpha-bits", 1,
 PRC_DESC("The minimum number of alpha bits requested if the alpha or rgba "
          "keyword is present in framebuffer-mode."));
ConfigVariableInt stencil_bits
("stencil-bits", 1,
 PRC_DESC("The minimum number of stencil bits requested if the stencil keyword "
          "is present in framebuffer-mode."));
ConfigVariableInt multisamples
("multisamples", 1,
 PRC_DESC("The minimum number of samples requested if the multisample keyword "
          "is present in framebuffer-mode."));

ConfigVariableDouble background_color
("background-color", "0.41 0.41 0.41",
 PRC_DESC("Specifies the rgb(a) value of the default background color for a "
          "new window or offscreen buffer."));


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
