// Filename: graphicsStateGuardian.h
// Created by:  drose (02eb99)
// Updated by: fperazzi, PandaSE (05May10) (added fetch_ptr_parameter,
//  _max_2d_texture_array_layers on z axis, get_supports_cg_profile)
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

#ifndef GRAPHICSSTATEGUARDIAN_H
#define GRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "frameBufferProperties.h"
#include "preparedGraphicsObjects.h"
#include "lens.h"
#include "graphicsStateGuardianBase.h"
#include "graphicsThreadingModel.h"
#include "graphicsPipe.h"
#include "sceneSetup.h"
#include "displayRegion.h"
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
#include "pnotify.h"
#include "pvector.h"
#include "shaderContext.h"
#include "bitMask.h"
#include "texture.h"
#include "occlusionQueryContext.h"
#include "stencilRenderStates.h"
#include "loader.h"
#include "shaderAttrib.h"
#include "texGenAttrib.h"
#include "textureAttrib.h"

class DrawableRegion;
class GraphicsEngine;
class ShaderGenerator;

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
class EXPCL_PANDA_DISPLAY GraphicsStateGuardian : public GraphicsStateGuardianBase {
  //
  // Interfaces all GSGs should have
  //
public:
  GraphicsStateGuardian(CoordinateSystem internal_coordinate_system,
                        GraphicsEngine *engine, GraphicsPipe *pipe);
  virtual ~GraphicsStateGuardian();

PUBLISHED:

  enum ShaderModel {
    SM_00,
    SM_11,
    SM_20,
    SM_2X,
    SM_30,
    SM_40,
    SM_50,
  };

  INLINE void release_all();
  INLINE int release_all_textures();
  INLINE int release_all_geoms();
  INLINE int release_all_vertex_buffers();
  INLINE int release_all_index_buffers();

  INLINE void set_active(bool active);
  INLINE bool is_active() const;
  INLINE bool is_valid() const;
  INLINE bool needs_reset() const;

  INLINE void set_incomplete_render(bool incomplete_render);
  virtual INLINE bool get_incomplete_render() const;
  virtual INLINE bool get_effective_incomplete_render() const;

  INLINE void set_loader(Loader *loader);
  INLINE Loader *get_loader() const;

  INLINE GraphicsPipe *get_pipe() const;
  GraphicsEngine *get_engine() const;
  INLINE const GraphicsThreadingModel &get_threading_model() const;

  INLINE bool is_hardware() const;
  virtual INLINE bool prefers_triangle_strips() const;
  virtual INLINE int get_max_vertices_per_array() const;
  virtual INLINE int get_max_vertices_per_primitive() const;

  INLINE int get_max_texture_stages() const;
  virtual INLINE int get_max_texture_dimension() const;
  INLINE int get_max_3d_texture_dimension() const;
  INLINE int get_max_2d_texture_array_layers() const; //z axis
  INLINE int get_max_cube_map_dimension() const;

  INLINE bool get_supports_texture_combine() const;
  INLINE bool get_supports_texture_saved_result() const;
  INLINE bool get_supports_texture_dot3() const;

  INLINE bool get_supports_3d_texture() const;
  INLINE bool get_supports_2d_texture_array() const;
  INLINE bool get_supports_cube_map() const;
  INLINE bool get_supports_tex_non_pow2() const;

  INLINE bool get_supports_compressed_texture() const;
  virtual INLINE bool get_supports_compressed_texture_format(int compression_mode) const;

  INLINE int get_max_lights() const;
  INLINE int get_max_clip_planes() const;

  INLINE int get_max_vertex_transforms() const;
  INLINE int get_max_vertex_transform_indices() const;

  INLINE bool get_copy_texture_inverted() const;
  virtual bool get_supports_multisample() const;
  INLINE bool get_supports_generate_mipmap() const;
  INLINE bool get_supports_depth_texture() const;
  INLINE bool get_supports_depth_stencil() const;
  INLINE bool get_supports_shadow_filter() const;
  INLINE bool get_supports_basic_shaders() const;
  INLINE bool get_supports_geometry_shaders() const;
  INLINE bool get_supports_tessellation_shaders() const;
  INLINE bool get_supports_glsl() const;
  INLINE bool get_supports_stencil() const;
  INLINE bool get_supports_two_sided_stencil() const;
  INLINE bool get_supports_geometry_instancing() const;

  INLINE int get_max_color_targets() const;
  INLINE int get_maximum_simultaneous_render_targets() const;

  INLINE int get_shader_model() const;
  INLINE void set_shader_model(int shader_model);

  virtual int get_supported_geom_rendering() const;
  virtual bool get_supports_cg_profile(const string &name) const;


  INLINE bool get_color_scale_via_lighting() const;
  INLINE bool get_alpha_scale_via_texture() const;
  INLINE bool get_alpha_scale_via_texture(const TextureAttrib *tex_attrib) const;
  INLINE bool get_runtime_color_scale() const;

  INLINE static TextureStage *get_alpha_scale_texture_stage();

  void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;
  virtual CoordinateSystem get_internal_coordinate_system() const;

  virtual PreparedGraphicsObjects *get_prepared_objects();

  virtual bool set_gamma(PN_stdfloat gamma);
  PN_stdfloat get_gamma(PN_stdfloat gamma);
  virtual void restore_gamma();

  INLINE void set_texture_quality_override(Texture::QualityLevel quality_level);
  INLINE Texture::QualityLevel get_texture_quality_override() const;

  EXTENSION(PyObject *get_prepared_textures() const);
  typedef bool TextureCallback(TextureContext *tc, void *callback_arg);
  void traverse_prepared_textures(TextureCallback *func, void *callback_arg);

#ifndef NDEBUG
  void set_flash_texture(Texture *tex);
  void clear_flash_texture();
  Texture *get_flash_texture() const;
#endif

PUBLISHED:

  virtual string get_driver_vendor();
  virtual string get_driver_renderer();
  virtual string get_driver_version();
  virtual int get_driver_version_major();
  virtual int get_driver_version_minor();
  virtual int get_driver_shader_version_major();
  virtual int get_driver_shader_version_minor();
  
  bool set_scene(SceneSetup *scene_setup);
  virtual SceneSetup *get_scene() const;

public:
  virtual TextureContext *prepare_texture(Texture *tex);
  virtual bool update_texture(TextureContext *tc, bool force);
  virtual void release_texture(TextureContext *tc);
  virtual bool extract_texture_data(Texture *tex);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);

  virtual ShaderContext *prepare_shader(Shader *shader);
  virtual void release_shader(ShaderContext *sc);

  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data);
  virtual void release_vertex_buffer(VertexBufferContext *vbc);

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data);
  virtual void release_index_buffer(IndexBufferContext *ibc);

  virtual bool get_supports_occlusion_query() const;
  virtual void begin_occlusion_query();
  virtual PT(OcclusionQueryContext) end_occlusion_query();

  virtual PT(GeomMunger) get_geom_munger(const RenderState *state,
                                         Thread *current_thread);
  virtual PT(GeomMunger) make_geom_munger(const RenderState *state,
                                          Thread *current_thread);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  virtual PN_stdfloat compute_distance_to(const LPoint3 &point) const;

  virtual void clear(DrawableRegion *clearable);
  
  const LMatrix4 *fetch_specified_value(Shader::ShaderMatSpec &spec, int altered);
  const LMatrix4 *fetch_specified_part(Shader::ShaderMatInput input, InternalName *name, LMatrix4 &t);
  const Shader::ShaderPtrData *fetch_ptr_parameter(const Shader::ShaderPtrSpec& spec);

  virtual void prepare_display_region(DisplayRegionPipelineReader *dr);
  virtual void clear_before_callback();
  virtual void clear_state_and_transform();

  virtual void remove_window(GraphicsOutputBase *window);

  virtual CPT(TransformState) calc_projection_mat(const Lens *lens);
  virtual bool prepare_lens();

  virtual bool begin_frame(Thread *current_thread);
PUBLISHED:
  virtual bool begin_scene();
  virtual void end_scene();
public:
  virtual void end_frame(Thread *current_thread);

  void set_current_properties(const FrameBufferProperties *properties);

  virtual bool depth_offset_decals();
  virtual CPT(RenderState) begin_decal_base_first();
  virtual CPT(RenderState) begin_decal_nested();
  virtual CPT(RenderState) begin_decal_base_second();
  virtual void finish_decal();

  virtual bool begin_draw_primitives(const GeomPipelineReader *geom_reader,
                                     const GeomMunger *munger,
                                     const GeomVertexDataPipelineReader *data_reader,
                                     bool force);
  virtual bool draw_triangles(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_tristrips(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_trifans(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_patches(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_lines(const GeomPrimitivePipelineReader *reader,
                          bool force);
  virtual bool draw_linestrips(const GeomPrimitivePipelineReader *reader,
                               bool force);
  virtual bool draw_points(const GeomPrimitivePipelineReader *reader,
                           bool force);
  virtual void end_draw_primitives();

  INLINE bool reset_if_new();
  INLINE void mark_new();
  virtual void reset();

  INLINE CPT(TransformState) get_external_transform() const;
  INLINE CPT(TransformState) get_internal_transform() const;

  RenderBuffer get_render_buffer(int buffer_type, const FrameBufferProperties &prop);

  INLINE const DisplayRegion *get_current_display_region() const;
  INLINE Lens::StereoChannel get_current_stereo_channel() const;
  INLINE int get_current_tex_view_offset() const;
  INLINE const Lens *get_current_lens() const;

  virtual CPT(TransformState) get_cs_transform_for(CoordinateSystem cs) const;
  virtual CPT(TransformState) get_cs_transform() const;
  INLINE CPT(TransformState) get_inv_cs_transform() const;

  void do_issue_clip_plane();
  void do_issue_color();
  void do_issue_color_scale();
  virtual void do_issue_light();

  virtual bool framebuffer_copy_to_texture
  (Texture *tex, int view, int z, const DisplayRegion *dr, const RenderBuffer &rb);
  virtual bool framebuffer_copy_to_ram
  (Texture *tex, int view, int z, const DisplayRegion *dr, const RenderBuffer &rb);

  virtual void bind_light(PointLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light,
                          int light_id);

  static void create_gamma_table (PN_stdfloat gamma, unsigned short *red_table, unsigned short *green_table, unsigned short *blue_table);

  virtual PT(Texture) make_shadow_buffer(const NodePath &light_np, GraphicsOutputBase *host);

#ifdef DO_PSTATS
  static void init_frame_pstats();
#endif

protected:
  virtual void reissue_transforms();

  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const LColor &color);
  virtual void enable_light(int light_id, bool enable);
  virtual void begin_bind_lights();
  virtual void end_bind_lights();

  virtual void enable_clip_planes(bool enable);
  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void begin_bind_clip_planes();
  virtual void bind_clip_plane(const NodePath &plane, int plane_id);
  virtual void end_bind_clip_planes();

  void determine_target_texture();

  virtual void free_pointers();
  virtual void close_gsg();
  void panic_deactivate();

  void determine_light_color_scale();

  static CPT(RenderState) get_unlit_state();
  static CPT(RenderState) get_unclipped_state();
  static CPT(RenderState) get_untextured_state();

  void async_reload_texture(TextureContext *tc);

protected:
  PT(SceneSetup) _scene_null;
  PT(SceneSetup) _scene_setup;

  // The current state of the graphics context, as of the last call to
  // set_state_and_transform().
  CPT(RenderState) _state_rs;

  // The desired state of the graphics context, during processing of
  // set_state_and_transform().
  CPT(RenderState) _target_rs;

  // This bitmask contains a 1 bit everywhere that _state_rs has a
  // known value.  If a bit is 0, the corresponding state must be
  // re-sent.
  // 
  // Derived GSGs should initialize _inv_state_mask in reset() as a mask of
  // 1's where they don't care, and 0's where they do care, about the state.
  RenderState::SlotMask _state_mask;
  RenderState::SlotMask _inv_state_mask;

  // The current transform, as of the last call to
  // set_state_and_transform().
  CPT(TransformState) _internal_transform;

  // The current TextureAttrib is a special case; we may further
  // restrict it (according to graphics cards limits) or extend it
  // (according to ColorScaleAttribs in effect) beyond what is
  // specifically requested in the scene graph.
  CPT(TextureAttrib) _target_texture;
  CPT(TextureAttrib) _state_texture;
  CPT(TexGenAttrib) _target_tex_gen;
  CPT(TexGenAttrib) _state_tex_gen;

  // Also, the shader might be the explicitly-requested shader, or it
  // might be an auto-generated one.
  CPT(ShaderAttrib) _state_shader;
  CPT(ShaderAttrib) _target_shader;

  // These are set by begin_draw_primitives(), and are only valid
  // between begin_draw_primitives() and end_draw_primitives().
  CPT(GeomMunger) _munger;
  const GeomVertexDataPipelineReader *_data_reader;

  unsigned int _color_write_mask;

  CPT(DisplayRegion) _current_display_region;
  Lens::StereoChannel _current_stereo_channel;
  int _current_tex_view_offset;
  CPT(Lens) _current_lens;
  CPT(TransformState) _projection_mat;
  CPT(TransformState) _projection_mat_inv;
  const FrameBufferProperties *_current_properties;

  CoordinateSystem _coordinate_system;
  CoordinateSystem _internal_coordinate_system;
  CPT(TransformState) _cs_transform;
  CPT(TransformState) _inv_cs_transform;

  LColor _scene_graph_color;
  bool _has_scene_graph_color;
  bool _transform_stale;
  bool _color_blend_involves_color_scale;
  bool _texture_involves_color_scale;
  bool _vertex_colors_enabled;
  bool _lighting_enabled;
  bool _clip_planes_enabled;
  bool _color_scale_enabled;
  LVecBase4 _current_color_scale;

  bool _has_material_force_color;
  LColor _material_force_color;
  LVecBase4 _light_color_scale;
  bool _has_texture_alpha_scale;

  bool _tex_gen_modifies_mat;
  bool _tex_gen_point_sprite;
  int _last_max_stage_index;

  bool _needs_reset;
  bool _is_valid;
  bool _closing_gsg;
  bool _active;
  bool _incomplete_render;
  bool _effective_incomplete_render;
  PT(Loader) _loader;

  PT(PreparedGraphicsObjects) _prepared_objects;

  bool _is_hardware;
  bool _prefers_triangle_strips;
  int _max_vertices_per_array;
  int _max_vertices_per_primitive;

  int _max_texture_stages;
  int _max_texture_dimension;
  int _max_3d_texture_dimension;
  int _max_2d_texture_array_layers; //on the z axis
  int _max_cube_map_dimension;

  bool _supports_texture_combine;
  bool _supports_texture_saved_result;
  bool _supports_texture_dot3;

  bool _supports_3d_texture;
  bool _supports_2d_texture_array;
  bool _supports_cube_map;
  bool _supports_tex_non_pow2;

  bool _supports_compressed_texture;
  BitMask32 _compressed_texture_formats;

  int _max_lights;
  int _max_clip_planes;

  int _max_vertex_transforms;
  int _max_vertex_transform_indices;

  bool _supports_occlusion_query;
  PT(OcclusionQueryContext) _current_occlusion_query;

  bool _copy_texture_inverted;
  bool _supports_multisample;
  bool _supports_generate_mipmap;
  bool _supports_depth_texture;
  bool _supports_depth_stencil;
  bool _supports_shadow_filter;
  bool _supports_basic_shaders;
  bool _supports_geometry_shaders;
  bool _supports_tessellation_shaders;
  bool _supports_glsl;
  bool _supports_framebuffer_multisample;
  bool _supports_framebuffer_blit;
  
  bool _supports_stencil;
  bool _supports_stencil_wrap;
  bool _supports_two_sided_stencil;
  bool _supports_geometry_instancing;

  int _max_color_targets;

  int  _supported_geom_rendering;
  bool _color_scale_via_lighting;
  bool _alpha_scale_via_texture;
  bool _runtime_color_scale;

  int  _stereo_buffer_mask;

  StencilRenderStates *_stencil_render_states;

  int _auto_detect_shader_model;
  int _shader_model;

  static PT(TextureStage) _alpha_scale_texture_stage;

  Shader::ShaderCaps _shader_caps;

  PN_stdfloat _gamma;
  Texture::QualityLevel _texture_quality_override;
  
  ShaderGenerator* _shader_generator;

#ifndef NDEBUG
  PT(Texture) _flash_texture;
#endif
  
public:
  // Statistics
  static PStatCollector _vertex_buffer_switch_pcollector;
  static PStatCollector _index_buffer_switch_pcollector;
  static PStatCollector _load_vertex_buffer_pcollector;
  static PStatCollector _load_index_buffer_pcollector;
  static PStatCollector _create_vertex_buffer_pcollector;
  static PStatCollector _create_index_buffer_pcollector;
  static PStatCollector _load_texture_pcollector;
  static PStatCollector _data_transferred_pcollector;
  static PStatCollector _texmgrmem_total_pcollector;
  static PStatCollector _texmgrmem_resident_pcollector;
  static PStatCollector _primitive_batches_pcollector;
  static PStatCollector _primitive_batches_tristrip_pcollector;
  static PStatCollector _primitive_batches_trifan_pcollector;
  static PStatCollector _primitive_batches_tri_pcollector;
  static PStatCollector _primitive_batches_patch_pcollector;
  static PStatCollector _primitive_batches_other_pcollector;
  static PStatCollector _vertices_tristrip_pcollector;
  static PStatCollector _vertices_trifan_pcollector;
  static PStatCollector _vertices_tri_pcollector;
  static PStatCollector _vertices_patch_pcollector;
  static PStatCollector _vertices_other_pcollector;
  static PStatCollector _vertices_indexed_tristrip_pcollector;
  static PStatCollector _state_pcollector;
  static PStatCollector _transform_state_pcollector;
  static PStatCollector _texture_state_pcollector;
  static PStatCollector _draw_primitive_pcollector;
  static PStatCollector _draw_set_state_pcollector;
  static PStatCollector _clear_pcollector;
  static PStatCollector _flush_pcollector;
  static PStatCollector _wait_occlusion_pcollector;

  // A whole slew of collectors to measure the cost of individual
  // state changes.  These are disabled by default.
  static PStatCollector _draw_set_state_transform_pcollector;
  static PStatCollector _draw_set_state_alpha_test_pcollector;
  static PStatCollector _draw_set_state_antialias_pcollector;
  static PStatCollector _draw_set_state_clip_plane_pcollector;
  static PStatCollector _draw_set_state_color_pcollector;
  static PStatCollector _draw_set_state_cull_face_pcollector;
  static PStatCollector _draw_set_state_depth_offset_pcollector;
  static PStatCollector _draw_set_state_depth_test_pcollector;
  static PStatCollector _draw_set_state_depth_write_pcollector;
  static PStatCollector _draw_set_state_render_mode_pcollector;
  static PStatCollector _draw_set_state_rescale_normal_pcollector;
  static PStatCollector _draw_set_state_shade_model_pcollector;
  static PStatCollector _draw_set_state_blending_pcollector;
  static PStatCollector _draw_set_state_shader_pcollector;
  static PStatCollector _draw_set_state_shader_parameters_pcollector;
  static PStatCollector _draw_set_state_texture_pcollector;
  static PStatCollector _draw_set_state_tex_matrix_pcollector;
  static PStatCollector _draw_set_state_tex_gen_pcollector;
  static PStatCollector _draw_set_state_material_pcollector;
  static PStatCollector _draw_set_state_light_pcollector;
  static PStatCollector _draw_set_state_stencil_pcollector;
  static PStatCollector _draw_set_state_fog_pcollector;
  static PStatCollector _draw_set_state_scissor_pcollector;

private:
  int _num_lights_enabled;
  int _num_clip_planes_enabled;

  PT(GraphicsPipe) _pipe;
  GraphicsEngine *_engine;
  GraphicsThreadingModel _threading_model;

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
