/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsEngine.h
 * @author drose
 * @date 2002-02-24
 */

#ifndef GRAPHICSENGINE_H
#define GRAPHICSENGINE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsBuffer.h"
#include "frameBufferProperties.h"
#include "graphicsThreadingModel.h"
#include "sceneSetup.h"
#include "pointerTo.h"
#include "thread.h"
#include "pmutex.h"
#include "reMutex.h"
#include "lightReMutex.h"
#include "conditionVar.h"
#include "pStatCollector.h"
#include "pset.h"
#include "ordered_vector.h"
#include "indirectLess.h"
#include "loader.h"
#include "referenceCount.h"

class Pipeline;
class DisplayRegion;
class GraphicsPipe;
class FrameBufferProperties;
class Texture;

/**
 * This class is the main interface to controlling the render process.  There
 * is typically only one GraphicsEngine in an application, and it synchronizes
 * rendering to all all of the active windows; although it is possible to have
 * multiple GraphicsEngine objects if multiple synchronicity groups are
 * required.
 *
 * The GraphicsEngine is responsible for managing the various cull and draw
 * threads.  The application simply calls engine->render_frame() and considers
 * it done.
 */
class EXPCL_PANDA_DISPLAY GraphicsEngine : public ReferenceCount {
PUBLISHED:
  explicit GraphicsEngine(Pipeline *pipeline = nullptr);
  BLOCKING ~GraphicsEngine();

  void set_threading_model(const GraphicsThreadingModel &threading_model);
  GraphicsThreadingModel get_threading_model() const;
  MAKE_PROPERTY(threading_model, get_threading_model, set_threading_model);

  INLINE const ReMutex &get_render_lock() const;
  MAKE_PROPERTY(render_lock, get_render_lock);

  INLINE void set_auto_flip(bool auto_flip);
  INLINE bool get_auto_flip() const;
  MAKE_PROPERTY(auto_flip, get_auto_flip, set_auto_flip);

  INLINE void set_portal_cull(bool value);
  INLINE bool get_portal_cull() const;
  MAKE_PROPERTY(portal_cull, get_portal_cull, set_portal_cull);

  INLINE void set_default_loader(Loader *loader);
  INLINE Loader *get_default_loader() const;
  MAKE_PROPERTY(default_loader, get_default_loader, set_default_loader);

  GraphicsOutput *make_output(GraphicsPipe *pipe,
                              const std::string &name, int sort,
                              const FrameBufferProperties &fb_prop,
                              const WindowProperties &win_prop,
                              int flags, GraphicsStateGuardian *gsg = nullptr,
                              GraphicsOutput *host = nullptr);

  // Syntactic shorthand versions of make_output
  INLINE GraphicsOutput *make_buffer(GraphicsOutput *host,
                                     const std::string &name, int sort,
                                     int x_size, int y_size);
  INLINE GraphicsOutput *make_buffer(GraphicsStateGuardian *gsg,
                                     const std::string &name, int sort,
                                     int x_size, int y_size);
  INLINE GraphicsOutput *make_parasite(GraphicsOutput *host,
                                       const std::string &name, int sort,
                                       int x_size, int y_size);

  bool add_window(GraphicsOutput *window, int sort);
  bool remove_window(GraphicsOutput *window);
  BLOCKING void remove_all_windows();
  void reset_all_windows(bool swapchain);

  bool is_empty() const;
  int get_num_windows() const;
  GraphicsOutput *get_window(int n) const;
  MAKE_SEQ(get_windows, get_num_windows, get_window);
  MAKE_SEQ_PROPERTY(windows, get_num_windows, get_window);

  BLOCKING void render_frame();
  BLOCKING void open_windows();
  BLOCKING void sync_frame();
  BLOCKING void ready_flip();
  BLOCKING void flip_frame();

  bool extract_texture_data(Texture *tex, GraphicsStateGuardian *gsg);
  void dispatch_compute(const LVecBase3i &work_groups,
                        const ShaderAttrib *sattr,
                        GraphicsStateGuardian *gsg);

  static GraphicsEngine *get_global_ptr();

public:
  enum ThreadState {
    TS_wait,
    TS_do_frame,
    TS_do_flip,
    TS_do_release,
    TS_do_windows,
    TS_do_compute,
    TS_do_extract,
    TS_do_screenshot,
    TS_terminate,
    TS_done
  };

  void texture_uploaded(Texture *tex);
  PT(Texture) do_get_screenshot(DisplayRegion *region, GraphicsStateGuardian *gsg);

public:
  static void do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
                      GraphicsStateGuardian *gsg, Thread *current_thread);

private:
  typedef ov_set< PT(GraphicsOutput), IndirectLess<GraphicsOutput> > Windows;
  typedef pset< PT(GraphicsStateGuardian) > GSGs;

  static bool scene_root_func(const PandaNode *node);
  bool is_scene_root(const PandaNode *node);

  void set_window_sort(GraphicsOutput *window, int sort);

  void cull_and_draw_together(Windows wlist, Thread *current_thread);
  void cull_and_draw_together(GraphicsOutput *win, DisplayRegion *dr,
                              Thread *current_thread);

  void cull_to_bins(Windows wlist, Thread *current_thread);
  void cull_to_bins(GraphicsOutput *win, GraphicsStateGuardian *gsg,
                    DisplayRegion *dr, SceneSetup *scene_setup,
                    CullResult *cull_result, Thread *current_thread);
  void draw_bins(const Windows &wlist, Thread *current_thread);
  void make_contexts(const Windows &wlist, Thread *current_thread);

  void process_events(const Windows &wlist, Thread *current_thread);
  void ready_flip_windows(const Windows &wlist, Thread *current_thread);
  void flip_windows(const Windows &wlist, Thread *current_thread);
  void do_sync_frame(Thread *current_thread);
  void do_ready_flip(Thread *current_thread);
  void do_flip_frame(Thread *current_thread);
  INLINE void close_gsg(GraphicsPipe *pipe, GraphicsStateGuardian *gsg);

  PT(SceneSetup) setup_scene(GraphicsStateGuardian *gsg,
                             DisplayRegionPipelineReader *dr);
  void do_draw(GraphicsOutput *win, GraphicsStateGuardian *gsg,
               DisplayRegion *dr, Thread *current_thread);

  void do_add_window(GraphicsOutput *window);
  void do_add_gsg(GraphicsStateGuardian *gsg, GraphicsPipe *pipe);
  void do_remove_window(GraphicsOutput *window, Thread *current_thread);
  void do_resort_windows();
  void terminate_threads(Thread *current_thread);
  void auto_adjust_capabilities(GraphicsStateGuardian *gsg);

#ifdef DO_PSTATS
  typedef std::map<TypeHandle, PStatCollector> CyclerTypeCounters;
  CyclerTypeCounters _all_cycler_types;
  CyclerTypeCounters _dirty_cycler_types;
  static void pstats_count_cycler_type(TypeHandle type, int count, void *data);
  static void pstats_count_dirty_cycler_type(TypeHandle type, int count, void *data);
#endif  // DO_PSTATS

  static const RenderState *get_invert_polygon_state();

  // The WindowRenderer class records the stages of the pipeline that each
  // thread (including the main thread, a.k.a.  "app") should process, and the
  // list of windows for each stage.

  // There is one WindowRenderer instance for app, and another instance for
  // each thread (the thread-specific WindowRenderers are actually instances
  // of RenderThread, below, which inherits from WindowRenderer).

  // The idea is that each window is associated with one or more
  // WindowRenderer objects, according to the threads in which its rendering
  // tasks (window, cull, and draw) are divided into.

  // The "window" task is responsible for doing any updates to the window
  // itself, such as size and placement, and is wholly responsible for any API
  // calls to the windowing system itself, unrelated to OpenGL-type calls.
  // This is normally done in app (the design of X-Windows is such that all X
  // calls must be issued in the same thread).

  // The "cull" task is responsible for crawling through the scene graph and
  // discovering all of the Geoms that are within the viewing frustum.  It
  // assembles all such Geoms, along with their computed net state and
  // transform, in a linked list of CullableObjects, which it stores for the
  // "draw" task, next.

  // The "draw" task is responsible for walking through the list of
  // CullableObjects recorded by the cull task, and issuing the appropriate
  // graphics commands to draw them.

  // There is an additional task, not often used, called "cdraw". This task,
  // if activated, will crawl through the scene graph and issue graphics
  // commands immediately, as each Geom is discovered.  It is only rarely used
  // because it cannot perform sorting beyond basic scene graph order, making
  // it less useful than a separate cull and draw task.

  // It is possible for all three of the normal tasks: window, cull, and draw,
  // to be handled by the same thread.  This is the normal, single-threaded
  // model: all tasks are handled by the app thread.  In this case, the window
  // will be added to _app's _window, _cull, and _draw lists.

/*
 * On the other hand, a window's tasks may also be distributed among as many
 * as three threads.  For instance, if the window is listed on _app's _window
 * list, but on thread A's _cull list, and thread B's _draw list, then the
 * window task will be handled in the app thread, while the cull task will be
 * handled by thread A, and the draw task will be handled (in parallel) by
 * thread B.  (In order for this to work, it will be necessary that thread A
 * and B are configured to view different stages of the graphics pipeline.
 * This is a more advanced topic than there is room to discuss in this
 * comment.)
 */

  // Manipulation of the various window lists in each WindowRenderer object is
  // always performed in the app thread.  The auxiliary threads are slaves to
  // the app thread, and they can only perform one of a handful of specified
  // tasks, none of which includes adding or removing windows from its lists.
  // The full set of tasks that a WindowRenderer may perform is enumerated in
  // ThreadState, above; see RenderThread::thread_main().

/*
 * There is a pair of condition variables for each thread, _cv_start and
 * _cv_done, that is used to synchronize requests made by app to a particular
 * thread.  The usual procedure to request a thread to perform a particular
 * task is the following: the app thread waits on the thread's _cv_done
 * variable, stores the value corresponding to the desired task in the
 * thread's _thread_state value, then signals the thread's _cv_start variable.
 * The thread, in turn, will perform its requested task, set its _thread_state
 * to TS_wait, and signal _cv_done.  See examples in the code, e.g.
 * open_windows(), for more details on this process.
 */

  // It is of course not necessary to signal any threads in order to perform
  // tasks listed in the _app WindowRenderer.  For this object only, we simply
  // call the appropriate methods on _app when we want the tasks to be
  // performed.

  class WindowRenderer {
  public:
    WindowRenderer(const std::string &name);

    void add_gsg(GraphicsStateGuardian *gsg);
    void add_window(Windows &wlist, GraphicsOutput *window);
    void remove_window(GraphicsOutput *window);
    void resort_windows();
    void do_frame(GraphicsEngine *engine, Thread *current_thread);
    void do_windows(GraphicsEngine *engine, Thread *current_thread);
    void do_ready_flip(GraphicsEngine *engine, Thread *current_thread);
    void do_flip(GraphicsEngine *engine, Thread *current_thread);
    void do_release(GraphicsEngine *engine, Thread *current_thread);
    void do_close(GraphicsEngine *engine, Thread *current_thread);
    void do_pending(GraphicsEngine *engine, Thread *current_thread);
    bool any_done_gsgs() const;

  public:
    Windows _cull;    // cull stage
    Windows _cdraw;   // cull-and-draw-together stage
    Windows _draw;    // draw stage
    Windows _window;  // window stage, i.e. process windowing events

    // These are not kept sorted.
    Windows _pending_close;    // moved from _window, pending close.

    GSGs _gsgs;       // draw stage

    LightReMutex _wl_lock;
  };

  class RenderThread : public Thread, public WindowRenderer {
  public:
    RenderThread(const std::string &name, GraphicsEngine *engine);
    virtual void thread_main();

    GraphicsEngine *_engine;
    Mutex _cv_mutex;
    ConditionVar _cv_start;
    ConditionVar _cv_done;
    ThreadState _thread_state;

    // These are stored for extract_texture_data and dispatch_compute.
    GraphicsStateGuardian *_gsg;
    PT(Texture) _texture;
    const RenderState *_state;
    DisplayRegion *_region;
    LVecBase3i _work_groups;
    bool _result;
  };

  WindowRenderer *get_window_renderer(const std::string &name, int pipeline_stage);

  Pipeline *_pipeline;
  Windows _windows;
  bool _windows_sorted;

  // This lock protects the next two fields.
  Mutex _new_windows_lock;
  unsigned int _window_sort_index;
  pvector<PT(GraphicsOutput)> _new_windows;

  WindowRenderer _app;
  typedef pmap<std::string, PT(RenderThread) > Threads;
  Threads _threads;
  GraphicsThreadingModel _threading_model;
  bool _auto_flip;
  bool _portal_enabled; //toggle to portal culling on/off
  PT(Loader) _default_loader;

  enum FlipState {
    FS_draw,  // Still drawing.
    FS_sync,  // All windows are done drawing.
    FS_flip,  // All windows are done drawing and have flipped.
  };
  FlipState _flip_state;

  bool _singular_warning_last_frame;
  bool _singular_warning_this_frame;

  ReMutex _lock;
  ReMutex _public_lock;

  class LoadedTexture {
  public:
    PT(Texture) _tex;
    UpdateSeq _image_modified;
  };
  typedef pvector<LoadedTexture> LoadedTextures;
  LoadedTextures _loaded_textures;
  Mutex _loaded_textures_lock;

  static PT(GraphicsEngine) _global_ptr;

  static PStatCollector _wait_pcollector;
  static PStatCollector _cycle_pcollector;
  static PStatCollector _app_pcollector;
  static PStatCollector _render_frame_pcollector;
  static PStatCollector _do_frame_pcollector;
  static PStatCollector _yield_pcollector;
  static PStatCollector _cull_pcollector;
  static PStatCollector _cull_setup_pcollector;
  static PStatCollector _cull_sort_pcollector;
  static PStatCollector _draw_pcollector;
  static PStatCollector _sync_pcollector;
  static PStatCollector _flip_pcollector;
  static PStatCollector _flip_begin_pcollector;
  static PStatCollector _flip_end_pcollector;
  static PStatCollector _transform_states_pcollector;
  static PStatCollector _transform_states_unused_pcollector;
  static PStatCollector _render_states_pcollector;
  static PStatCollector _render_states_unused_pcollector;
  static PStatCollector _cyclers_pcollector;
  static PStatCollector _dirty_cyclers_pcollector;
  static PStatCollector _delete_pcollector;

  static PStatCollector _sw_sprites_pcollector;
  static PStatCollector _vertex_data_small_pcollector;
  static PStatCollector _vertex_data_independent_pcollector;
  static PStatCollector _vertex_data_pending_pcollector;
  static PStatCollector _vertex_data_resident_pcollector;
  static PStatCollector _vertex_data_compressed_pcollector;
  static PStatCollector _vertex_data_used_disk_pcollector;
  static PStatCollector _vertex_data_unused_disk_pcollector;

  static PStatCollector _cnode_volume_pcollector;
  static PStatCollector _gnode_volume_pcollector;
  static PStatCollector _geom_volume_pcollector;
  static PStatCollector _node_volume_pcollector;
  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;
  static PStatCollector _volume_polygon_pcollector;
  static PStatCollector _test_polygon_pcollector;
  static PStatCollector _volume_plane_pcollector;
  static PStatCollector _test_plane_pcollector;
  static PStatCollector _volume_sphere_pcollector;
  static PStatCollector _test_sphere_pcollector;
  static PStatCollector _volume_box_pcollector;
  static PStatCollector _test_box_pcollector;
  static PStatCollector _volume_capsule_pcollector;
  static PStatCollector _test_capsule_pcollector;
  static PStatCollector _volume_inv_sphere_pcollector;
  static PStatCollector _test_inv_sphere_pcollector;
  static PStatCollector _volume_geom_pcollector;
  static PStatCollector _test_geom_pcollector;

  static PStatCollector _occlusion_untested_pcollector;
  static PStatCollector _occlusion_passed_pcollector;
  static PStatCollector _occlusion_failed_pcollector;
  static PStatCollector _occlusion_tests_pcollector;

  friend class WindowRenderer;
  friend class GraphicsOutput;
};

#include "graphicsEngine.I"

#endif
