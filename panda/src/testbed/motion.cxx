// Filename: motion.cxx
// Created by:  frang (23Mar99)
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

#include "eventHandler.h"
#include "texture.h"
#include "graphicsWindow.h"
#include "graphicsStateGuardian.h"
#include "chancfg.h"
#include "notify.h"

#include <string>
#include "plist.h"

extern GraphicsWindow* win;

extern int framework_main(int argc, char *argv[]);
extern void (*extra_display_func)();
extern void (*define_keys)(EventHandler&);

#include <GL/gl.h>
typedef plist<Texture*> TexList;
TexList screenshots;
typedef plist<PixelBuffer*> PBList;
PBList sshots;
int num_frames = 4;
int frame_num = 0;

void motion_display_func( void ) {
  GraphicsStateGuardian* g = win->get_gsg();
  const RenderBuffer& r = g->get_render_buffer(RenderBuffer::T_front);
  Texture* t;
  PixelBuffer* p;

  if (frame_num < num_frames) {
    t = new Texture;
    t->_pbuffer->set_xsize(win->get_width());
    t->_pbuffer->set_ysize(win->get_height());
    p = new PixelBuffer;
    p->set_xsize(win->get_width());
    p->set_ysize(win->get_height());
    p->_image = PTA_uchar(win->get_width() * win->get_height() * 4);
  } else {
    TexList::iterator i = screenshots.begin();
    PBList::iterator l = sshots.begin();
    for (int j=0, k=frame_num%num_frames; j<k; ++i, ++j, ++l);
    t = *i;
    p = *l;
  }

  // g->copy_texture_from(t, r);
  g->copy_pixel_buffer_from(p, r);

  const RenderBuffer& rb = g->get_render_buffer(RenderBuffer::T_back);

  if (frame_num < num_frames) {
    // screenshots.push_back(t);
    sshots.push_back(p);
  }

  // now we accumulate.  Off the top of my head I have 3 plans for this:
  //   1) apply the screen shots to one or several whole-screen polygon(s) with
  //      appropriate alpha
  //   2) accumulate this into an accumulation buffer every frame
  //   3) blend the new frame into the accumulation buffer, and subtractively
  //      blend the old one out.

  // version 2.  Even this could be better if we didn't start by blowing away
  // the frame we just rendered.
  GLfloat d = 1. / GLfloat(screenshots.size() + 1.);
  TexList::iterator i = screenshots.begin();
  PBList::iterator j = sshots.begin();
  // for (TexList::iterator i=screenshots.begin(); i!=screenshots.end(); ++i) {
  for (; j!=sshots.end(); ++j) {
    // t = *i;
    // g->draw_texture_to(t, rb);
    p = *j;
    g->draw_pixel_buffer_to(p, rb);
    // if (i == screenshots.begin())
    if (j == sshots.begin())
      glAccum(GL_LOAD, d);
    else
      glAccum(GL_ACCUM, d);
  }
  glAccum(GL_RETURN, 1.0);
  g->end_frame();

  ++frame_num;
}

void event_plus(CPT_Event) {
  frame_num %= num_frames++;
  nout << "now blending " << num_frames << " frames." << endl;
}

void event_minus(CPT_Event) {
  frame_num %= --num_frames;
  nout << "now blending " << num_frames << " frames." << endl;
  // screenshots.erase(--(screenshots.end()));
  sshots.erase(--(sshots.end()));
}

void motion_keys(EventHandler& eh) {
  eh.add_hook("plus", event_plus);
  eh.add_hook("minus", event_minus);
}

int main(int argc, char *argv[]) {
  extra_display_func = &motion_display_func;
  define_keys = &motion_keys;
  return framework_main(argc, argv);
}
