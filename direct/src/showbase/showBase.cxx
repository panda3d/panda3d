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
#include "graphicsWindow.h"
#include "chancfg.h"
#include "renderBuffer.h"
#include "get_config_path.h"
#include "camera.h"
#include "graphicsPipeSelection.h"


ConfigureDef(config_showbase);
ConfigureFn(config_showbase) {
}

DSearchPath &
get_particle_path() {
  static DSearchPath *particle_path = NULL;
  return get_config_path("particle-path", particle_path);
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
  RenderBuffer rb = gsg->get_render_buffer(RenderBuffer::T_front);

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
static int num_fullscreen_testsizes = 0;
#define MAX_FULLSCREEN_TESTS 10
static int fullscreen_testsizes[MAX_FULLSCREEN_TESTS * 2];

void
add_fullscreen_testsize(int xsize, int ysize) {
  if ((xsize == 0) && (ysize == 0)) {
    num_fullscreen_testsizes = 0;
    return;
  }

  // silently fail if maxtests exceeded
  if (num_fullscreen_testsizes < MAX_FULLSCREEN_TESTS) {
    fullscreen_testsizes[num_fullscreen_testsizes * 2] = xsize;
    fullscreen_testsizes[num_fullscreen_testsizes * 2 + 1] = ysize;
    num_fullscreen_testsizes++;
  }
}

void
runtest_fullscreen_sizes(GraphicsWindow *win) {
  win->verify_window_sizes(num_fullscreen_testsizes, fullscreen_testsizes);
}

bool
query_fullscreen_testresult(int xsize, int ysize) {
  // stupid linear search that works ok as long as total tests are small
  int i;
  for (i=0; i < num_fullscreen_testsizes; i++) {
    if((fullscreen_testsizes[i * 2] == xsize) &&
       (fullscreen_testsizes[i * 2 + 1] == ysize))
      return true;
  }
  return false;
}

