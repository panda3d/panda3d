// Filename: displayRegion.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef DISPLAYREGION_H
#define DISPLAYREGION_H

#include "pandabase.h"

#include "displayRegionBase.h"
#include "drawableRegion.h"
#include "referenceCount.h"
#include "nodePath.h"
#include "cullResult.h"
#include "sceneSetup.h"
#include "pointerTo.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageWriter.h"
#include "pipelineCycler.h"
#include "config_display.h"
#include "lens.h"
#include "deletedChain.h"
#include "plist.h"
#include "pStatCollector.h"
#include "cullTraverser.h"
#include "callbackObject.h"
#include "luse.h"

class GraphicsOutput;
class GraphicsPipe;
class CullHandler;
class Camera;
class PNMImage;
class CullTraverser;

////////////////////////////////////////////////////////////////////
//       Class : DisplayRegion
// Description : A rectangular subregion within a window for rendering
//               into.  Typically, there is one DisplayRegion that
//               covers the whole window, but you may also create
//               smaller DisplayRegions for having different regions
//               within the window that represent different scenes.
//               You may also stack up DisplayRegions like panes of
//               glass, usually for layering 2-d interfaces on top of
//               a 3-d scene.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY DisplayRegion : public DisplayRegionBase, public DrawableRegion {
protected:
  DisplayRegion(GraphicsOutput *window, const LVecBase4 &dimensions);

private:
  DisplayRegion(const DisplayRegion &copy);
  void operator = (const DisplayRegion &copy);

public:
  virtual ~DisplayRegion();
  void cleanup();

  INLINE bool operator < (const DisplayRegion &other) const;

PUBLISHED:
  INLINE void get_dimensions(PN_stdfloat &l, PN_stdfloat &r, PN_stdfloat &b, PN_stdfloat &t) const;
  INLINE LVecBase4 get_dimensions() const;
  INLINE PN_stdfloat get_left() const;
  INLINE PN_stdfloat get_right() const;
  INLINE PN_stdfloat get_bottom() const;
  INLINE PN_stdfloat get_top() const;
  INLINE void set_dimensions(PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t);
  virtual void set_dimensions(const LVecBase4 &dimensions);

  INLINE GraphicsOutput *get_window() const;
  GraphicsPipe *get_pipe() const;
  virtual bool is_stereo() const;

  virtual void set_camera(const NodePath &camera);
  INLINE NodePath get_camera(Thread *current_thread = Thread::get_current_thread()) const;

  virtual void set_active(bool active);
  INLINE bool is_active() const;

  virtual void set_sort(int sort);
  INLINE int get_sort() const;

  virtual void set_stereo_channel(Lens::StereoChannel stereo_channel);
  INLINE Lens::StereoChannel get_stereo_channel() const;
  virtual void set_tex_view_offset(int tex_view_offset);
  INLINE int get_tex_view_offset() const;

  virtual void set_incomplete_render(bool incomplete_render);
  INLINE bool get_incomplete_render() const;

  virtual void set_texture_reload_priority(int texture_reload_priority);
  INLINE int get_texture_reload_priority() const;

  void set_lens_index(int index);
  INLINE int get_lens_index() const;

  virtual void set_cull_traverser(CullTraverser *trav);
  CullTraverser *get_cull_traverser();

  INLINE void set_cube_map_index(int cube_map_index);
  virtual void set_target_tex_page(int page);
  INLINE int get_target_tex_page() const;

  INLINE void set_cull_callback(CallbackObject *object);
  INLINE void clear_cull_callback();
  INLINE CallbackObject *get_cull_callback() const;

  INLINE void set_draw_callback(CallbackObject *object);
  INLINE void clear_draw_callback();
  INLINE CallbackObject *get_draw_callback() const;

  INLINE int get_pixel_width() const;
  INLINE int get_pixel_height() const;

  virtual void output(ostream &out) const;

  static Filename make_screenshot_filename(
    const string &prefix = "screenshot");
  Filename save_screenshot_default(const string &prefix = "screenshot");
  bool save_screenshot(
    const Filename &filename, const string &image_comment = "");
  bool get_screenshot(PNMImage &image);
  PT(Texture) get_screenshot();

  virtual PT(PandaNode) make_cull_result_graph();

public:
  void compute_pixels();
  void compute_pixels_all_stages();
  void compute_pixels(int x_size, int y_size);
  void compute_pixels_all_stages(int x_size, int y_size);
  INLINE void get_pixels(int &pl, int &pr, int &pb, int &pt) const;
  INLINE void get_region_pixels(int &xo, int &yo, int &w, int &h) const;
  INLINE void get_region_pixels_i(int &xo, int &yo, int &w, int &h) const;

  virtual bool supports_pixel_zoom() const;

  INLINE void set_cull_result(CullResult *cull_result, SceneSetup *scene_setup,
                              Thread *current_thread);
  INLINE CullResult *get_cull_result(Thread *current_thread) const;
  INLINE SceneSetup *get_scene_setup(Thread *current_thread) const;

  INLINE PStatCollector &get_cull_region_pcollector();
  INLINE PStatCollector &get_draw_region_pcollector();

private:
  class CData;

  void win_display_regions_changed();
  void do_compute_pixels(int x_size, int y_size, CData *cdata);
  void set_active_index(int index);

protected:
  virtual void do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
                       GraphicsStateGuardian *gsg, Thread *current_thread);

protected:
  // The associated window is a permanent property of the
  // DisplayRegion.  It doesn't need to be cycled.
  GraphicsOutput *_window;

  bool _incomplete_render;
  int _texture_reload_priority;

  // Ditto for the cull traverser.
  PT(CullTraverser) _trav;

private:
  // This is the data that is associated with the DisplayRegion that
  // needs to be cycled every frame, but represents the parameters as
  // specified by the user, and which probably will not change that
  // often.
  class EXPCL_PANDA_DISPLAY CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);

    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return DisplayRegion::get_class_type();
    }

    LVecBase4 _dimensions;  // left, right, bottom, top
    
    int _pl;
    int _pr;
    int _pb;
    int _pt;
    int _pbi;
    int _pti;
    int _lens_index; // index into which lens of a camera is associated with this display region.  0 is default
    
    NodePath _camera;
    Camera *_camera_node;
    
    bool _active;
    int _sort;
    Lens::StereoChannel _stereo_channel;
    int _tex_view_offset;
    int _target_tex_page;

    PT(CallbackObject) _cull_callback;
    PT(CallbackObject) _draw_callback;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageWriter<CData> CDStageWriter;

  // This is a special cycler created to hold the results from the
  // cull traversal, for (a) the draw traversal, and (b) the next
  // frame's cull traversal.  It needs to be cycled, but it gets its
  // own cycler because it will certainly change every frame, so we
  // don't need to lump all the heavy data above in with this
  // lightweight cycler.
  class EXPCL_PANDA_DISPLAY CDataCull : public CycleData {
  public:
    CDataCull();
    CDataCull(const CDataCull &copy);

    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return DisplayRegion::get_class_type();
    }

    PT(CullResult) _cull_result;
    PT(SceneSetup) _scene_setup;
  };
  PipelineCycler<CDataCull> _cycler_cull;
  typedef CycleDataReader<CDataCull> CDCullReader;
  typedef CycleDataWriter<CDataCull> CDCullWriter;

  PStatCollector _cull_region_pcollector;
  PStatCollector _draw_region_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DisplayRegionBase::init_type();
    register_type(_type_handle, "DisplayRegion",
                  DisplayRegionBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsEngine;
  friend class GraphicsOutput;
  friend class DisplayRegionCullCallbackData;
  friend class DisplayRegionPipelineReader;
};

////////////////////////////////////////////////////////////////////
//       Class : DisplayRegionPipelineReader
// Description : Encapsulates the data from a DisplayRegion,
//               pre-fetched for one stage of the pipeline.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY DisplayRegionPipelineReader {
public:
  INLINE DisplayRegionPipelineReader(DisplayRegion *object, Thread *current_thread);
private:
  INLINE DisplayRegionPipelineReader(const DisplayRegionPipelineReader &copy);
  INLINE void operator = (const DisplayRegionPipelineReader &copy);

public:
  INLINE ~DisplayRegionPipelineReader();
  ALLOC_DELETED_CHAIN(DisplayRegionPipelineReader);

  INLINE DisplayRegion *get_object() const;
  INLINE Thread *get_current_thread() const;

  INLINE bool is_any_clear_active() const;

  INLINE void get_dimensions(PN_stdfloat &l, PN_stdfloat &r, PN_stdfloat &b, PN_stdfloat &t) const;
  INLINE const LVecBase4 &get_dimensions() const;
  INLINE PN_stdfloat get_left() const;
  INLINE PN_stdfloat get_right() const;
  INLINE PN_stdfloat get_bottom() const;
  INLINE PN_stdfloat get_top() const;

  INLINE GraphicsOutput *get_window() const;
  GraphicsPipe *get_pipe() const;

  INLINE NodePath get_camera() const;
  INLINE bool is_active() const;
  INLINE int get_sort() const;
  INLINE Lens::StereoChannel get_stereo_channel() const;
  INLINE int get_tex_view_offset();
  INLINE bool get_clear_depth_between_eyes() const;
  INLINE int get_target_tex_page() const;
  INLINE CallbackObject *get_draw_callback() const;

  INLINE void get_pixels(int &pl, int &pr, int &pb, int &pt) const;
  INLINE void get_region_pixels(int &xo, int &yo, int &w, int &h) const;
  INLINE void get_region_pixels_i(int &xo, int &yo, int &w, int &h) const;

  INLINE int get_pixel_width() const;
  INLINE int get_pixel_height() const;

  INLINE int get_lens_index() const;

private:
  DisplayRegion *_object;
  Thread *_current_thread;
  const DisplayRegion::CData *_cdata;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "DisplayRegionPipelineReader");
  }

private:
  static TypeHandle _type_handle;
};

#include "displayRegion.I"

#endif /* DISPLAYREGION_H */
