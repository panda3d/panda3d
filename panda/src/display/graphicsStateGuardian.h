/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsStateGuardian.h
 * @author drose
 * @date 1999-02-02
 * @author fperazzi, PandaSE
 * @date 2010-05-05
 *  _max_2d_texture_array_layers on z axis, get_supports_cg_profile)
 */

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
#include "timerQueryContext.h"
#include "loader.h"
#include "shaderAttrib.h"
#include "texGenAttrib.h"
#include "textureAttrib.h"
#include "shaderGenerator.h"

class DrawableRegion;
class GraphicsEngine;

/**
 * Encapsulates all the communication with a particular instance of a given
 * rendering backend.  Tries to guarantee that redundant state-change requests
 * are not issued (hence "state guardian").
 *
 * There will be one of these objects for each different graphics context
 * active in the system.
 */
class EXPCL_PANDA_DISPLAY GraphicsStateGuardian : public GraphicsStateGuardianBase {
  // Interfaces all GSGs should have
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
    SM_51,
  };

  INLINE void release_all();
  INLINE int release_all_textures();
  INLINE int release_all_samplers();
  INLINE int release_all_geoms();
  INLINE int release_all_vertex_buffers();
  INLINE int release_all_index_buffers();
  INLINE int release_all_shader_buffers();

  INLINE void set_active(bool active);
  INLINE bool is_active() const;
  INLINE bool is_valid() const;
  INLINE bool needs_reset() const;
  MAKE_PROPERTY(active, is_active, set_active);
  MAKE_PROPERTY(valid, is_valid);

  INLINE void set_incomplete_render(bool incomplete_render);
  virtual INLINE bool get_incomplete_render() const;
  virtual INLINE bool get_effective_incomplete_render() const;
  MAKE_PROPERTY(incomplete_render, get_incomplete_render, set_incomplete_render);
  MAKE_PROPERTY(effective_incomplete_render, get_effective_incomplete_render);

  INLINE void set_loader(Loader *loader);
  INLINE Loader *get_loader() const;
  MAKE_PROPERTY(loader, get_loader, set_loader);

  INLINE void set_shader_generator(ShaderGenerator *shader_generator);
  INLINE ShaderGenerator *get_shader_generator() const;
  MAKE_PROPERTY(shader_generator, get_shader_generator, set_shader_generator);

  INLINE GraphicsPipe *get_pipe() const;
  GraphicsEngine *get_engine() const;
  INLINE const GraphicsThreadingModel &get_threading_model() const;
  MAKE_PROPERTY(pipe, get_pipe);

  INLINE bool is_hardware() const;
  virtual INLINE bool prefers_triangle_strips() const;
  virtual INLINE int get_max_vertices_per_array() const;
  virtual INLINE int get_max_vertices_per_primitive() const;

  INLINE int get_max_texture_stages() const;
  virtual INLINE int get_max_texture_dimension() const;
  INLINE int get_max_3d_texture_dimension() const;
  INLINE int get_max_2d_texture_array_layers() const; //z axis
  INLINE int get_max_cube_map_dimension() const;
  INLINE int get_max_buffer_texture_size() const;

  INLINE bool get_supports_texture_combine() const;
  INLINE bool get_supports_texture_saved_result() const;
  INLINE bool get_supports_texture_dot3() const;

  INLINE bool get_supports_3d_texture() const;
  INLINE bool get_supports_2d_texture_array() const;
  INLINE bool get_supports_cube_map() const;
  INLINE bool get_supports_buffer_texture() const;
  INLINE bool get_supports_cube_map_array() const;
  INLINE bool get_supports_tex_non_pow2() const;
  INLINE bool get_supports_texture_srgb() const;

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
  INLINE bool get_supports_luminance_texture() const;
  INLINE bool get_supports_shadow_filter() const;
  INLINE bool get_supports_sampler_objects() const;
  INLINE bool get_supports_basic_shaders() const;
  INLINE bool get_supports_geometry_shaders() const;
  INLINE bool get_supports_tessellation_shaders() const;
  INLINE bool get_supports_compute_shaders() const;
  INLINE bool get_supports_glsl() const;
  INLINE bool get_supports_hlsl() const;
  INLINE bool get_supports_stencil() const;
  INLINE bool get_supports_two_sided_stencil() const;
  INLINE bool get_supports_geometry_instancing() const;
  INLINE bool get_supports_indirect_draw() const;

  INLINE bool get_supports_occlusion_query() const;
  INLINE bool get_supports_timer_query() const;
  INLINE bool get_timer_queries_active() const;

  INLINE int get_max_color_targets() const;
  INLINE int get_maximum_simultaneous_render_targets() const;
  INLINE bool get_supports_dual_source_blending() const;

  MAKE_PROPERTY(max_vertices_per_array, get_max_vertices_per_array);
  MAKE_PROPERTY(max_vertices_per_primitive, get_max_vertices_per_primitive);
  MAKE_PROPERTY(max_texture_stages, get_max_texture_stages);
  MAKE_PROPERTY(max_texture_dimension, get_max_texture_dimension);
  MAKE_PROPERTY(max_3d_texture_dimension, get_max_3d_texture_dimension);
  MAKE_PROPERTY(max_2d_texture_array_layers, get_max_2d_texture_array_layers);
  MAKE_PROPERTY(max_cube_map_dimension, get_max_cube_map_dimension);
  MAKE_PROPERTY(max_buffer_texture_size, get_max_buffer_texture_size);
  MAKE_PROPERTY(supports_texture_combine, get_supports_texture_combine);
  MAKE_PROPERTY(supports_texture_saved_result, get_supports_texture_saved_result);
  MAKE_PROPERTY(supports_texture_dot3, get_supports_texture_dot3);
  MAKE_PROPERTY(supports_3d_texture, get_supports_3d_texture);
  MAKE_PROPERTY(supports_2d_texture_array, get_supports_2d_texture_array);
  MAKE_PROPERTY(supports_cube_map, get_supports_cube_map);
  MAKE_PROPERTY(supports_buffer_texture, get_supports_buffer_texture);
  MAKE_PROPERTY(supports_cube_map_array, get_supports_cube_map_array);
  MAKE_PROPERTY(supports_tex_non_pow2, get_supports_tex_non_pow2);
  MAKE_PROPERTY(supports_texture_srgb, get_supports_texture_srgb);
  MAKE_PROPERTY(supports_compressed_texture, get_supports_compressed_texture);
  MAKE_PROPERTY(max_lights, get_max_lights);
  MAKE_PROPERTY(max_clip_planes, get_max_clip_planes);
  MAKE_PROPERTY(max_vertex_transforms, get_max_vertex_transforms);
  MAKE_PROPERTY(max_vertex_transform_indices, get_max_vertex_transform_indices);
  MAKE_PROPERTY(copy_texture_inverted, get_copy_texture_inverted);
  MAKE_PROPERTY(supports_multisample, get_supports_multisample);
  MAKE_PROPERTY(supports_generate_mipmap, get_supports_generate_mipmap);
  MAKE_PROPERTY(supports_depth_texture, get_supports_depth_texture);
  MAKE_PROPERTY(supports_depth_stencil, get_supports_depth_stencil);
  MAKE_PROPERTY(supports_luminance_texture, get_supports_luminance_texture);
  MAKE_PROPERTY(supports_shadow_filter, get_supports_shadow_filter);
  MAKE_PROPERTY(supports_sampler_objects, get_supports_sampler_objects);
  MAKE_PROPERTY(supports_basic_shaders, get_supports_basic_shaders);
  MAKE_PROPERTY(supports_geometry_shaders, get_supports_geometry_shaders);
  MAKE_PROPERTY(supports_tessellation_shaders, get_supports_tessellation_shaders);
  MAKE_PROPERTY(supports_compute_shaders, get_supports_compute_shaders);
  MAKE_PROPERTY(supports_glsl, get_supports_glsl);
  MAKE_PROPERTY(supports_hlsl, get_supports_hlsl);
  MAKE_PROPERTY(supports_stencil, get_supports_stencil);
  MAKE_PROPERTY(supports_two_sided_stencil, get_supports_two_sided_stencil);
  MAKE_PROPERTY(supports_geometry_instancing, get_supports_geometry_instancing);
  MAKE_PROPERTY(supports_indirect_draw, get_supports_indirect_draw);
  MAKE_PROPERTY(supports_occlusion_query, get_supports_occlusion_query);
  MAKE_PROPERTY(supports_timer_query, get_supports_timer_query);
  MAKE_PROPERTY(timer_queries_active, get_timer_queries_active);
  MAKE_PROPERTY(max_color_targets, get_max_color_targets);
  MAKE_PROPERTY(supports_dual_source_blending, get_supports_dual_source_blending);

  INLINE ShaderModel get_shader_model() const;
  INLINE void set_shader_model(ShaderModel shader_model);
  MAKE_PROPERTY(shader_model, get_shader_model, set_shader_model);

  virtual int get_supported_geom_rendering() const;
  virtual bool get_supports_cg_profile(const std::string &name) const;

  INLINE bool get_color_scale_via_lighting() const;
  INLINE bool get_alpha_scale_via_texture() const;
  INLINE bool get_alpha_scale_via_texture(const TextureAttrib *tex_attrib) const;
  INLINE bool get_runtime_color_scale() const;

  INLINE static TextureStage *get_alpha_scale_texture_stage();

  void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;
  virtual CoordinateSystem get_internal_coordinate_system() const;
  MAKE_PROPERTY(coordinate_system, get_coordinate_system, set_coordinate_system);

  virtual PreparedGraphicsObjects *get_prepared_objects();
  MAKE_PROPERTY(prepared_objects, get_prepared_objects);

  virtual bool set_gamma(PN_stdfloat gamma);
  PN_stdfloat get_gamma() const;
  virtual void restore_gamma();
  MAKE_PROPERTY(gamma, get_gamma, set_gamma);

  INLINE void set_texture_quality_override(Texture::QualityLevel quality_level);
  INLINE Texture::QualityLevel get_texture_quality_override() const;
  MAKE_PROPERTY(texture_quality_override, get_texture_quality_override,
                                          set_texture_quality_override);

  EXTENSION(PyObject *get_prepared_textures() const);
  typedef bool TextureCallback(TextureContext *tc, void *callback_arg);
  void traverse_prepared_textures(TextureCallback *func, void *callback_arg);

#ifndef NDEBUG
  void set_flash_texture(Texture *tex);
  void clear_flash_texture();
  Texture *get_flash_texture() const;
  MAKE_PROPERTY(flash_texture, get_flash_texture, set_flash_texture);
#endif

PUBLISHED:
  virtual bool has_extension(const std::string &extension) const;

  virtual std::string get_driver_vendor();
  virtual std::string get_driver_renderer();
  virtual std::string get_driver_version();
  virtual int get_driver_version_major();
  virtual int get_driver_version_minor();
  virtual int get_driver_shader_version_major();
  virtual int get_driver_shader_version_minor();

  MAKE_PROPERTY(driver_vendor, get_driver_vendor);
  MAKE_PROPERTY(driver_renderer, get_driver_renderer);
  MAKE_PROPERTY(driver_version, get_driver_version);
  MAKE_PROPERTY(driver_version_major, get_driver_version_major);
  MAKE_PROPERTY(driver_version_minor, get_driver_version_minor);
  MAKE_PROPERTY(driver_shader_version_major, get_driver_shader_version_major);
  MAKE_PROPERTY(driver_shader_version_minor, get_driver_shader_version_minor);

  bool set_scene(SceneSetup *scene_setup);
  virtual SceneSetup *get_scene() const final;
  MAKE_PROPERTY(scene, get_scene, set_scene);

public:
  virtual TextureContext *prepare_texture(Texture *tex, int view);
  virtual bool update_texture(TextureContext *tc, bool force);
  virtual void release_texture(TextureContext *tc);
  virtual void release_textures(const pvector<TextureContext *> &contexts);
  virtual bool extract_texture_data(Texture *tex);

  virtual SamplerContext *prepare_sampler(const SamplerState &sampler);
  virtual void release_sampler(SamplerContext *sc);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);

  virtual ShaderContext *prepare_shader(Shader *shader);
  virtual void release_shader(ShaderContext *sc);

  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data);
  virtual void release_vertex_buffer(VertexBufferContext *vbc);
  virtual void release_vertex_buffers(const pvector<BufferContext *> &contexts);

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data);
  virtual void release_index_buffer(IndexBufferContext *ibc);
  virtual void release_index_buffers(const pvector<BufferContext *> &contexts);

  virtual BufferContext *prepare_shader_buffer(ShaderBuffer *data);
  virtual void release_shader_buffer(BufferContext *ibc);
  virtual void release_shader_buffers(const pvector<BufferContext *> &contexts);

  virtual void begin_occlusion_query();
  virtual PT(OcclusionQueryContext) end_occlusion_query();

  virtual PT(TimerQueryContext) issue_timer_query(int pstats_index);

  virtual void dispatch_compute(int size_x, int size_y, int size_z);

  virtual PT(GeomMunger) get_geom_munger(const RenderState *state,
                                         Thread *current_thread);
  virtual PT(GeomMunger) make_geom_munger(const RenderState *state,
                                          Thread *current_thread);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  PN_stdfloat compute_distance_to(const LPoint3 &point) const;

  virtual void clear(DrawableRegion *clearable);

  const LMatrix4 *fetch_specified_value(Shader::ShaderMatSpec &spec, int altered);
  const LMatrix4 *fetch_specified_part(Shader::ShaderMatInput input, InternalName *name,
                                       LMatrix4 &t, int index);
  const LMatrix4 *fetch_specified_member(const NodePath &np, CPT_InternalName member, LMatrix4 &t);
  PT(Texture) fetch_specified_texture(Shader::ShaderTexSpec &spec,
                                      SamplerState &sampler, int &view);
  const Shader::ShaderPtrData *fetch_ptr_parameter(const Shader::ShaderPtrSpec& spec);
  bool fetch_ptr_parameter(const Shader::ShaderPtrSpec &spec, Shader::ShaderPtrData &data);

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

  void flush_timer_queries();

  void set_current_properties(const FrameBufferProperties *properties);

  virtual bool depth_offset_decals();
  virtual CPT(RenderState) begin_decal_base_first();
  virtual CPT(RenderState) begin_decal_nested();
  virtual CPT(RenderState) begin_decal_base_second();
  virtual void finish_decal();

  virtual bool begin_draw_primitives(const GeomPipelineReader *geom_reader,
                                     const GeomVertexDataPipelineReader *data_reader,
                                     bool force);
  virtual bool draw_triangles(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_triangles_adj(const GeomPrimitivePipelineReader *reader,
                                  bool force);
  virtual bool draw_tristrips(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_tristrips_adj(const GeomPrimitivePipelineReader *reader,
                                  bool force);
  virtual bool draw_trifans(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_patches(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_lines(const GeomPrimitivePipelineReader *reader,
                          bool force);
  virtual bool draw_lines_adj(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_linestrips(const GeomPrimitivePipelineReader *reader,
                               bool force);
  virtual bool draw_linestrips_adj(const GeomPrimitivePipelineReader *reader,
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

  PT(Texture) get_shadow_map(const NodePath &light_np, GraphicsOutputBase *host=nullptr);
  PT(Texture) get_dummy_shadow_map(Texture::TextureType texture_type) const;
  virtual GraphicsOutput *make_shadow_buffer(LightLensNode *light, Texture *tex, GraphicsOutput *host);

  virtual void ensure_generated_shader(const RenderState *state);

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
  void determine_target_shader();

  virtual void free_pointers();
  virtual void close_gsg();
  void panic_deactivate();

  void determine_light_color_scale();

  static CPT(RenderState) get_unlit_state();
  static CPT(RenderState) get_unclipped_state();
  static CPT(RenderState) get_untextured_state();

  AsyncFuture *async_reload_texture(TextureContext *tc);

protected:
  PT(SceneSetup) _scene_null;
  PT(SceneSetup) _scene_setup;

  // The current state of the graphics context, as of the last call to
  // set_state_and_transform().
  CPT(RenderState) _state_rs;

  // The desired state of the graphics context, during processing of
  // set_state_and_transform().
  CPT(RenderState) _target_rs;

  // This bitmask contains a 1 bit everywhere that _state_rs has a known
  // value.  If a bit is 0, the corresponding state must be re-sent.  Derived
  // GSGs should initialize _inv_state_mask in reset() as a mask of 1's where
  // they don't care, and 0's where they do care, about the state.
  RenderState::SlotMask _state_mask;
  RenderState::SlotMask _inv_state_mask;

  // The current transform, as of the last call to set_state_and_transform().
  CPT(TransformState) _internal_transform;

  // The current TextureAttrib is a special case; we may further restrict it
  // (according to graphics cards limits) or extend it (according to
  // ColorScaleAttribs in effect) beyond what is specifically requested in the
  // scene graph.
  CPT(TextureAttrib) _target_texture;
  CPT(TextureAttrib) _state_texture;
  CPT(TexGenAttrib) _target_tex_gen;
  CPT(TexGenAttrib) _state_tex_gen;

  // Also, the shader might be the explicitly-requested shader, or it might be
  // an auto-generated one.
  CPT(ShaderAttrib) _state_shader;
  CPT(ShaderAttrib) _target_shader;

  // This is set by begin_draw_primitives(), and are only valid between
  // begin_draw_primitives() and end_draw_primitives().
  const GeomVertexDataPipelineReader *_data_reader;

  unsigned int _color_write_mask;

  PT(DisplayRegion) _current_display_region;
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
  int _max_buffer_texture_size;

  bool _supports_texture_combine;
  bool _supports_texture_saved_result;
  bool _supports_texture_dot3;

  bool _supports_3d_texture;
  bool _supports_2d_texture_array;
  bool _supports_cube_map;
  bool _supports_buffer_texture;
  bool _supports_cube_map_array;
  bool _supports_tex_non_pow2;
  bool _supports_texture_srgb;

  bool _supports_compressed_texture;
  BitMask32 _compressed_texture_formats;

  int _max_lights;
  int _max_clip_planes;

  int _max_vertex_transforms;
  int _max_vertex_transform_indices;

  bool _supports_occlusion_query;
  PT(OcclusionQueryContext) _current_occlusion_query;

  bool _supports_timer_query;
#ifdef DO_PSTATS
  int _pstats_gpu_thread;
  bool _timer_queries_active;
  PStatFrameData _pstats_gpu_data;

  int _last_query_frame;
  int _last_num_queried;
  // double _timer_delta;
  typedef pdeque<PT(TimerQueryContext)> TimerQueryQueue;
  TimerQueryQueue _pending_timer_queries;
#endif

  bool _copy_texture_inverted;
  bool _supports_multisample;
  bool _supports_generate_mipmap;
  bool _supports_depth_texture;
  bool _supports_depth_stencil;
  bool _supports_luminance_texture;
  bool _supports_shadow_filter;
  bool _supports_sampler_objects;
  bool _supports_basic_shaders;
  bool _supports_geometry_shaders;
  bool _supports_tessellation_shaders;
  bool _supports_compute_shaders;
  bool _supports_glsl;
  bool _supports_hlsl;
  bool _supports_framebuffer_multisample;
  bool _supports_framebuffer_blit;

  bool _supports_stencil;
  bool _supports_stencil_wrap;
  bool _supports_two_sided_stencil;
  bool _supports_geometry_instancing;
  bool _supports_indirect_draw;

  int _max_color_targets;
  bool _supports_dual_source_blending;

  int  _supported_geom_rendering;
  bool _color_scale_via_lighting;
  bool _alpha_scale_via_texture;
  bool _runtime_color_scale;

  int  _stereo_buffer_mask;

  ShaderModel _auto_detect_shader_model;
  ShaderModel _shader_model;

  static PT(TextureStage) _alpha_scale_texture_stage;

  Shader::ShaderCaps _shader_caps;

  PN_stdfloat _gamma;
  Texture::QualityLevel _texture_quality_override;

  PT(ShaderGenerator) _shader_generator;

#ifndef NDEBUG
  PT(Texture) _flash_texture;
#endif

public:
  // Statistics
  static PStatCollector _vertex_buffer_switch_pcollector;
  static PStatCollector _index_buffer_switch_pcollector;
  static PStatCollector _shader_buffer_switch_pcollector;
  static PStatCollector _load_vertex_buffer_pcollector;
  static PStatCollector _load_index_buffer_pcollector;
  static PStatCollector _load_shader_buffer_pcollector;
  static PStatCollector _create_vertex_buffer_pcollector;
  static PStatCollector _create_index_buffer_pcollector;
  static PStatCollector _create_shader_buffer_pcollector;
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
  static PStatCollector _flush_pcollector;
  static PStatCollector _compute_dispatch_pcollector;
  static PStatCollector _wait_occlusion_pcollector;
  static PStatCollector _wait_timer_pcollector;
  static PStatCollector _timer_queries_pcollector;
  static PStatCollector _command_latency_pcollector;

  static PStatCollector _prepare_pcollector;
  static PStatCollector _prepare_texture_pcollector;
  static PStatCollector _prepare_sampler_pcollector;
  static PStatCollector _prepare_geom_pcollector;
  static PStatCollector _prepare_shader_pcollector;
  static PStatCollector _prepare_vertex_buffer_pcollector;
  static PStatCollector _prepare_index_buffer_pcollector;
  static PStatCollector _prepare_shader_buffer_pcollector;

  // A whole slew of collectors to measure the cost of individual state
  // changes.  These are disabled by default.
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

EXPCL_PANDA_DISPLAY std::ostream &operator << (std::ostream &out, GraphicsStateGuardian::ShaderModel sm);

#include "graphicsStateGuardian.I"

#endif
