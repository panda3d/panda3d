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
#include "sceneSetup.h"
#include "pointerTo.h"
#include "thread.h"
#include "mutex.h"
#include "conditionVar.h"
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
  ~GraphicsEngine();

  void add_window(GraphicsWindow *window);
  bool remove_window(GraphicsWindow *window);

  void render_frame();
  void render_subframe(GraphicsStateGuardian *gsg, DisplayRegion *dr);

  enum ThreadingModel {
    TM_invalid,
    TM_appculldraw,
    TM_appcull_draw,
    TM_app_culldraw,
    TM_app_cull_draw,
    TM_appcdraw,
    TM_app_cdraw,
  };

  void set_threading_model(ThreadingModel threading_model);
  INLINE ThreadingModel get_threading_model() const;

public:
  static ThreadingModel string_threading_model(const string &string);
  
private:
  INLINE void start_cull();
  void cull_and_draw_together();
  void cull_and_draw_together(GraphicsStateGuardian *gsg, DisplayRegion *dr);

  void cull_bin_draw();
  void cull_bin_draw(GraphicsStateGuardian *gsg, DisplayRegion *dr);

  PT(SceneSetup) setup_scene(const NodePath &camera, 
                             GraphicsStateGuardian *gsg);
  void do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
               GraphicsStateGuardian *gsg);
  void do_draw(CullResult *cull_result, SceneSetup *scene_setup,
               GraphicsStateGuardian *gsg, DisplayRegion *dr);

  bool setup_gsg(GraphicsStateGuardian *gsg, SceneSetup *scene_setup);

  void terminate_threads();

  enum ThreadState {
    TS_wait,
    TS_do_frame,
    TS_terminate
  };

  class CullThread : public Thread {
  public:
    CullThread(const string &name, GraphicsEngine *engine);
    virtual void thread_main();

    GraphicsEngine *_engine;
    Mutex _cv_mutex;
    ConditionVar _cv;
    ThreadState _thread_state;
  };

  class DrawThread : public Thread {
  public:
    DrawThread(const string &name, GraphicsEngine *engine);
    virtual void thread_main();

    GraphicsEngine *_engine;
    Mutex _cv_mutex;
    ConditionVar _cv;
    ThreadState _thread_state;
  };

  Pipeline *_pipeline;

  typedef pset<PT(GraphicsWindow)> Windows;
  Windows _windows;

  ThreadingModel _threading_model;
  bool _cull_sorting;

  PT(CullThread) _cull_thread;
  PT(DrawThread) _draw_thread;

  static PStatCollector _cull_pcollector;
  static PStatCollector _draw_pcollector;

  friend class CullThread;
};

ostream &
operator << (ostream &out, GraphicsEngine::ThreadingModel threading_model);

#include "graphicsEngine.I"

#endif

