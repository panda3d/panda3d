// Filename: graphicsStateGuardian.h
// Created by:  drose (02Feb99)
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

#ifndef GRAPHICSSTATEGUARDIAN_H
#define GRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "savedFrameBuffer.h"
#include "frameBufferProperties.h"
#include "displayRegionStack.h"
#include "lensStack.h"
#include "preparedGraphicsObjects.h"

#include "graphicsStateGuardianBase.h"
#include "graphicsThreadingModel.h"
#include "graphicsPipe.h"
#include "sceneSetup.h"
#include "luse.h"
#include "coordinateSystem.h"
#include "factory.h"
#include "pStatCollector.h"
#include "transformState.h"
#include "renderState.h"
#include "light.h"
#include "planeNode.h"
#include "colorWriteAttrib.h"
#include "colorBlendAttrib.h"
#include "textureAttrib.h"
#include "transparencyAttrib.h"
#include "config_display.h"

#include "notify.h"
#include "pvector.h"

class DrawableRegion;
class GraphicsEngine;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsStateGuardian
// Description : Encapsulates all the communication with a particular
//               instance of a given rendering backend.  Tries to
//               guarantee that redundant state-change requests are
//               not issued (hence "state guardian").
//
//               There will be one of these objects for each different
//               graphics context active in the system.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsStateGuardian : public GraphicsStateGuardianBase {
  //
  // Interfaces all GSGs should have
  //
public:
  GraphicsStateGuardian(const FrameBufferProperties &properties,
                        CoordinateSystem internal_coordinate_system);
  virtual ~GraphicsStateGuardian();

PUBLISHED:
  INLINE int release_all_textures();
  INLINE int release_all_geoms();

  INLINE void set_active(bool active);
  INLINE bool is_active() const;

  INLINE const FrameBufferProperties &get_properties() const;
  INLINE GraphicsPipe *get_pipe() const;
  INLINE GraphicsEngine *get_engine() const;
  INLINE const GraphicsThreadingModel &get_threading_model() const;

  INLINE int get_max_texture_stages() const;
  INLINE bool get_copy_texture_inverted() const;
  virtual bool get_supports_multisample() const;
  INLINE bool get_supports_generate_mipmap() const;
  INLINE bool get_supports_render_texture() const;

public:
  INLINE bool set_scene(SceneSetup *scene_setup);
  INLINE SceneSetup *get_scene() const;

  virtual PreparedGraphicsObjects *get_prepared_objects();

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void apply_texture(TextureContext *tc);
  virtual void release_texture(TextureContext *tc);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  virtual void set_color_clear_value(const Colorf &value);
  virtual void set_depth_clear_value(const float value);
  virtual void do_clear(const RenderBuffer &buffer)=0;

  void clear(DrawableRegion *clearable);
  INLINE void clear(DisplayRegion *dr);

  virtual void prepare_display_region()=0;
  virtual bool prepare_lens();

  INLINE int force_normals();
  INLINE int undo_force_normals();

  virtual bool begin_frame();
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame();

  // These functions will be queried by the GeomIssuer to determine if
  // it should issue normals, texcoords, and/or colors, based on the
  // GSG's current state.
  virtual bool wants_normals() const;
  virtual bool wants_texcoords() const;
  virtual bool wants_colors() const;

  virtual bool depth_offset_decals();
  virtual CPT(RenderState) begin_decal_base_first();
  virtual CPT(RenderState) begin_decal_nested();
  virtual CPT(RenderState) begin_decal_base_second();
  virtual void finish_decal();

  virtual bool framebuffer_bind_to_texture(GraphicsOutput *win, Texture *tex);
  virtual void framebuffer_release_texture(GraphicsOutput *win, Texture *tex);

  INLINE bool reset_if_new();
  virtual void reset();

  INLINE void modify_state(const RenderState *state);
  INLINE void set_state(const RenderState *state);
  INLINE void set_transform(const TransformState *transform);

  RenderBuffer get_render_buffer(int buffer_type);

  INLINE const DisplayRegion *get_current_display_region(void) const;
  INLINE const Lens *get_current_lens() const;

  INLINE DisplayRegionStack push_display_region(const DisplayRegion *dr);
  INLINE void pop_display_region(DisplayRegionStack &node);

  void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;
  INLINE CoordinateSystem get_internal_coordinate_system() const;

  INLINE const TransformState *get_cs_transform() const;

  virtual void issue_transform(const TransformState *transform);
  virtual void issue_color_scale(const ColorScaleAttrib *attrib);
  virtual void issue_color(const ColorAttrib *attrib);
  virtual void issue_light(const LightAttrib *attrib);
  virtual void issue_color_write(const ColorWriteAttrib *attrib);
  virtual void issue_transparency(const TransparencyAttrib *attrib);
  virtual void issue_color_blend(const ColorBlendAttrib *attrib);
  virtual void issue_texture(const TextureAttrib *attrib);
  virtual void issue_clip_plane(const ClipPlaneAttrib *attrib);

  virtual void bind_light(PointLight *light_obj, const NodePath &light, 
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light,
                          int light_id);

protected:
  INLINE NodePath get_light(int light_id) const;
  virtual bool slot_new_light(int light_id);
  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const Colorf &color);
  virtual void enable_light(int light_id, bool enable);
  virtual void begin_bind_lights();
  virtual void end_bind_lights();

  INLINE PlaneNode *get_clip_plane(int plane_id) const;
  virtual bool slot_new_clip_plane(int plane_id);
  virtual void enable_clip_planes(bool enable);
  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void begin_bind_clip_planes();
  virtual void bind_clip_plane(PlaneNode *plane, int pane_id);
  virtual void end_bind_clip_planes();

  virtual void set_blend_mode();

  virtual void finish_modify_state();

  virtual void free_pointers();
  virtual void close_gsg();
  void panic_deactivate();

  INLINE void set_properties(const FrameBufferProperties &properties);

#ifdef DO_PSTATS
  // These functions are used to update the active texture memory
  // usage record (and other frame-based measurements) in Pstats.
  void init_frame_pstats();
  void add_to_texture_record(TextureContext *tc);
  void add_to_geom_record(GeomContext *gc);

  pset<TextureContext *> _current_textures;
  pset<GeomContext *> _current_geoms;
#else
  INLINE void init_frame_pstats() { }
  INLINE void add_to_texture_record(TextureContext *) { }
  INLINE void add_to_geom_record(GeomContext *) { }
  INLINE void record_state_change(TypeHandle) { }
#endif

  static CPT(RenderState) get_unlit_state();
  static CPT(RenderState) get_unclipped_state();
  static CPT(RenderState) get_untextured_state();

protected:
  PT(SceneSetup) _scene_setup;

  CPT(RenderState) _state;
  CPT(TransformState) _transform;

  int _buffer_mask;
  Colorf _color_clear_value;
  float _depth_clear_value;
  bool _stencil_clear_value;
  Colorf _accum_clear_value;

  int _display_region_stack_level;
  int _frame_buffer_stack_level;
  int _lens_stack_level;

  CPT(DisplayRegion) _current_display_region;
  CPT(Lens) _current_lens;

  // This is used by wants_normals().  It's used as a semaphore:
  // increment it to enable normals, and decrement it when you're
  // done.  The graphics engine will apply normals if it is nonzero.
  int _force_normals;

  CoordinateSystem _coordinate_system;
  CoordinateSystem _internal_coordinate_system;
  CPT(TransformState) _cs_transform;

  Colorf _scene_graph_color;
  bool _has_scene_graph_color;
  bool _scene_graph_color_stale;
  bool _color_blend_involves_color_scale;
  bool _texture_involves_color_scale;
  bool _vertex_colors_enabled;
  bool _lighting_enabled;
  bool _clip_planes_enabled;

  bool _color_scale_enabled;
  LVecBase4f _current_color_scale;

  ColorWriteAttrib::Mode _color_write_mode;
  ColorBlendAttrib::Mode _color_blend_mode;
  TransparencyAttrib::Mode _transparency_mode;
  CPT(ColorBlendAttrib) _color_blend;
  bool _blend_mode_stale;

  CPT(TextureAttrib) _pending_texture;
  bool _texture_stale;

  bool _needs_reset;
  bool _closing_gsg;
  bool _active;

  PT(PreparedGraphicsObjects) _prepared_objects;
  int _max_texture_stages;
  bool _copy_texture_inverted;
  bool _supports_multisample;
  bool _supports_generate_mipmap;
  bool _supports_render_texture;

public:
  // Statistics
  static PStatCollector _total_texusage_pcollector;
  static PStatCollector _active_texusage_pcollector;
  static PStatCollector _total_geom_pcollector;
  static PStatCollector _active_geom_pcollector;
  static PStatCollector _total_geom_node_pcollector;
  static PStatCollector _active_geom_node_pcollector;
  static PStatCollector _total_texmem_pcollector;
  static PStatCollector _used_texmem_pcollector;
  static PStatCollector _texmgrmem_total_pcollector;
  static PStatCollector _texmgrmem_resident_pcollector;
  static PStatCollector _vertices_tristrip_pcollector;
  static PStatCollector _vertices_trifan_pcollector;
  static PStatCollector _vertices_tri_pcollector;
  static PStatCollector _vertices_other_pcollector; 
  static PStatCollector _vertices_indexed_tristrip_pcollector;
  static PStatCollector _state_pcollector;
  static PStatCollector _transform_state_pcollector;
  static PStatCollector _texture_state_pcollector;
  static PStatCollector _draw_primitive_pcollector;
  static PStatCollector _clear_pcollector;
  static PStatCollector _flush_pcollector;

private:
  class LightInfo {
  public:
    INLINE LightInfo();
    NodePath _light;
    bool _enabled;
    bool _next_enabled;
  };

  pvector<LightInfo> _light_info;
  bool _lighting_enabled_this_frame;

  class ClipPlaneInfo {
  public:
    INLINE ClipPlaneInfo();
    PT(PlaneNode) _plane;
    bool _enabled;
    bool _next_enabled;
  };

  pvector<ClipPlaneInfo> _clip_plane_info;
  bool _clip_planes_enabled_this_frame;

  FrameBufferProperties _properties;
  PT(GraphicsPipe) _pipe;
  GraphicsEngine *_engine;
  GraphicsThreadingModel _threading_model;

public:
  void traverse_prepared_textures(bool (*pertex_callbackfn)(TextureContext *,void *),void *callback_arg);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    GraphicsStateGuardianBase::init_type();
    register_type(_type_handle, "GraphicsStateGuardian",
                  GraphicsStateGuardianBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsPipe;
  friend class GraphicsWindow;
  friend class GraphicsEngine;
};

#include "graphicsStateGuardian.I"

#endif
