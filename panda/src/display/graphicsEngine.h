// Filename: graphicsEngine.h
// Created by:  drose (24Feb02)
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

#ifndef GRAPHICSENGINE_H
#define GRAPHICSENGINE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "pointerTo.h"
#include "pset.h"
#include "pStatCollector.h"

class Pipeline;
class DisplayRegion;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsEngine
// Description : This class is the main interface to controlling the
//               render process.  There is typically only one
//               GraphicsEngine in an application, and it synchronizes
//               rendering to all all of the active windows; although
//               it is possible to have multiple GraphicsEngine
//               objects if multiple synchronicity groups are
//               required.
//
//               The GraphicsEngine is responsible for managing the
//               cull and draw processes.  The application simply
//               calls engine->render_frame() and considers it done.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsEngine : public Namable {
PUBLISHED:
  GraphicsEngine(Pipeline *pipeline = NULL);

  void add_window(GraphicsWindow *window);
  bool remove_window(GraphicsWindow *window);

  void render_frame();

private:
  void cull_and_draw_together();
  void cull_and_draw_together(GraphicsWindow *win, DisplayRegion *dr);

  void cull_bin_draw();
  void cull_bin_draw(GraphicsWindow *win, DisplayRegion *dr);

  void do_cull(CullHandler *cull_handler, const qpNodePath &camera,
               GraphicsStateGuardian *gsg);
  void do_draw(CullResult *cull_result, GraphicsStateGuardian *gsg,
               DisplayRegion *dr);

  bool set_gsg_lens(GraphicsStateGuardian *gsg, DisplayRegion *dr);

  Pipeline *_pipeline;

  typedef pset<PT(GraphicsWindow)> Windows;
  Windows _windows;

  static PStatCollector _cull_pcollector;
  static PStatCollector _draw_pcollector;
};

#include "graphicsEngine.I"

#endif

