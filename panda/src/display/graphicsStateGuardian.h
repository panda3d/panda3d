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
#include "config_display.h"
#include "geomMunger.h"
#include "geomVertexData.h"
#include "notify.h"
#include "pvector.h"
#include "attribSlots.h"

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
  INLINE void release_all();
  INLINE int release_all_textures();
  INLINE int release_all_geoms();
  INLINE int release_all_vertex_buffers();
  INLINE int release_all_index_buffers();

  INLINE void set_active(bool active);
  INLINE bool is_active() const;
  INLINE bool is_valid() const;

  INLINE const FrameBufferProperties &get_properties() const;
  INLINE GraphicsPipe *get_pipe() const;
  INLINE GraphicsEngine *get_engine() const;
  INLINE const GraphicsThreadingModel &get_threading_model() const;

  INLINE bool prefers_triangle_strips() const;
  INLINE int get_max_vertices_per_array() const;
  INLINE int get_max_vertices_per_primitive() const;

  INLINE int get_max_texture_stages() const;
  INLINE int get_max_texture_dimension() const;
  INLINE int get_max_3d_texture_dimension() const;
  INLINE int get_max_cube_map_dimension() const;

  INLINE bool get_supports_texture_combine() const;
  INLINE bool get_supports_texture_saved_result() const;
  INLINE bool get_supports_texture_dot3() const;

  INLINE bool get_supports_3d_texture() const;
  INLINE bool get_supports_cube_map() const;

  INLINE int get_max_lights() const;
  INLINE int get_max_clip_planes() const;

  INLINE int get_max_vertex_transforms() const;
  INLINE int get_max_vertex_transform_indices() const;

  INLINE bool get_copy_texture_inverted() const;
  virtual bool get_supports_multisample() const;
  INLINE bool get_supports_generate_mipmap() const;
  INLINE bool get_supports_render_texture() const;
  INLINE bool get_supports_depth_texture() const;
  INLINE bool get_supports_shadow_filter() const;
  INLINE bool get_supports_basic_shaders() const;
  
  virtual int get_supported_geom_rendering() const;

  INLINE bool get_color_scale_via_lighting() const;
  
  void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;
  virtual CoordinateSystem get_internal_coordinate_system() const;

  INLINE void make_global_gsg();
  INLINE static GraphicsStateGuardian *get_global_gsg();

public:
  INLINE bool set_scene(SceneSetup *scene_setup);
  INLINE SceneSetup *get_scene() const;

  virtual PreparedGraphicsObjects *get_prepared_objects();

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void release_texture(TextureContext *tc);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);

  virtual ShaderContext *prepare_shader(ShaderExpansion *shader);
  virtual void release_shader(ShaderContext *sc);
  
  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data);
  virtual void release_vertex_buffer(VertexBufferContext *vbc);

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data);
  virtual void release_index_buffer(IndexBufferContext *ibc);

  PT(GeomMunger) get_geom_munger(const RenderState *state);
  virtual PT(GeomMunger) make_geom_munger(const RenderState *state);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  virtual float compute_distance_to(const LPoint3f &point) const;

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

  virtual bool depth_offset_decals();
  virtual CPT(RenderState) begin_decal_base_first();
  virtual CPT(RenderState) begin_decal_nested();
  virtual CPT(RenderState) begin_decal_base_second();
  virtual void finish_decal();

  virtual bool begin_draw_primitives(const Geom *geom, 
                                     const GeomMunger *munger,
                                     const GeomVertexData *vertex_data);
  virtual void draw_triangles(const GeomTriangles *primitive);
  virtual void draw_tristrips(const GeomTristrips *primitive);
  virtual void draw_trifans(const GeomTrifans *primitive);
  virtual void draw_lines(const GeomLines *primitive);
  virtual void draw_linestrips(const GeomLinestrips *primitive);
  virtual void draw_points(const GeomPoints *primitive);
  virtual void end_draw_primitives();

  INLINE bool reset_if_new();
  INLINE void mark_new();
  virtual void reset();

  INLINE CPT(TransformState) get_transform();
  
  RenderBuffer get_render_buffer(int buffer_type);

  INLINE const DisplayRegion *get_current_display_region() const;
  INLINE const Lens *get_current_lens() const;

  INLINE DisplayRegionStack push_display_region(const DisplayRegion *dr);
  INLINE void pop_display_region(DisplayRegionStack &node);

  INLINE const TransformState *get_cs_transform() const;
  INLINE const TransformState *get_inv_cs_transform() const;

  void do_issue_clip_plane();
  void do_issue_color();
  void do_issue_color_scale();
  void do_issue_light();
  
  virtual void bind_light(PointLight *light_obj, const NodePath &light, 
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light,
                          int light_id);

protected:
  INLINE NodePath get_light(int light_id) const;
  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const Colorf &color);
  virtual void enable_light(int light_id, bool enable);
  virtual void begin_bind_lights();
  virtual void end_bind_lights();

  INLINE NodePath get_clip_plane(int plane_id) const;
  virtual void enable_clip_planes(bool enable);
  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void begin_bind_clip_planes();
  virtual void bind_clip_plane(const NodePath &plane, int plane_id);
  virtual void end_bind_clip_planes();

  virtual void free_pointers();
  virtual void close_gsg();
  void panic_deactivate();

  void determine_light_color_scale();

  INLINE void set_properties(const FrameBufferProperties &properties);

#ifdef DO_PSTATS
  // These functions are used to update the active texture memory
  // usage record (and other frame-based measurements) in Pstats.
  void init_frame_pstats();
  void add_to_texture_record(TextureContext *tc);
  void add_to_geom_record(GeomContext *gc);
  void add_to_vertex_buffer_record(VertexBufferContext *vbc);
  void add_to_index_buffer_record(IndexBufferContext *ibc);
  void add_to_total_buffer_record(VertexBufferContext *vbc);
  void add_to_total_buffer_record(IndexBufferContext *ibc);

  pset<TextureContext *> _current_textures;
  pset<GeomContext *> _current_geoms;
  pset<VertexBufferContext *> _current_vertex_buffers;
  pset<IndexBufferContext *> _current_index_buffers;
#else
  INLINE void init_frame_pstats() { }
  INLINE void add_to_texture_record(TextureContext *) { }
  INLINE void add_to_geom_record(GeomContext *) { }
  INLINE void record_state_change(TypeHandle) { }
  INLINE void add_to_vertex_buffer_record(VertexBufferContext *) { }
  INLINE void add_to_index_buffer_record(IndexBufferContext *) { }
  INLINE void add_to_total_buffer_record(VertexBufferContext *) { };
  INLINE void add_to_total_buffer_record(IndexBufferContext *) { };
#endif

  static CPT(RenderState) get_unlit_state();
  static CPT(RenderState) get_unclipped_state();
  static CPT(RenderState) get_untextured_state();

protected:
  PT(SceneSetup) _scene_null;
  PT(SceneSetup) _scene_setup;
  
  AttribSlots _state;
  AttribSlots _target;
  CPT(RenderState) _state_rs;
  CPT(RenderState) _target_rs;
  CPT(TransformState) _external_transform;
  CPT(TransformState) _internal_transform;
  CPT(GeomMunger) _munger;
  CPT(GeomVertexData) _vertex_data;

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
  CPT(TransformState) _inv_cs_transform;

  Colorf _scene_graph_color;
  bool _has_scene_graph_color;
  bool _transform_stale;
  bool _color_blend_involves_color_scale;
  bool _texture_involves_color_scale;
  bool _vertex_colors_enabled;
  bool _lighting_enabled;
  bool _clip_planes_enabled;
  bool _color_scale_enabled;
  LVecBase4f _current_color_scale;

  bool _has_material_force_color;
  Colorf _material_force_color;
  LVecBase4f _light_color_scale;

  bool _tex_gen_modifies_mat;
  bool _tex_gen_point_sprite;
  int _last_max_stage_index;

  bool _needs_reset;
  bool _is_valid;
  bool _closing_gsg;
  bool _active;

  PT(PreparedGraphicsObjects) _prepared_objects;

  bool _prefers_triangle_strips;
  int _max_vertices_per_array;
  int _max_vertices_per_primitive;

  int _max_texture_stages;
  int _max_texture_dimension;
  int _max_3d_texture_dimension;
  int _max_cube_map_dimension;

  bool _supports_texture_combine;
  bool _supports_texture_saved_result;
  bool _supports_texture_dot3;

  bool _supports_3d_texture;
  bool _supports_cube_map;

  int _max_lights;
  int _max_clip_planes;

  int _max_vertex_transforms;
  int _max_vertex_transform_indices;

  bool _copy_texture_inverted;
  bool _supports_multisample;
  bool _supports_generate_mipmap;
  bool _supports_render_texture;
  bool _supports_depth_texture;
  bool _supports_shadow_filter;
  bool _supports_basic_shaders;
  int _supported_geom_rendering;
  bool _color_scale_via_lighting;
  
public:
  // Statistics
  static PStatCollector _total_texusage_pcollector;
  static PStatCollector _active_texusage_pcollector;
  static PStatCollector _texture_count_pcollector;
  static PStatCollector _active_texture_count_pcollector;
  static PStatCollector _vertex_buffer_switch_pcollector;
  static PStatCollector _index_buffer_switch_pcollector;
  static PStatCollector _load_vertex_buffer_pcollector;
  static PStatCollector _load_index_buffer_pcollector;
  static PStatCollector _create_vertex_buffer_pcollector;
  static PStatCollector _create_index_buffer_pcollector;
  static PStatCollector _load_texture_pcollector;
  static PStatCollector _data_transferred_pcollector;
  static PStatCollector _total_geom_pcollector;
  static PStatCollector _active_geom_pcollector;
  static PStatCollector _total_buffers_pcollector;
  static PStatCollector _active_vertex_buffers_pcollector;
  static PStatCollector _active_index_buffers_pcollector;
  static PStatCollector _total_geom_node_pcollector;
  static PStatCollector _active_geom_node_pcollector;
  static PStatCollector _total_texmem_pcollector;
  static PStatCollector _used_texmem_pcollector;
  static PStatCollector _texmgrmem_total_pcollector;
  static PStatCollector _texmgrmem_resident_pcollector;
  static PStatCollector _primitive_batches_pcollector;
  static PStatCollector _primitive_batches_tristrip_pcollector;
  static PStatCollector _primitive_batches_trifan_pcollector;
  static PStatCollector _primitive_batches_tri_pcollector;
  static PStatCollector _primitive_batches_other_pcollector;
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
    NodePath _plane;
    bool _enabled;
    bool _next_enabled;
  };

  pvector<ClipPlaneInfo> _clip_plane_info;
  bool _clip_planes_enabled_this_frame;

  FrameBufferProperties _properties;
  PT(GraphicsPipe) _pipe;
  GraphicsEngine *_engine;
  GraphicsThreadingModel _threading_model;

  static GraphicsStateGuardian *_global_gsg;

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
