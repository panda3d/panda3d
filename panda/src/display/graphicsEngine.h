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
class GraphicsPipe;

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
//               various cull and draw threads.  The application
//               simply calls engine->render_frame() and considers it
//               done.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsEngine {
PUBLISHED:
  GraphicsEngine(Pipeline *pipeline = NULL);
  ~GraphicsEngine();

  void set_threading_model(const string &threading_model);
  string get_threading_model() const;

  INLINE GraphicsWindow *make_window(GraphicsPipe *pipe);
  GraphicsWindow *make_window(GraphicsPipe *pipe,
                              const string &threading_model);
  bool remove_window(GraphicsWindow *window);
  void remove_all_windows();

  void render_frame();
  void sync_frame();
  
  void render_subframe(GraphicsStateGuardian *gsg, DisplayRegion *dr,
                       bool cull_sorting);

private:
  typedef pset< PT(GraphicsWindow) > Windows;

  void cull_and_draw_together(const Windows &wlist);
  void cull_and_draw_together(GraphicsStateGuardian *gsg, DisplayRegion *dr);

  void cull_bin_draw(const Windows &wlist);
  void cull_bin_draw(GraphicsStateGuardian *gsg, DisplayRegion *dr);

  void process_events(const GraphicsEngine::Windows &wlist);
  void flip_windows(const GraphicsEngine::Windows &wlist);
  void do_sync_frame();

  PT(SceneSetup) setup_scene(const NodePath &camera, 
                             GraphicsStateGuardian *gsg);
  void do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
               GraphicsStateGuardian *gsg);
  void do_draw(CullResult *cull_result, SceneSetup *scene_setup,
               GraphicsStateGuardian *gsg, DisplayRegion *dr);

  bool setup_gsg(GraphicsStateGuardian *gsg, SceneSetup *scene_setup);

  void do_remove_window(GraphicsWindow *window);
  void terminate_threads();

  // The WindowRenderer class records the stages of the pipeline that
  // each thread (including the main thread, a.k.a. "app") should
  // process, and the list of windows for each stage.
  class WindowRenderer {
  public:
    void add_window(Windows &wlist, GraphicsWindow *window);
    void remove_window(GraphicsWindow *window);
    void do_frame(GraphicsEngine *engine);
    void do_flip(GraphicsEngine *engine);
    void do_release(GraphicsEngine *engine);
    void do_close(GraphicsEngine *engine);
    void do_pending(GraphicsEngine *engine);

    Windows _cull;    // cull stage
    Windows _cdraw;   // cull-and-draw-together stage
    Windows _draw;    // draw stage
    Windows _window;  // window stage, i.e. process windowing events 
    Windows _pending_release; // moved from _draw, pending release_gsg.
    Windows _pending_close;   // moved from _window, pending close.
    Mutex _wl_lock;
  };

  enum ThreadState {
    TS_wait,
    TS_do_frame,
    TS_do_flip,
    TS_do_release,
    TS_terminate
  };

  class RenderThread : public Thread, public WindowRenderer {
  public:
    RenderThread(const string &name, GraphicsEngine *engine);
    virtual void thread_main();

    GraphicsEngine *_engine;
    Mutex _cv_mutex;
    ConditionVar _cv;
    ThreadState _thread_state;
  };

  WindowRenderer *get_window_renderer(const string &name);

  Pipeline *_pipeline;
  Windows _windows;

  WindowRenderer _app;
  typedef pmap<string, PT(RenderThread) > Threads;
  Threads _threads;
  string _threading_model;

  bool _needs_sync;
  Mutex _lock;

  static PStatCollector _cull_pcollector;
  static PStatCollector _draw_pcollector;
  friend class WindowRenderer;
};

#include "graphicsEngine.I"

#endif

