// Filename: displayRegion.h
// Created by:  mike (09Jan97)
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

#ifndef DISPLAYREGION_H
#define DISPLAYREGION_H

#include "pandabase.h"

#include "drawableRegion.h"
#include "referenceCount.h"
#include "nodePath.h"
#include "cullResult.h"
#include "sceneSetup.h"
#include "pointerTo.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageWriter.h"
#include "pipelineCycler.h"
#include "config_display.h"
#include "lens.h"
#include "deletedChain.h"
#include "plist.h"
#include "pStatCollector.h"

class GraphicsOutput;
class GraphicsPipe;
class CullHandler;
class Camera;
class PNMImage;

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
class EXPCL_PANDA DisplayRegion : public ReferenceCount, public DrawableRegion {
protected:
  DisplayRegion(GraphicsOutput *window);
  DisplayRegion(GraphicsOutput *window,
                float l, float r, float b, float t);
private:
  DisplayRegion(const DisplayRegion &copy);
  void operator = (const DisplayRegion &copy);

public:
  virtual ~DisplayRegion();
  void cleanup();

  INLINE bool operator < (const DisplayRegion &other) const;

PUBLISHED:
  INLINE void get_dimensions(float &l, float &r, float &b, float &t) const;
  INLINE float get_left() const;
  INLINE float get_right() const;
  INLINE float get_bottom() const;
  INLINE float get_top() const;
  void set_dimensions(float l, float r, float b, float t);

  INLINE GraphicsOutput *get_window() const;
  GraphicsPipe *get_pipe() const;

  void set_camera(const NodePath &camera);
  INLINE NodePath get_camera(Thread *current_thread = Thread::get_current_thread()) const;

  void set_active(bool active);
  INLINE bool is_active() const;

  void set_sort(int sort);
  INLINE int get_sort() const;

  void set_stereo_channel(Lens::StereoChannel stereo_channel);
  INLINE Lens::StereoChannel get_stereo_channel();

  INLINE void set_clear_depth_between_eyes(bool clear_depth_between_eyes);
  INLINE bool get_clear_depth_between_eyes() const;

  INLINE void set_cube_map_index(int cube_map_index);
  INLINE int get_cube_map_index() const;

  void compute_pixels();
  void compute_pixels_all_stages();
  void compute_pixels(int x_size, int y_size);
  void compute_pixels_all_stages(int x_size, int y_size);
  INLINE void get_pixels(int &pl, int &pr, int &pb, int &pt) const;
  INLINE void get_region_pixels(int &xo, int &yo, int &w, int &h) const;
  INLINE void get_region_pixels_i(int &xo, int &yo, int &w, int &h) const;

  INLINE int get_pixel_width() const;
  INLINE int get_pixel_height() const;

  void output(ostream &out) const;

  static Filename make_screenshot_filename(
    const string &prefix = "screenshot");
  Filename save_screenshot_default(const string &prefix = "screenshot");
  bool save_screenshot(
    const Filename &filename, const string &image_comment = "");
  bool get_screenshot(PNMImage &image);

public:
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

  // The associated window is a permanent property of the
  // DisplayRegion.  It doesn't need to be cycled.
  GraphicsOutput *_window;
  bool _clear_depth_between_eyes;

  // This is the data that is associated with the DisplayRegion that
  // needs to be cycled every frame, but represents the parameters as
  // specified by the user, and which probably will not change that
  // often.
  class EXPCL_PANDA CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);

    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return DisplayRegion::get_class_type();
    }

    float _l;
    float _r;
    float _b;
    float _t;
    
    int _pl;
    int _pr;
    int _pb;
    int _pt;
    int _pbi;
    int _pti;
    
    NodePath _camera;
    Camera *_camera_node;
    
    bool _active;
    int _sort;
    Lens::StereoChannel _stereo_channel;
    int _cube_map_index;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageWriter<CData> CDStageWriter;

  // This is a special cycler created to hold the results from the
  // cull traversal, for (a) the draw traversal, and (b) the next
  // frame's cull traversal.  It needs to be cycled, but it gets its
  // own cycler because it will certainly change every frame, so we
  // don't need to lump all the heavy data above in with this
  // lightweight cycler.
  class EXPCL_PANDA CDataCull : public CycleData {
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
    ReferenceCount::init_type();
    register_type(_type_handle, "DisplayRegion",
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class GraphicsOutput;
  friend class DisplayRegionPipelineReader;
};

////////////////////////////////////////////////////////////////////
//       Class : DisplayRegionPipelineReader
// Description : Encapsulates the data from a DisplayRegion,
//               pre-fetched for one stage of the pipeline.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DisplayRegionPipelineReader {
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

  INLINE void get_dimensions(float &l, float &r, float &b, float &t) const;
  INLINE float get_left() const;
  INLINE float get_right() const;
  INLINE float get_bottom() const;
  INLINE float get_top() const;

  INLINE GraphicsOutput *get_window() const;
  GraphicsPipe *get_pipe() const;

  INLINE NodePath get_camera() const;
  INLINE bool is_active() const;
  INLINE int get_sort() const;
  INLINE Lens::StereoChannel get_stereo_channel();
  INLINE bool get_clear_depth_between_eyes() const;
  INLINE int get_cube_map_index() const;

  INLINE void get_pixels(int &pl, int &pr, int &pb, int &pt) const;
  INLINE void get_region_pixels(int &xo, int &yo, int &w, int &h) const;
  INLINE void get_region_pixels_i(int &xo, int &yo, int &w, int &h) const;

  INLINE int get_pixel_width() const;
  INLINE int get_pixel_height() const;

private:
  DisplayRegion *_object;
  Thread *_current_thread;
  const DisplayRegion::CData *_cdata;
};

INLINE ostream &operator << (ostream &out, const DisplayRegion &dr);

#include "displayRegion.I"

#endif /* DISPLAYREGION_H */
