// Filename: showBase.cxx
// Created by:  shochet (02Feb00)
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

#include "showBase.h"

#include "throw_event.h"
#include "interactiveGraphicsPipe.h"
#include "graphicsWindow.h"
#include "chancfg.h"
#include "renderBuffer.h"
#include "get_config_path.h"
#include "qpcamera.h"

ConfigureDef(config_showbase);
ConfigureFn(config_showbase) {
}

DSearchPath &
get_particle_path() {
  static DSearchPath *particle_path = NULL;
  return get_config_path("particle-path", particle_path);
}

// Default channel config
std::string chan_config = "single";
std::string window_title = "Panda3D";


PT(GraphicsPipe) 
make_graphics_pipe() {
  PT(GraphicsPipe) main_pipe;

  // load display modules
  GraphicsPipe::resolve_modules();

  nout << "Known pipe types:" << endl;
  GraphicsPipe::get_factory().write_types(nout, 2);

  // Create a window
  main_pipe = GraphicsPipe::get_factory().
    make_instance(InteractiveGraphicsPipe::get_class_type());

  if (main_pipe == (GraphicsPipe*)0L) {
    nout << "No interactive pipe is available!  Check your Configrc!\n";
    return NULL;
  }

  nout << "Opened a '" << main_pipe->get_type().get_name()
       << "' interactive graphics pipe." << endl;
  return main_pipe;
}

qpChanConfig
qpmake_graphics_window(GraphicsPipe *pipe, const qpNodePath &render) {
  PT(GraphicsWindow) main_win;
  ChanCfgOverrides override;

  // Now use ChanConfig to create the window.
  override.setField(ChanCfgOverrides::Mask,
                    ((unsigned int)(W_DOUBLE|W_DEPTH|W_MULTISAMPLE)));

  std::string title = config_showbase.GetString("window-title", window_title);
  override.setField(ChanCfgOverrides::Title, title);

  std::string conf = config_showbase.GetString("chan-config", chan_config);
  qpChanConfig chan_config(pipe, conf, render, override);
  main_win = chan_config.get_win();
  assert(main_win != (GraphicsWindow*)0L);

  return chan_config;
}

// Throw the "NewFrame" event in the C++ world.  Some of the lerp code
// depends on receiving this.
void 
throw_new_frame() {
  throw_event("NewFrame");
}


void
take_snapshot(GraphicsWindow *win, const string &name) {
  GraphicsStateGuardian* gsg = win->get_gsg();
  const RenderBuffer& rb = gsg->get_render_buffer(RenderBuffer::T_front);

  CPT(DisplayRegion) dr = win->get_display_region(0);
  nassertv(dr != (DisplayRegion *)NULL);

  int width = dr->get_pixel_width();
  int height = dr->get_pixel_height();

  PixelBuffer p(width, height, 3, 1, PixelBuffer::T_unsigned_byte,
                PixelBuffer::F_rgb);

  p.copy(gsg, dr, rb);
  p.write(name);
}

// Returns the configure object for accessing config variables from a
// scripting language.
ConfigShowbase &
get_config_showbase() {
  return config_showbase;
}

// klunky interface since we cant pass array from python->C++ to use verify_window_sizes directly
static unsigned int num_fullscreen_testsizes=0;
#define MAX_FULLSCREEN_TESTS 10
static unsigned int fullscreen_testsizes[MAX_FULLSCREEN_TESTS*2];

void add_fullscreen_testsize(unsigned int xsize,unsigned int ysize) {
    if((xsize==0) && (ysize==0)) {
        num_fullscreen_testsizes=0;
        return;
    }

    // silently fail if maxtests exceeded
    if(num_fullscreen_testsizes<MAX_FULLSCREEN_TESTS) {
        fullscreen_testsizes[num_fullscreen_testsizes*2]=xsize;
        fullscreen_testsizes[num_fullscreen_testsizes*2+1]=ysize;
        num_fullscreen_testsizes++;
    }
}

void runtest_fullscreen_sizes(GraphicsWindow *win) {
    (void) win->verify_window_sizes(num_fullscreen_testsizes,fullscreen_testsizes);
}

bool query_fullscreen_testresult(unsigned int xsize,unsigned int ysize) {
    // stupid linear search that works ok as long as total tests are small
    unsigned int i;
    for(i=0;i<num_fullscreen_testsizes;i++) {
        if((fullscreen_testsizes[i*2]==xsize) &&
           (fullscreen_testsizes[i*2+1]==ysize))
          return true;
    }
    return false;
}

