// Filename: indirect.cxx
// Created by:  cary (25Mar99)
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

#include <eventHandler.h>
#include <texture.h>
#include <graphicsWindow.h>
#include <graphicsStateGuardian.h>
#include <pixelBuffer.h>
#include "dconfig.h"
#include <chancfg.h>
#include <string>

Configure(indirect);

extern GraphicsWindow* win;
extern std::string chan_config;

extern int framework_main(int argc, char *argv[]);
extern void (*extra_display_func)();
extern void (*define_keys)(EventHandler&);
extern void (*first_init)();

Texture* t = (Texture*)0L;
PixelBuffer* pb = (PixelBuffer *)0L;
DisplayRegion* dr1 = (DisplayRegion*)0L;
DisplayRegion* dr2 = (DisplayRegion*)0L;

bool use_canned_texture = indirect.GetBool("canned-texture", false);
bool use_texture = indirect.GetBool("use-texture", true);
bool full_region = indirect.GetBool("full-region", true);
bool side_by_side = indirect.GetBool("side-by-side", true);
bool right_to_left = indirect.GetBool("right-to-left", true);

int logs[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 0 };

int binary_log_cap(int x) {
  int i = 0;
  for (; (x > logs[i])&&(logs[i] != 0); ++i);
  if (logs[i] == 0)
    return 4096;
  return logs[i];
}

void indirect_display_func( void ) {
  GraphicsStateGuardian* g = win->get_gsg();
  const RenderBuffer& rf = g->get_render_buffer(RenderBuffer::T_front);
  const RenderBuffer& rb = g->get_render_buffer(RenderBuffer::T_back);

  if (use_texture) {
    if (t == (Texture*)0L) {
      t = new Texture;
      if (use_canned_texture)
        t->read( "smiley.rgba" );
      else if (full_region) {
        int a, b, w, h;
        dr1->get_region_pixels(a, b, w, h);
        t->_pbuffer->set_xsize(binary_log_cap(w));
        t->_pbuffer->set_ysize(binary_log_cap(h));
      } else {
        t->_pbuffer->set_xsize( 512 );
        t->_pbuffer->set_ysize( 512 );
      }
    }
    if (side_by_side) {
      if (right_to_left)
        g->prepare_display_region(dr2);
      else
        g->prepare_display_region(dr1);
    }
    if (!use_canned_texture) {
      if (full_region) {
        if (side_by_side) {
          if (right_to_left)
            t->copy_from( rb, dr2 );
          else
            t->copy_from( rb, dr1 );
        } else
          t->copy_from( rb, dr1 );
      } else
        t->copy_from( rb );
    }
    if (side_by_side) {
      if (right_to_left)
        g->prepare_display_region(dr1);
      else
        g->prepare_display_region(dr2);
    }
    if (full_region) {
      if (side_by_side) {
        if (right_to_left)
          t->draw_to( rf, dr1 );
        else
          t->draw_to( rf, dr2 );
      } else
        t->draw_to( rf, dr1 );
    } else
      t->draw_to( rf );
  } else {
    if (pb == (PixelBuffer*)0L) {
      pb = new PixelBuffer;
      if (use_canned_texture)
        pb->read( "smiley.rgba" );
      else if (full_region) {
        int a, b, w, h;
        dr1->get_region_pixels(a, b, w, h);
        w = binary_log_cap(w);
        h = binary_log_cap(h);
        pb->set_xsize(w);
        pb->set_ysize(h);
        pb->_image = PTA_uchar(w * h * 3);
      } else {
        pb->set_xsize(512);
        pb->set_ysize(512);
        pb->_image = PTA_uchar(512 * 512 * 3);
      }
    }
    if (side_by_side) {
      if (right_to_left)
        g->prepare_display_region(dr2);
      else
        g->prepare_display_region(dr1);
    }
    if (!use_canned_texture)
      pb->copy_from( rf );
    if (side_by_side) {
      if (right_to_left)
        g->prepare_display_region(dr1);
      else
        g->prepare_display_region(dr2);
    }
    pb->draw_to( rf );
  }
}

void indirect_init(void) {
  if (side_by_side)
    chan_config = "two-frame";
}

void indirect_keys(EventHandler&) {
  DRList::iterator i = win->getDRBegin();
  dr1 = *i;
  if (side_by_side) {
    ++i;
    dr2 = *i;
    if (right_to_left)
      dr1->set_active(false);
    else
      dr2->set_active(false);
  }
}

int main(int argc, char *argv[]) {
  define_keys = &indirect_keys;
  extra_display_func = &indirect_display_func;
  first_init = &indirect_init;
  return framework_main(argc, argv);
}
