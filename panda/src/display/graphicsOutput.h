/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsOutput.h
 * @author drose
 * @date 2004-02-06
 */

#ifndef GRAPHICSOUTPUT_H
#define GRAPHICSOUTPUT_H

#include "pandabase.h"

#include "graphicsPipe.h"
#include "displayRegion.h"
#include "stereoDisplayRegion.h"
#include "graphicsStateGuardian.h"
#include "drawableRegion.h"
#include "renderBuffer.h"
#include "graphicsOutputBase.h"
#include "luse.h"
#include "typedWritableReferenceCount.h"
#include "pandaNode.h"
#include "pStatCollector.h"
#include "pnotify.h"
#include "lightMutex.h"
#include "filename.h"
#include "drawMask.h"
#include "pvector.h"
#include "weakPointerTo.h"
#include "nodePath.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "updateSeq.h"
#include "asyncFuture.h"

class PNMImage;
class GraphicsEngine;

/**
 * This is a base class for the various different classes that represent the
 * result of a frame of rendering.  The most common kind of GraphicsOutput is
 * a GraphicsWindow, which is a real-time window on the desktop, but another
 * example is GraphicsBuffer, which is an offscreen buffer.
 *
 * The actual rendering, and anything associated with the graphics context
 * itself, is managed by the associated GraphicsStateGuardian (which might
 * output to multiple GraphicsOutput objects).
 *
 * GraphicsOutputs are not actually writable to bam files, of course, but they
 * may be passed as event parameters, so they inherit from
 * TypedWritableReferenceCount instead of TypedReferenceCount for that
 * convenience.
 */
class EXPCL_PANDA_DISPLAY GraphicsOutput : public GraphicsOutputBase, public DrawableRegion {
protected:
  GraphicsOutput(GraphicsEngine *engine,
                 GraphicsPipe *pipe,
                 const std::string &name,
                 const FrameBufferProperties &fb_prop,
                 const WindowProperties &win_prop, int flags,
                 GraphicsStateGuardian *gsg,
                 GraphicsOutput *host,
                 bool default_stereo_flags);
  GraphicsOutput(const GraphicsOutput &copy) = delete;
  GraphicsOutput &operator = (const GraphicsOutput &copy) = delete;

PUBLISHED:
  enum RenderTextureMode {
    RTM_none,

    // Try to render to the texture directly, but if that is not possible,
    // fall back to RTM_copy_texture.
    RTM_bind_or_copy,

    // Copy the image from the buffer to the texture every frame.
    RTM_copy_texture,

    // Copy the image from the buffer to system RAM every frame.
    RTM_copy_ram,

    // Copy the image from the buffer to the texture after a call to
    // trigger_copy().
    RTM_triggered_copy_texture,

    // Copy the image from the buffer to system RAM after a call to
    // trigger_copy().
    RTM_triggered_copy_ram,

    // Render directly to a layered texture, such as a cube map, 3D texture or
    // 2D texture array.  The layer that is being rendered to is selected by a
    // geometry shader.
    RTM_bind_layered,
  };

  // There are many reasons to call begin_frameend_frame.
  enum FrameMode {
    FM_render,   // We are rendering a frame.
    FM_parasite, // We are rendering a frame of a parasite.
    FM_refresh,  // We are just refreshing the display or exposing the window.
  };

  virtual ~GraphicsOutput();

  INLINE GraphicsStateGuardian *get_gsg() const;
  INLINE GraphicsPipe *get_pipe() const;
  INLINE GraphicsEngine *get_engine() const;
  INLINE const std::string &get_name() const;
  MAKE_PROPERTY(gsg, get_gsg);
  MAKE_PROPERTY(pipe, get_pipe);
  MAKE_PROPERTY(engine, get_engine);
  MAKE_PROPERTY(name, get_name);

  INLINE int count_textures() const;
  INLINE bool has_texture() const;
  virtual INLINE Texture *get_texture(int i=0) const;
  INLINE RenderTexturePlane get_texture_plane(int i=0) const;
  INLINE RenderTextureMode get_rtm_mode(int i=0) const;
  void clear_render_textures();
  void add_render_texture(Texture *tex, RenderTextureMode mode,
                          RenderTexturePlane bitplane=RTP_COUNT);
  void setup_render_texture(Texture *tex, bool allow_bind, bool to_ram);

  INLINE const LVecBase2i &get_size() const;
  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE LVecBase2i get_fb_size() const;
  INLINE int get_fb_x_size() const;
  INLINE int get_fb_y_size() const;
  INLINE LVecBase2i get_sbs_left_size() const;
  INLINE int get_sbs_left_x_size() const;
  INLINE int get_sbs_left_y_size() const;
  INLINE LVecBase2i get_sbs_right_size() const;
  INLINE int get_sbs_right_x_size() const;
  INLINE int get_sbs_right_y_size() const;
  INLINE bool has_size() const;
  INLINE bool is_valid() const;
  INLINE bool is_nonzero_size() const;

  MAKE_PROPERTY(size, get_size);
  MAKE_PROPERTY(fb_size, get_fb_size);
  MAKE_PROPERTY(sbs_left_size, get_sbs_left_size);
  MAKE_PROPERTY(sbs_right_size, get_sbs_right_size);

  void set_active(bool active);
  virtual bool is_active() const;
  MAKE_PROPERTY(active, is_active, set_active);

  void set_one_shot(bool one_shot);
  bool get_one_shot() const;
  MAKE_PROPERTY(one_shot, get_one_shot, set_one_shot);

  void set_inverted(bool inverted);
  INLINE bool get_inverted() const;
  MAKE_PROPERTY(inverted, get_inverted, set_inverted);

  INLINE void set_swap_eyes(bool swap_eyes);
  INLINE bool get_swap_eyes() const;
  MAKE_PROPERTY(swap_eyes, get_swap_eyes, set_swap_eyes);

  INLINE void set_red_blue_stereo(bool red_blue_stereo,
                                  unsigned int left_eye_color_mask,
                                  unsigned int right_eye_color_mask);
  INLINE bool get_red_blue_stereo() const;
  INLINE unsigned int get_left_eye_color_mask() const;
  INLINE unsigned int get_right_eye_color_mask() const;

  void set_side_by_side_stereo(bool side_by_side_stereo);
  void set_side_by_side_stereo(bool side_by_side_stereo,
                               const LVecBase4 &sbs_left_dimensions,
                               const LVecBase4 &sbs_right_dimensions);
  INLINE bool get_side_by_side_stereo() const;
  INLINE const LVecBase4 &get_sbs_left_dimensions() const;
  INLINE const LVecBase4 &get_sbs_right_dimensions() const;

  INLINE const FrameBufferProperties &get_fb_properties() const;
  INLINE bool is_stereo() const;

  INLINE void clear_delete_flag();
  bool get_delete_flag() const;

  virtual void set_sort(int sort);
  INLINE int get_sort() const;
  MAKE_PROPERTY(sort, get_sort, set_sort);

  INLINE void set_child_sort(int child_sort);
  INLINE void clear_child_sort();
  INLINE int get_child_sort() const;
  MAKE_PROPERTY(child_sort, get_child_sort, set_child_sort);

  INLINE AsyncFuture *trigger_copy();

  INLINE DisplayRegion *make_display_region();
  INLINE DisplayRegion *make_display_region(PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t);
  DisplayRegion *make_display_region(const LVecBase4 &dimensions);
  INLINE DisplayRegion *make_mono_display_region();
  INLINE DisplayRegion *make_mono_display_region(PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t);
  DisplayRegion *make_mono_display_region(const LVecBase4 &dimensions);
  INLINE StereoDisplayRegion *make_stereo_display_region();
  INLINE StereoDisplayRegion *make_stereo_display_region(PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t);
  StereoDisplayRegion *make_stereo_display_region(const LVecBase4 &dimensions);
  bool remove_display_region(DisplayRegion *display_region);
  void remove_all_display_regions();

  INLINE DisplayRegion *get_overlay_display_region() const;
  void set_overlay_display_region(DisplayRegion *display_region);

  int get_num_display_regions() const;
  PT(DisplayRegion) get_display_region(int n) const;
  MAKE_SEQ(get_display_regions, get_num_display_regions, get_display_region);
  MAKE_SEQ_PROPERTY(display_regions, get_num_display_regions, get_display_region);

  int get_num_active_display_regions() const;
  PT(DisplayRegion) get_active_display_region(int n) const;
  MAKE_SEQ(get_active_display_regions, get_num_active_display_regions, get_active_display_region);
  MAKE_SEQ_PROPERTY(active_display_regions, get_num_active_display_regions, get_active_display_region);

  GraphicsOutput *make_texture_buffer(
      const std::string &name, int x_size, int y_size,
      Texture *tex = nullptr, bool to_ram = false, FrameBufferProperties *fbp = nullptr);
  GraphicsOutput *make_cube_map(const std::string &name, int size,
                                NodePath &camera_rig,
                                DrawMask camera_mask = PandaNode::get_all_camera_mask(),
                                bool to_ram = false, FrameBufferProperties *fbp = nullptr);

  INLINE static Filename make_screenshot_filename(
      const std::string &prefix = "screenshot");
  INLINE Filename save_screenshot_default(
      const std::string &prefix = "screenshot");
  INLINE bool save_screenshot(
      const Filename &filename, const std::string &image_comment = "");
  INLINE bool get_screenshot(PNMImage &image);
  INLINE PT(Texture) get_screenshot();

  NodePath get_texture_card();

  virtual bool share_depth_buffer(GraphicsOutput *graphics_output);
  virtual void unshare_depth_buffer();

  virtual bool get_supports_render_texture() const;
  MAKE_PROPERTY(supports_render_texture, get_supports_render_texture);

PUBLISHED:
  // These are not intended to be called directly by the user, but they're
  // published anyway since they might occasionally be useful for low-level
  // debugging.
  virtual bool flip_ready() const;
  virtual GraphicsOutput *get_host();

public:
  INLINE bool operator < (const GraphicsOutput &other) const;

  virtual void request_open();
  virtual void request_close();

  virtual void set_close_now();
  virtual void reset_window(bool swapchain);
  virtual void clear_pipe();

  void set_size_and_recalc(int x, int y);

  // It is an error to call any of the following methods from any thread other
  // than the draw thread.  These methods are normally called by the
  // GraphicsEngine.
  virtual void clear(Thread *current_thread);
  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  void change_scenes(DisplayRegionPipelineReader *new_dr);
  virtual void select_target_tex_page(int page);

  // These methods will be called within the app (main) thread.
  virtual void begin_flip();
  virtual void ready_flip();
  virtual void end_flip();

  // It is an error to call any of the following methods from any thread other
  // than the window thread.  These methods are normally called by the
  // GraphicsEngine.
  virtual void process_events();

  INLINE PStatCollector &get_cull_window_pcollector();
  INLINE PStatCollector &get_draw_window_pcollector();
  INLINE PStatCollector &get_clear_window_pcollector();

protected:
  virtual void pixel_factor_changed();
  void prepare_for_deletion();
  void promote_to_copy_texture();
  bool copy_to_textures();

  INLINE void begin_frame_spam(FrameMode mode);
  INLINE void end_frame_spam(FrameMode mode);
  INLINE void clear_cube_map_selection();
  INLINE void trigger_flip();

  class CData;

private:
  PT(GeomVertexData) create_texture_card_vdata(int x, int y);

  DisplayRegion *add_display_region(DisplayRegion *display_region);
  bool do_remove_display_region(DisplayRegion *display_region);

  INLINE void win_display_regions_changed();

  INLINE void determine_display_regions() const;
  void do_determine_display_regions(CData *cdata);

  static unsigned int parse_color_mask(const std::string &word);

protected:
  PT(GraphicsStateGuardian) _gsg;
  GraphicsEngine *_engine;
  PT(GraphicsPipe) _pipe;
  PT(GraphicsOutput) _host;
  FrameBufferProperties _fb_properties;
  bool _stereo;
  std::string _name;
  bool _flip_ready;
  int _target_tex_page;
  int _target_tex_view;
  DisplayRegion *_prev_page_dr;
  PT(GeomNode) _texture_card;
  PT(AsyncFuture) _trigger_copy;

  class RenderTexture {
  public:
    PT(Texture) _texture;
    RenderTexturePlane _plane;
    RenderTextureMode _rtm_mode;
  };
  typedef pvector<RenderTexture> RenderTextures;

private:
  int _sort;
  int _child_sort;
  bool _got_child_sort;
  unsigned int _internal_sort_index;

protected:
  bool _inverted;
  bool _swap_eyes;
  bool _red_blue_stereo;
  unsigned int _left_eye_color_mask;
  unsigned int _right_eye_color_mask;
  bool _side_by_side_stereo;
  LVecBase4 _sbs_left_dimensions;
  LVecBase4 _sbs_right_dimensions;
  bool _delete_flag;

  // These weak pointers are used to keep track of whether the buffer's bound
  // Textures have been deleted or not.  Until they have, we don't auto-close
  // the buffer (since that would deallocate the memory associated with the
  // texture).
  pvector<WPT(Texture)> _hold_textures;

protected:
  LightMutex _lock;
  // protects _display_regions.
  PT(DisplayRegion) _overlay_display_region;
  typedef pvector< PT(DisplayRegion) > TotalDisplayRegions;
  TotalDisplayRegions _total_display_regions;
  typedef pvector<DisplayRegion *> ActiveDisplayRegions;

  // This is the data that is associated with the GraphicsOutput that needs to
  // be cycled every frame.  Mostly we don't cycle this data, but we do cycle
  // the textures list, and the active flag.
  class EXPCL_PANDA_DISPLAY CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);

    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return GraphicsOutput::get_class_type();
    }

    RenderTextures _textures;
    UpdateSeq _textures_seq;
    bool _active;
    int _one_shot_frame;
    ActiveDisplayRegions _active_display_regions;
    bool _active_display_regions_stale;
  };
  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageWriter<CData> CDStageWriter;

protected:
  int _creation_flags;
  LVecBase2i _size;
  bool _has_size;
  bool _is_valid;
  bool _is_nonzero_size;

  static PStatCollector _make_current_pcollector;
  static PStatCollector _copy_texture_pcollector;
  static PStatCollector _cull_pcollector;
  static PStatCollector _draw_pcollector;
  PStatCollector _cull_window_pcollector;
  PStatCollector _draw_window_pcollector;
  PStatCollector _clear_window_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsOutputBase::init_type();
    register_type(_type_handle, "GraphicsOutput",
                  GraphicsOutputBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsPipe;
  friend class GraphicsEngine;
  friend class DisplayRegion;
};

EXPCL_PANDA_DISPLAY std::ostream &operator << (std::ostream &out, GraphicsOutput::FrameMode mode);

#include "graphicsOutput.I"

#endif /* GRAPHICSOUTPUT_H */
