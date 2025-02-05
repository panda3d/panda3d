/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsStateGuardian.cxx
 * @author drose
 * @date 1999-02-02
 * @author fperazzi, PandaSE
 * @date 2010-05-05
 *  _max_2d_texture_array_layers, _supports_2d_texture_array,
 *  get_supports_cg_profile)
 */

#include "graphicsStateGuardian.h"
#include "graphicsEngine.h"
#include "config_display.h"
#include "textureContext.h"
#include "vertexBufferContext.h"
#include "indexBufferContext.h"
#include "renderBuffer.h"
#include "light.h"
#include "planeNode.h"
#include "throw_event.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "pStatGPUTimer.h"
#include "geomTristrips.h"
#include "geomTrifans.h"
#include "geomLinestrips.h"
#include "colorWriteAttrib.h"
#include "shader.h"
#include "pnotify.h"
#include "drawableRegion.h"
#include "displayRegion.h"
#include "graphicsOutput.h"
#include "texturePool.h"
#include "geomMunger.h"
#include "stateMunger.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "sphereLight.h"
#include "spotlight.h"
#include "textureReloadRequest.h"
#include "shaderAttrib.h"
#include "materialAttrib.h"
#include "depthWriteAttrib.h"
#include "lightAttrib.h"
#include "texGenAttrib.h"
#include "shaderGenerator.h"
#include "lightLensNode.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "clipPlaneAttrib.h"
#include "fogAttrib.h"
#include "renderModeAttrib.h"
#include "config_pstatclient.h"

#include <limits.h>

using std::string;

static const LMatrix4 shadow_bias_mat(0.5f, 0.0f, 0.0f, 0.0f,
                                      0.0f, 0.5f, 0.0f, 0.0f,
                                      0.0f, 0.0f, 0.5f, 0.0f,
                                      0.5f, 0.5f, 0.5f, 1.0f);

//PStatCollector GraphicsStateGuardian::_vertex_buffer_switch_pcollector("Buffer switch:Vertex");
//PStatCollector GraphicsStateGuardian::_index_buffer_switch_pcollector("Buffer switch:Index");
//PStatCollector GraphicsStateGuardian::_shader_buffer_switch_pcollector("Buffer switch:Shader");
PStatCollector GraphicsStateGuardian::_load_vertex_buffer_pcollector("Draw:Transfer data:Vertex buffer");
PStatCollector GraphicsStateGuardian::_load_index_buffer_pcollector("Draw:Transfer data:Index buffer");
PStatCollector GraphicsStateGuardian::_load_shader_buffer_pcollector("Draw:Transfer data:Shader buffer");
PStatCollector GraphicsStateGuardian::_create_vertex_buffer_pcollector("Draw:Transfer data:Create Vertex buffer");
PStatCollector GraphicsStateGuardian::_create_index_buffer_pcollector("Draw:Transfer data:Create Index buffer");
PStatCollector GraphicsStateGuardian::_create_shader_buffer_pcollector("Draw:Transfer data:Create Shader buffer");
PStatCollector GraphicsStateGuardian::_load_texture_pcollector("Draw:Transfer data:Texture");
PStatCollector GraphicsStateGuardian::_data_transferred_pcollector("Data transferred");
PStatCollector GraphicsStateGuardian::_texmgrmem_total_pcollector("Texture manager");
PStatCollector GraphicsStateGuardian::_texmgrmem_resident_pcollector("Texture manager:Resident");
PStatCollector GraphicsStateGuardian::_primitive_batches_pcollector("Primitive batches");
PStatCollector GraphicsStateGuardian::_primitive_batches_tristrip_pcollector("Primitive batches:Triangle strips");
PStatCollector GraphicsStateGuardian::_primitive_batches_trifan_pcollector("Primitive batches:Triangle fans");
PStatCollector GraphicsStateGuardian::_primitive_batches_tri_pcollector("Primitive batches:Triangles");
PStatCollector GraphicsStateGuardian::_primitive_batches_patch_pcollector("Primitive batches:Patches");
PStatCollector GraphicsStateGuardian::_primitive_batches_other_pcollector("Primitive batches:Other");
PStatCollector GraphicsStateGuardian::_vertices_tristrip_pcollector("Vertices:Triangle strips");
PStatCollector GraphicsStateGuardian::_vertices_trifan_pcollector("Vertices:Triangle fans");
PStatCollector GraphicsStateGuardian::_vertices_tri_pcollector("Vertices:Triangles");
PStatCollector GraphicsStateGuardian::_vertices_patch_pcollector("Vertices:Patches");
PStatCollector GraphicsStateGuardian::_vertices_other_pcollector("Vertices:Other");
PStatCollector GraphicsStateGuardian::_state_pcollector("State changes");
PStatCollector GraphicsStateGuardian::_transform_state_pcollector("State changes:Transforms");
PStatCollector GraphicsStateGuardian::_texture_state_pcollector("State changes:Textures");
PStatCollector GraphicsStateGuardian::_draw_primitive_pcollector("Draw:Primitive:Draw");
PStatCollector GraphicsStateGuardian::_draw_set_state_pcollector("Draw:Set State");
PStatCollector GraphicsStateGuardian::_flush_pcollector("Draw:Flush");
PStatCollector GraphicsStateGuardian::_compute_dispatch_pcollector("Draw:Compute dispatch");
PStatCollector GraphicsStateGuardian::_compute_work_groups_pcollector("Compute work groups");

PStatCollector GraphicsStateGuardian::_wait_occlusion_pcollector("Wait:Occlusion");
PStatCollector GraphicsStateGuardian::_wait_timer_pcollector("Wait:Timer Queries");
PStatCollector GraphicsStateGuardian::_timer_queries_pcollector("Timer queries");

PStatCollector GraphicsStateGuardian::_prepare_pcollector("Draw:Prepare");
PStatCollector GraphicsStateGuardian::_prepare_texture_pcollector("Draw:Prepare:Texture");
PStatCollector GraphicsStateGuardian::_prepare_sampler_pcollector("Draw:Prepare:Sampler");
PStatCollector GraphicsStateGuardian::_prepare_geom_pcollector("Draw:Prepare:Geom");
PStatCollector GraphicsStateGuardian::_prepare_shader_pcollector("Draw:Prepare:Shader");
PStatCollector GraphicsStateGuardian::_prepare_vertex_buffer_pcollector("Draw:Prepare:Vertex buffer");
PStatCollector GraphicsStateGuardian::_prepare_index_buffer_pcollector("Draw:Prepare:Index buffer");
PStatCollector GraphicsStateGuardian::_prepare_shader_buffer_pcollector("Draw:Prepare:Shader buffer");

PStatCollector GraphicsStateGuardian::_draw_set_state_transform_pcollector("Draw:Set State:Transform");
PStatCollector GraphicsStateGuardian::_draw_set_state_alpha_test_pcollector("Draw:Set State:Alpha test");
PStatCollector GraphicsStateGuardian::_draw_set_state_antialias_pcollector("Draw:Set State:Antialias");
PStatCollector GraphicsStateGuardian::_draw_set_state_clip_plane_pcollector("Draw:Set State:Clip plane");
PStatCollector GraphicsStateGuardian::_draw_set_state_color_pcollector("Draw:Set State:Color");
PStatCollector GraphicsStateGuardian::_draw_set_state_cull_face_pcollector("Draw:Set State:Cull face");
PStatCollector GraphicsStateGuardian::_draw_set_state_depth_offset_pcollector("Draw:Set State:Depth offset");
PStatCollector GraphicsStateGuardian::_draw_set_state_depth_test_pcollector("Draw:Set State:Depth test");
PStatCollector GraphicsStateGuardian::_draw_set_state_depth_write_pcollector("Draw:Set State:Depth write");
PStatCollector GraphicsStateGuardian::_draw_set_state_render_mode_pcollector("Draw:Set State:Render mode");
PStatCollector GraphicsStateGuardian::_draw_set_state_rescale_normal_pcollector("Draw:Set State:Rescale normal");
PStatCollector GraphicsStateGuardian::_draw_set_state_shade_model_pcollector("Draw:Set State:Shade model");
PStatCollector GraphicsStateGuardian::_draw_set_state_blending_pcollector("Draw:Set State:Blending");
PStatCollector GraphicsStateGuardian::_draw_set_state_shader_pcollector("Draw:Set State:Shader");
PStatCollector GraphicsStateGuardian::_draw_set_state_shader_parameters_pcollector("Draw:Set State:Shader Parameters");
PStatCollector GraphicsStateGuardian::_draw_set_state_texture_pcollector("Draw:Set State:Texture");
PStatCollector GraphicsStateGuardian::_draw_set_state_tex_matrix_pcollector("Draw:Set State:Tex matrix");
PStatCollector GraphicsStateGuardian::_draw_set_state_tex_gen_pcollector("Draw:Set State:Tex gen");
PStatCollector GraphicsStateGuardian::_draw_set_state_material_pcollector("Draw:Set State:Material");
PStatCollector GraphicsStateGuardian::_draw_set_state_light_pcollector("Draw:Set State:Light");
PStatCollector GraphicsStateGuardian::_draw_set_state_stencil_pcollector("Draw:Set State:Stencil");
PStatCollector GraphicsStateGuardian::_draw_set_state_fog_pcollector("Draw:Set State:Fog");
PStatCollector GraphicsStateGuardian::_draw_set_state_scissor_pcollector("Draw:Set State:Scissor");

PT(TextureStage) GraphicsStateGuardian::_alpha_scale_texture_stage = nullptr;

TypeHandle GraphicsStateGuardian::_type_handle;

/**
 *
 */

GraphicsStateGuardian::
GraphicsStateGuardian(CoordinateSystem internal_coordinate_system,
                      GraphicsEngine *engine, GraphicsPipe *pipe) :
  _internal_coordinate_system(internal_coordinate_system),
  _pipe(pipe),
  _engine(engine)
{
  _coordinate_system = CS_invalid;
  _internal_transform = TransformState::make_identity();

  if (_internal_coordinate_system == CS_default) {
    _internal_coordinate_system = get_default_coordinate_system();
  }

  set_coordinate_system(get_default_coordinate_system());

  _data_reader = nullptr;
  _current_display_region = nullptr;
  _current_stereo_channel = Lens::SC_mono;
  _current_tex_view_offset = 0;
  _current_lens = nullptr;
  _projection_mat = TransformState::make_identity();
  _projection_mat_inv = TransformState::make_identity();

  _needs_reset = true;
  _is_valid = false;
  _current_properties = nullptr;
  _closing_gsg = false;
  _active = true;
  _prepared_objects = new PreparedGraphicsObjects;
  _stereo_buffer_mask = ~0;
  _incomplete_render = allow_incomplete_render;
  _effective_incomplete_render = false;
  _loader = Loader::get_global_ptr();

  _is_hardware = false;
  _prefers_triangle_strips = false;
  _max_vertices_per_array = INT_MAX;
  _max_vertices_per_primitive = INT_MAX;

  // Initially, we set this to 1 (the default--no multitexturing supported).
  // A derived GSG may set this differently if it supports multitexturing.
  _max_texture_stages = 1;

  // Also initially, we assume there are no limits on texture sizes, and that
  // 3-d and cube-map textures are not supported.
  _max_texture_dimension = -1;
  _max_3d_texture_dimension = 0;
  _max_2d_texture_array_layers = 0;
  _max_cube_map_dimension = 0;
  _max_buffer_texture_size = 0;

  // Assume we don't support these fairly advanced texture combiner modes.
  _supports_texture_combine = false;
  _supports_texture_saved_result = false;
  _supports_texture_dot3 = false;

  _supports_3d_texture = false;
  _supports_2d_texture_array = false;
  _supports_cube_map = false;
  _supports_buffer_texture = false;
  _supports_cube_map_array = false;
  _supports_tex_non_pow2 = false;
  _supports_texture_srgb = false;
  _supports_compressed_texture = false;
  _compressed_texture_formats.clear();
  _compressed_texture_formats.set_bit(Texture::CM_off);

  // Assume no limits on number of lights or clip planes.
  _max_lights = -1;
  _max_clip_planes = -1;

  // Assume no vertex blending capability.
  _max_vertex_transforms = 0;
  _max_vertex_transform_indices = 0;

  _supports_occlusion_query = false;
  _supports_timer_query = false;

#ifdef DO_PSTATS
  _timer_queries_active = false;
  _pstats_gpu_thread = -1;
#endif

  // Initially, we set this to false; a GSG that knows it has this property
  // should set it to true.
  _copy_texture_inverted = false;

  // Similarly with these capabilities flags.
  _supports_multisample = false;
  _supports_generate_mipmap = false;
  _supports_depth_texture = false;
  _supports_depth_stencil = false;
  _supports_shadow_filter = false;
  _supports_sampler_objects = false;
  _supports_basic_shaders = false;
  _supports_geometry_shaders = false;
  _supports_tessellation_shaders = false;
  _supports_glsl = false;
  _supports_hlsl = false;

  _max_compute_work_group_count = LVecBase3i(0, 0, 0);
  _max_compute_work_group_size = LVecBase3i(0, 0, 0);
  _max_compute_work_group_invocations = 0;

  _supports_stencil = false;
  _supports_stencil_wrap = false;
  _supports_two_sided_stencil = false;
  _supports_geometry_instancing = false;
  _supports_indirect_draw = false;

  // We are safe assuming it has luminance support
  _supports_luminance_texture = true;

  // Assume a maximum of 1 render target in absence of MRT.
  _max_color_targets = 1;
  _supports_dual_source_blending = false;

  _supported_geom_rendering = 0;

  // If this is true, then we can apply a color andor color scale by twiddling
  // the material andor ambient light (which could mean enabling lighting even
  // without a LightAttrib).
  _color_scale_via_lighting = color_scale_via_lighting;

  // Similarly for applying a texture to achieve uniform alpha scaling.
  _alpha_scale_via_texture = alpha_scale_via_texture;

  // Few GSG's can do this, since it requires touching each vertex as it is
  // rendered.
  _runtime_color_scale = false;

  // The default is no shader support.
  _auto_detect_shader_model = SM_00;
  _shader_model = SM_00;

  _gamma = 1.0f;
  _texture_quality_override = Texture::QL_default;

  // Give it a unique identifier.  Unlike a pointer, we can guarantee that
  // this value will never be reused.
  static size_t next_index = 0;
  _id = next_index++;
}

/**
 *
 */
GraphicsStateGuardian::
~GraphicsStateGuardian() {
  remove_gsg(this);
  GeomMunger::unregister_mungers_for_gsg(this);

  // Remove the munged states for this GSG.  This requires going through all
  // states, although destructing a GSG should be rare enough for this not to
  // matter too much.
  // Note that if uniquify-states is false, we can't iterate over all the
  // states, and some GSGs will linger.  Let's hope this isn't a problem.
  LightReMutexHolder holder(*RenderState::_states_lock);
  size_t size = RenderState::_states.get_num_entries();
  for (size_t si = 0; si < size; ++si) {
    const RenderState *state = RenderState::_states.get_key(si);
    state->_mungers.remove(_id);
    state->_munged_states.remove(_id);
  }
}

/**
 * Returns the graphics engine that created this GSG. Since there is normally
 * only one GraphicsEngine object in an application, this is usually the same
 * as the global GraphicsEngine.
 */
GraphicsEngine *GraphicsStateGuardian::
get_engine() const {
  nassertr(_engine != nullptr, GraphicsEngine::get_global_ptr());
  return _engine;
}

/**
 * Returns true if this particular GSG supports using the multisample bits to
 * provide antialiasing, and also supports M_multisample and
 * M_multisample_mask transparency modes.  If this is not true for a
 * particular GSG, Panda will map the M_multisample modes to M_binary.
 *
 * This method is declared virtual solely so that it can be queried from
 * cullResult.cxx.
 */
bool GraphicsStateGuardian::
get_supports_multisample() const {
  return _supports_multisample;
}

/**
 * Returns the union of Geom::GeomRendering values that this particular GSG
 * can support directly.  If a Geom needs to be rendered that requires some
 * additional properties, the StandardMunger and/or the CullableObject will
 * convert it as needed.
 *
 * This method is declared virtual solely so that it can be queried from
 * cullableObject.cxx.
 */
int GraphicsStateGuardian::
get_supported_geom_rendering() const {
  return _supported_geom_rendering;
}

/**
 * Returns true if this particular GSG supports the specified Cg Shader
 * Profile.
 */
bool GraphicsStateGuardian::
get_supports_cg_profile(const string &name) const {
  return false;
}

/**
 * Changes the coordinate system in effect on this particular gsg.  This is
 * also called the "external" coordinate system, since it is the coordinate
 * system used by the scene graph, external to to GSG.
 *
 * Normally, this will be the default coordinate system, but it might be set
 * differently at runtime.  It will automatically be copied from the current
 * lens's coordinate system as each DisplayRegion is rendered.
 */
void GraphicsStateGuardian::
set_coordinate_system(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  if (_coordinate_system == cs) {
    return;
  }
  _coordinate_system = cs;

  // Changing the external coordinate system changes the cs_transform.
  if (_internal_coordinate_system == CS_default ||
      _internal_coordinate_system == _coordinate_system) {
    _cs_transform = TransformState::make_identity();
    _inv_cs_transform = TransformState::make_identity();

  } else {
    _cs_transform =
      TransformState::make_mat
      (LMatrix4::convert_mat(_coordinate_system,
                              _internal_coordinate_system));
    _inv_cs_transform =
      TransformState::make_mat
      (LMatrix4::convert_mat(_internal_coordinate_system,
                              _coordinate_system));
  }
}

/**
 * Returns the coordinate system used internally by the GSG.  This may be the
 * same as the external coordinate system reported by get_coordinate_system(),
 * or it may be something different.
 *
 * In any case, vertices that have been transformed before being handed to the
 * GSG (that is, vertices with a contents value of C_clip_point) will be
 * expected to be in this coordinate system.
 */
CoordinateSystem GraphicsStateGuardian::
get_internal_coordinate_system() const {
  return _internal_coordinate_system;
}

/**
 * Returns the set of texture and geom objects that have been prepared with
 * this GSG (and possibly other GSG's that share objects).
 */
PreparedGraphicsObjects *GraphicsStateGuardian::
get_prepared_objects() {
  return _prepared_objects;
}

/**
 * Set gamma.  Returns true on success.
 */
bool GraphicsStateGuardian::
set_gamma(PN_stdfloat gamma) {
  _gamma = gamma;

  return false;
}

/**
 * Get the current gamma setting.
 */
PN_stdfloat GraphicsStateGuardian::
get_gamma() const {
  return _gamma;
}

/**
 * Restore original gamma setting.
 */
void GraphicsStateGuardian::
restore_gamma() {
}

/**
 * Calls the indicated function on all currently-prepared textures, or until
 * the callback function returns false.
 */
void GraphicsStateGuardian::
traverse_prepared_textures(GraphicsStateGuardian::TextureCallback *func,
                           void *callback_arg) {
  ReMutexHolder holder(_prepared_objects->_lock);
  PreparedGraphicsObjects::Textures::const_iterator ti;
  for (ti = _prepared_objects->_prepared_textures.begin();
       ti != _prepared_objects->_prepared_textures.end();
       ++ti) {
    bool result = (*func)(*ti, callback_arg);
    if (!result) {
      return;
    }
  }
}

/**
 * Sets the "flash texture".  This is a debug feature; when enabled, the
 * specified texture will begin flashing in the scene, helping you to find it
 * visually.
 *
 * The texture also flashes with a color code: blue for mipmap level 0, yellow
 * for mipmap level 1, and red for mipmap level 2 or higher (even for textures
 * that don't have mipmaps).  This gives you an idea of the choice of the
 * texture size.  If it is blue, the texture is being drawn the proper size or
 * magnified; if it is yellow, it is being minified a little bit; and if it
 * red, it is being minified considerably.  If you see a red texture when you
 * are right in front of it, you should consider reducing the size of the
 * texture to avoid wasting texture memory.
 *
 * Not all rendering backends support the flash_texture feature.  Presently,
 * it is only supported by OpenGL.
 */
void GraphicsStateGuardian::
set_flash_texture(Texture *tex) {
#ifndef NDEBUG
  _flash_texture = tex;
#endif
}

/**
 * Resets the "flash texture", so that no textures will flash.  See
 * set_flash_texture().
 */
void GraphicsStateGuardian::
clear_flash_texture() {
#ifndef NDEBUG
  _flash_texture = nullptr;
#endif
}

/**
 * Returns the current "flash texture", if any, or NULL if none.  See
 * set_flash_texture().
 */
Texture *GraphicsStateGuardian::
get_flash_texture() const {
#ifndef NDEBUG
  return _flash_texture;
#else
  return nullptr;
#endif
}

/**
 * Sets the SceneSetup object that indicates the initial camera position, etc.
 * This must be called before traversal begins.  Returns true if the scene is
 * acceptable, false if something's wrong.  This should be called in the draw
 * thread only.
 */
bool GraphicsStateGuardian::
set_scene(SceneSetup *scene_setup) {
  _scene_setup = scene_setup;
  _current_lens = scene_setup->get_lens();
  if (_current_lens == nullptr) {
    return false;
  }

  set_coordinate_system(_current_lens->get_coordinate_system());

  _projection_mat = calc_projection_mat(_current_lens);
  if (_projection_mat == nullptr) {
    return false;
  }
  _projection_mat_inv = _projection_mat->get_inverse();
  return prepare_lens();
}

/**
 * Returns the current SceneSetup object.
 */
SceneSetup *GraphicsStateGuardian::
get_scene() const {
  return _scene_setup;
}

/**
 * Creates whatever structures the GSG requires to represent the texture
 * internally, and returns a newly-allocated TextureContext object with this
 * data.  It is the responsibility of the calling function to later call
 * release_texture() with this same pointer (which will also delete the
 * pointer).
 *
 * This function should not be called directly to prepare a texture.  Instead,
 * call Texture::prepare().
 */
TextureContext *GraphicsStateGuardian::
prepare_texture(Texture *) {
  return nullptr;
}

/**
 * Ensures that the current Texture data is refreshed onto the GSG.  This
 * means updating the texture properties and/or re-uploading the texture
 * image, if necessary.  This should only be called within the draw thread.
 *
 * If force is true, this function will not return until the texture has been
 * fully uploaded.  If force is false, the function may choose to upload a
 * simple version of the texture instead, if the texture is not fully resident
 * (and if get_incomplete_render() is true).
 */
bool GraphicsStateGuardian::
update_texture(TextureContext *, bool) {
  return true;
}

/**
 * Ensures that the current Texture data is refreshed onto the GSG.  This
 * means updating the texture properties and/or re-uploading the texture
 * image, if necessary.  This should only be called within the draw thread.
 *
 * If force is true, this function will not return until the texture has been
 * fully uploaded.  If force is false, the function may choose to upload a
 * simple version of the texture instead, if the texture is not fully resident
 * (and if get_incomplete_render() is true).
 */
bool GraphicsStateGuardian::
update_texture(TextureContext *tc, bool force, CompletionToken token) {
  bool result = update_texture(tc, force);
  token.complete(result);
  return result;
}

/**
 * Frees the resources previously allocated via a call to prepare_texture(),
 * including deleting the TextureContext itself, if it is non-NULL.
 */
void GraphicsStateGuardian::
release_texture(TextureContext *) {
}

/**
 * Frees the resources previously allocated via a call to prepare_texture(),
 * including deleting the TextureContext itself, if it is non-NULL.
 */
void GraphicsStateGuardian::
release_textures(const pvector<TextureContext *> &contexts) {
  for (TextureContext *tc : contexts) {
    release_texture(tc);
  }
}

/**
 * This method should only be called by the GraphicsEngine.  Do not call it
 * directly; call GraphicsEngine::extract_texture_data() instead.
 *
 * This method will be called in the draw thread to download the texture
 * memory's image into its ram_image value.  It returns true on success, false
 * otherwise.
 */
bool GraphicsStateGuardian::
extract_texture_data(Texture *) {
  return false;
}

/**
 * Creates whatever structures the GSG requires to represent the sampler
 * internally, and returns a newly-allocated SamplerContext object with this
 * data.  It is the responsibility of the calling function to later call
 * release_sampler() with this same pointer (which will also delete the
 * pointer).
 *
 * This function should not be called directly to prepare a sampler.  Instead,
 * call Texture::prepare().
 */
SamplerContext *GraphicsStateGuardian::
prepare_sampler(const SamplerState &sampler) {
  return nullptr;
}

/**
 * Frees the resources previously allocated via a call to prepare_sampler(),
 * including deleting the SamplerContext itself, if it is non-NULL.
 */
void GraphicsStateGuardian::
release_sampler(SamplerContext *) {
}

/**
 * Prepares the indicated Geom for retained-mode rendering, by creating
 * whatever structures are necessary in the GSG (for instance, vertex
 * buffers). Returns the newly-allocated GeomContext that can be used to
 * render the geom.
 */
GeomContext *GraphicsStateGuardian::
prepare_geom(Geom *) {
  return nullptr;
}

/**
 * Frees the resources previously allocated via a call to prepare_geom(),
 * including deleting the GeomContext itself, if it is non-NULL.
 *
 * This function should not be called directly to prepare a Geom.  Instead,
 * call Geom::prepare().
 */
void GraphicsStateGuardian::
release_geom(GeomContext *) {
}

/**
 * Compile a vertex/fragment shader body.
 */
ShaderContext *GraphicsStateGuardian::
prepare_shader(Shader *shader) {
  return nullptr;
}

/**
 * Releases the resources allocated by prepare_shader
 */
void GraphicsStateGuardian::
release_shader(ShaderContext *sc) {
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
VertexBufferContext *GraphicsStateGuardian::
prepare_vertex_buffer(GeomVertexArrayData *) {
  return nullptr;
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the VertexBufferContext itself, if necessary.
 */
void GraphicsStateGuardian::
release_vertex_buffer(VertexBufferContext *) {
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the VertexBufferContext itself, if necessary.
 */
void GraphicsStateGuardian::
release_vertex_buffers(const pvector<BufferContext *> &contexts) {
  for (BufferContext *bc : contexts) {
    release_vertex_buffer((VertexBufferContext *)bc);
  }
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
IndexBufferContext *GraphicsStateGuardian::
prepare_index_buffer(GeomPrimitive *) {
  return nullptr;
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the IndexBufferContext itself, if necessary.
 */
void GraphicsStateGuardian::
release_index_buffer(IndexBufferContext *) {
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the IndexBufferContext itself, if necessary.
 */
void GraphicsStateGuardian::
release_index_buffers(const pvector<BufferContext *> &contexts) {
  for (BufferContext *bc : contexts) {
    release_index_buffer((IndexBufferContext *)bc);
  }
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
BufferContext *GraphicsStateGuardian::
prepare_shader_buffer(ShaderBuffer *) {
  return nullptr;
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the BufferContext itself, if necessary.
 */
void GraphicsStateGuardian::
release_shader_buffer(BufferContext *) {
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the BufferContext itself, if necessary.
 */
void GraphicsStateGuardian::
release_shader_buffers(const pvector<BufferContext *> &contexts) {
  for (BufferContext *bc : contexts) {
    release_shader_buffer(bc);
  }
}

/**
 * This method should only be called by the GraphicsEngine.  Do not call it
 * directly; call GraphicsEngine::extract_texture_data() instead.
 *
 * This method will be called in the draw thread to download the buffer's
 * current contents synchronously.
 */
bool GraphicsStateGuardian::
extract_shader_buffer_data(ShaderBuffer *buffer, vector_uchar &data) {
  return false;
}

/**
 * Begins a new occlusion query.  After this call, you may call
 * begin_draw_primitives() and draw_triangles()/draw_whatever() repeatedly.
 * Eventually, you should call end_occlusion_query() before the end of the
 * frame; that will return a new OcclusionQueryContext object that will tell
 * you how many pixels represented by the bracketed geometry passed the depth
 * test.
 *
 * It is not valid to call begin_occlusion_query() between another
 * begin_occlusion_query() .. end_occlusion_query() sequence.
 */
void GraphicsStateGuardian::
begin_occlusion_query() {
  nassertv(_current_occlusion_query == nullptr);
}

/**
 * Ends a previous call to begin_occlusion_query(). This call returns the
 * OcclusionQueryContext object that will (eventually) report the number of
 * pixels that passed the depth test between the call to
 * begin_occlusion_query() and end_occlusion_query().
 */
PT(OcclusionQueryContext) GraphicsStateGuardian::
end_occlusion_query() {
  nassertr(_current_occlusion_query != nullptr, nullptr);
  PT(OcclusionQueryContext) result = _current_occlusion_query;
  _current_occlusion_query = nullptr;
  return result;
}

/**
 * Adds a timer query to the command stream, associated with the given PStats
 * collector index.
 */
void GraphicsStateGuardian::
issue_timer_query(int pstats_index) {
}

/**
 * A latency query is a special type of timer query that measures the
 * difference between CPU time and GPU time, ie. how far the GPU is behind in
 * processing the commands being generated by the CPU right now.
 */
void GraphicsStateGuardian::
issue_latency_query(int pstats_index) {
}

/**
 * Dispatches a currently bound compute shader using the given work group
 * counts.
 */
void GraphicsStateGuardian::
dispatch_compute(int num_groups_x, int num_groups_y, int num_groups_z) {
  nassert_raise("Compute shaders not supported by GSG");
}

/**
 * Looks up or creates a GeomMunger object to munge vertices appropriate to
 * this GSG for the indicated state.
 */
PT(GeomMunger) GraphicsStateGuardian::
get_geom_munger(const RenderState *state, Thread *current_thread) {
  RenderState::Mungers &mungers = state->_mungers;

  if (!mungers.is_empty()) {
    // Before we even look up the map, see if the _last_mi value points to
    // this GSG.  This is likely because we tend to visit the same state
    // multiple times during a frame.  Also, this might well be the only GSG
    // in the world anyway.
    int mi = state->_last_mi;
    if (mi >= 0 && (size_t)mi < mungers.get_num_entries() && mungers.get_key(mi) == _id) {
      PT(GeomMunger) munger = mungers.get_data(mi);
      if (munger->is_registered()) {
        return munger;
      }
    }

    // Nope, we have to look it up in the map.
    mi = mungers.find(_id);
    if (mi >= 0) {
      PT(GeomMunger) munger = mungers.get_data(mi);
      if (munger->is_registered()) {
        state->_last_mi = mi;
        return munger;
      } else {
        // This GeomMunger is no longer registered.  Remove it from the map.
        mungers.remove_element(mi);
      }
    }
  }

  // Nothing in the map; create a new entry.
  PT(GeomMunger) munger = make_geom_munger(state, current_thread);
  nassertr(munger != nullptr && munger->is_registered(), munger);
  nassertr(munger->is_of_type(StateMunger::get_class_type()), munger);

  state->_last_mi = mungers.store(_id, munger);
  return munger;
}

/**
 * Creates a new GeomMunger object to munge vertices appropriate to this GSG
 * for the indicated state.
 */
PT(GeomMunger) GraphicsStateGuardian::
make_geom_munger(const RenderState *state, Thread *current_thread) {
  // The default implementation returns no munger at all, but presumably,
  // every kind of GSG needs some special munging action, so real GSG's will
  // override this to return something more useful.
  return nullptr;
}

/**
 * This function will compute the distance to the indicated point, assumed to
 * be in eye coordinates, from the camera plane.  The point is assumed to be
 * in the GSG's internal coordinate system.
 */
PN_stdfloat GraphicsStateGuardian::
compute_distance_to(const LPoint3 &point) const {
  switch (_internal_coordinate_system) {
  case CS_zup_right:
    return point[1];

  case CS_yup_right:
    return -point[2];

  case CS_zup_left:
    return -point[1];

  case CS_yup_left:
    return point[2];

  default:
    gsg_cat.error()
      << "Invalid coordinate system in compute_distance_to: "
      << (int)_internal_coordinate_system << "\n";
    return 0.0f;
  }
}

/**
 * A shader can request a number of values from the current render state.  These
 * are stored in the form of a matrix.  Each ShaderContext caches the current
 * value of these matrices, and calls this routine to update the matrices that
 * have changed based on the aspects of the render state that were altered.
 */
void GraphicsStateGuardian::
update_shader_matrix_cache(Shader *shader, LVecBase4f *cache, int altered) {
  for (Shader::ShaderMatPart &part : shader->_mat_parts) {
    if (altered & part._dep) {
      fetch_specified_part(part._part, part._arg, cache, part._count);
    }
    cache += part._count * part._size;
  }
}

/**
 * The gsg contains a large number of useful matrices:
 *
 * - the world transform,
 * - the modelview matrix,
 * - the cs_transform,
 * - etc, etc.
 *
 * A shader can request any of these values, and furthermore, it can request
 * that various compositions, inverses, and transposes be performed.  The
 * ShaderMatSpec is a data structure indicating what datum is desired and what
 * conversions to perform.  This routine, fetch_specified_value, is
 * responsible for doing the actual retrieval and conversions.
 *
 * Some values, like the following, aren't matrices:
 *
 * - window size
 * - texture coordinates of card center
 *
 * This routine can fetch these values as well, by shoehorning them into a
 * matrix.  In this way, we avoid the need for a separate routine to fetch
 * these values.
 */
const LVecBase4f *GraphicsStateGuardian::
fetch_specified_value(Shader::ShaderMatSpec &spec, const LVecBase4f *cache, LVecBase4f *scratch) {
  LVecBase3f v;

  const LVecBase4f *cache0 = cache + spec._cache_offset[0];
  const LVecBase4f *cache1 = cache + spec._cache_offset[1];

  LMatrix4f &m = *(LMatrix4f *)scratch;

  switch (spec._func) {
  case Shader::SMF_first:
    return cache0;

  case Shader::SMF_compose:
    m.multiply(*(LMatrix4f *)cache0, *(LMatrix4f *)cache1);
    return (LVecBase4f *)&m;

  case Shader::SMF_transform_dlight:
    m = *(LMatrix4f *)cache0;
    v = (*(LMatrix4f *)cache1).xform_vec(cache0[2].get_xyz());
    v.normalize();
    m.set_row(2, v);
    v = (*(LMatrix4f *)cache1).xform_vec(cache0[3].get_xyz());
    v.normalize();
    m.set_row(3, v);
    return (LVecBase4f *)&m;

  case Shader::SMF_transform_plight:
    {
      // Careful not to touch the w component, which contains the near value.
      m = *(LMatrix4f *)cache0;
      LPoint3f point = (*(LMatrix4f *)cache1).xform_point(cache0[2].get_xyz());
      m(2, 0) = point[0];
      m(2, 1) = point[1];
      m(2, 2) = point[2];
      return (LVecBase4f *)&m;
    }

  case Shader::SMF_transform_slight:
    m = *(LMatrix4f *)cache0;
    m.set_row(2, (*(LMatrix4f *)cache1).xform_point(cache0[2].get_xyz()));
    v = (*(LMatrix4f *)cache1).xform_vec(cache0[3].get_xyz());
    v.normalize();
    m.set_row(3, v);
    return (LVecBase4f *)&m;

  case Shader::SMF_shader_input_ptr:
    return (const LVecBase4f *)fetch_ptr_parameter(spec, scratch);
  }

  // Should never get here
  m = LMatrix4f::ident_mat();
  return (LVecBase4f *)&m;
}

/**
 * Fetches a numeric shader input, doing conversion as necessary using the
 * given amount of scratch space.
 */
const void *GraphicsStateGuardian::
fetch_ptr_parameter(Shader::ShaderMatSpec &spec, LVecBase4f *scratch) {
  Shader::ShaderPtrData ptr_data;
  if (!_target_shader->get_shader_input_ptr(spec._arg[0], ptr_data)) {
    return nullptr;
  }

  nassertr(spec._num_components > 0, nullptr);

  int array_size = std::min(spec._array_count, (int)ptr_data._size / spec._num_components);
  switch (spec._numeric_type) {
  case Shader::SPT_float:
    {
      float *data = (float *)scratch;

      switch (ptr_data._type) {
      case Shader::SPT_int:
        // Convert int data to float data.
        for (int i = 0; i < (array_size * spec._num_components); ++i) {
          data[i] = (float)(((int*)ptr_data._ptr)[i]);
        }
        return data;

      case Shader::SPT_uint:
        // Convert unsigned int data to float data.
        for (int i = 0; i < (array_size * spec._num_components); ++i) {
          data[i] = (float)(((unsigned int*)ptr_data._ptr)[i]);
        }
        return data;

      case Shader::SPT_double:
        // Downgrade double data to float data.
        for (int i = 0; i < (array_size * spec._num_components); ++i) {
          data[i] = (float)(((double*)ptr_data._ptr)[i]);
        }
        return data;

      case Shader::SPT_float:
        return (float *)ptr_data._ptr;

      default:
#ifndef NDEBUG
        display_cat.error()
          << "Invalid ShaderPtrData type " << (int)ptr_data._type
          << " for shader input '" << spec._id._name << "'\n";
#endif
        return nullptr;
      }

      return data;
    }
    break;

  case Shader::SPT_int:
    if (ptr_data._type != Shader::SPT_int &&
        ptr_data._type != Shader::SPT_uint) {
      display_cat.error()
        << "Cannot pass floating-point data to integer shader input '" << spec._id._name << "'\n";

      // Deactivate it to make sure the user doesn't get flooded with this
      // error.
      spec._dep = 0;

    } else {
      return ptr_data._ptr;
    }
    break;

  case Shader::SPT_uint:
    if (ptr_data._type != Shader::SPT_uint &&
        ptr_data._type != Shader::SPT_int) {
      display_cat.error()
        << "Cannot pass floating-point data to integer shader input '" << spec._id._name << "'\n";

      // Deactivate it to make sure the user doesn't get flooded with this
      // error.
      spec._dep = 0;
      return nullptr;

    } else {
      return ptr_data._ptr;
    }
    break;

  case Shader::SPT_double:
    display_cat.error()
      << "Passing double-precision shader inputs to shaders is not currently supported\n";

    // Deactivate it to make sure the user doesn't get flooded with this
    // error.
    spec._dep = 0;
    break;

  case Shader::SPT_bool:
    if (ptr_data._type == Shader::SPT_double) {
      unsigned int *data = (unsigned int *)scratch;
      for (int i = 0; i < (array_size * spec._num_components); ++i) {
        data[i] = ((double *)ptr_data._ptr)[i] != 0;
      }
      return data;
    } else {
      return (float *)ptr_data._ptr;
    }

  case Shader::SPT_unknown:
    break;
  }

  return nullptr;
}

/**
 * See fetch_specified_value
 */
void GraphicsStateGuardian::
fetch_specified_part(Shader::ShaderMatInput part, InternalName *name,
                     LVecBase4f *into, int count) {
  nassertv(count > 0);

  switch (part) {
  case Shader::SMO_identity: {
    for (int i = 0; i < count; ++i) {
      ((LMatrix4f *)into)[i] = LMatrix4f::ident_mat();
    }
    return;
  }
  case Shader::SMO_window_size:
  case Shader::SMO_pixel_size: {
    LVecBase2i pixel_size = _current_display_region->get_pixel_size();
    into[0].set(pixel_size[0], pixel_size[1], 0, 1);
    return;
  }
  case Shader::SMO_frame_number: {
    int count = ClockObject::get_global_clock()->get_frame_count();
    ((LVecBase4i *)into)[0].fill(count);
    return;
  }
  case Shader::SMO_frame_time: {
    PN_stdfloat time = ClockObject::get_global_clock()->get_frame_time();
    into[0].fill(time);
    return;
  }
  case Shader::SMO_frame_delta: {
    PN_stdfloat dt = ClockObject::get_global_clock()->get_dt();
    into[0].fill(dt);
    return;
  }
  case Shader::SMO_texpad_x: {
    Texture *tex = _target_shader->get_shader_input_texture(name);
    nassertv(tex != nullptr);
    int sx = tex->get_x_size() - tex->get_pad_x_size();
    int sy = tex->get_y_size() - tex->get_pad_y_size();
    int sz = tex->get_z_size() - tex->get_pad_z_size();
    double cx = (sx * 0.5) / tex->get_x_size();
    double cy = (sy * 0.5) / tex->get_y_size();
    double cz = (sz * 0.5) / tex->get_z_size();
    into[0].set(cx, cy, cz, 0);
    return;
  }
  case Shader::SMO_texpix_x: {
    Texture *tex = _target_shader->get_shader_input_texture(name);
    nassertv(tex != nullptr);
    double px = 1.0 / tex->get_x_size();
    double py = 1.0 / tex->get_y_size();
    double pz = 1.0 / tex->get_z_size();
    into[0].set(px, py, pz, 0);
    return;
  }
  case Shader::SMO_attr_material: {
    const MaterialAttrib *target_material = (const MaterialAttrib *)
      _target_rs->get_attrib_def(MaterialAttrib::get_class_slot());
    // Material matrix contains AMBIENT, DIFFUSE, EMISSION, SPECULAR+SHININESS
    if (target_material->is_off()) {
      into[Shader::MA_ambient].set(1, 1, 1, 1);
      into[Shader::MA_diffuse].set(1, 1, 1, 1);
      into[Shader::MA_emission].set(0, 0, 0, 0);
      into[Shader::MA_specular].set(0, 0, 0, 0);
      into[Shader::MA_base_color].set(0, 0, 0, 0);
      into[Shader::MA_metallic_ior_roughness].set(0, 0, 0, 1);
      return;
    }
    Material *m = target_material->get_material();
    LVecBase4 spc = m->get_specular();
    spc[3] = m->get_shininess();
    into[Shader::MA_ambient] = LCAST(float, m->get_ambient());
    into[Shader::MA_diffuse] = LCAST(float, m->get_diffuse());
    into[Shader::MA_emission] = LCAST(float, m->get_emission());
    into[Shader::MA_specular] = LCAST(float, spc);
    into[Shader::MA_base_color] = LCAST(float, m->get_base_color());
    into[Shader::MA_metallic_ior_roughness].set(m->get_metallic(), m->get_refractive_index(), 0, m->get_roughness());
    return;
  }
  case Shader::SMO_attr_color: {
    const ColorAttrib *target_color = (const ColorAttrib *)
      _target_rs->get_attrib_def(ColorAttrib::get_class_slot());
    if (target_color->get_color_type() != ColorAttrib::T_flat) {
      into[0].set(1, 1, 1, 1);
      return;
    }
    into[0] = LCAST(float, target_color->get_color());
    return;
  }
  case Shader::SMO_attr_colorscale: {
    const ColorScaleAttrib *target_color = (const ColorScaleAttrib *)
      _target_rs->get_attrib_def(ColorScaleAttrib::get_class_slot());
    if (target_color->is_identity()) {
      into[0].set(1, 1, 1, 1);
      return;
    }
    into[0] = LCAST(float, target_color->get_scale());
    return;
  }
  case Shader::SMO_attr_fog: {
    const FogAttrib *target_fog = (const FogAttrib *)
      _target_rs->get_attrib_def(FogAttrib::get_class_slot());
    Fog *fog = target_fog->get_fog();
    if (fog == nullptr) {
      into[0].set(0, 1, 1, 1);
      into[1].set(1, 1, 1, 1);
      return;
    }
    PN_stdfloat start, end;
    fog->get_linear_range(start, end);
    into[0].set(fog->get_exp_density(), start, end, 1.0f / (end - start));
    into[1] = LCAST(float, fog->get_color());
    return;
  }
  case Shader::SMO_alight_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    AmbientLight *lt;
    DCAST_INTO_V(lt, np.node());
    into[0] = LCAST(float, lt->get_color());
    return;
  }
  case Shader::SMO_satten_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    Spotlight *lt;
    DCAST_INTO_V(lt, np.node());
    LVecBase3 const &a = lt->get_attenuation();
    PN_stdfloat x = lt->get_exponent();
    into[0].set(a[0], a[1], a[2], x);
    return;
  }
  case Shader::SMO_dlight_x: {
    // The dlight matrix contains COLOR, SPECULAR, DIRECTION, PSEUDOHALFANGLE
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    DirectionalLight *lt;
    DCAST_INTO_V(lt, np.node());
    LColor const &c = lt->get_color();
    LColor const &s = lt->get_specular_color();
    LMatrix4 t = np.get_net_transform()->get_mat() *
                 _scene_setup->get_world_transform()->get_mat();
    LVecBase3 d = -(t.xform_vec(lt->get_direction()));
    d.normalize();
    LVecBase3 h = d + LVecBase3(0,-1,0);
    h.normalize();
    into[0].set(c[0], c[1], c[2], c[3]);
    into[1].set(s[0], s[1], s[2], c[3]);
    into[2].set(d[0], d[1], d[2], 0);
    into[3].set(h[0], h[1], h[2], 0);
    return;
  }
  case Shader::SMO_plight_x: {
    // The plight matrix contains COLOR, SPECULAR, POINT, ATTENUATION
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    PointLight *lt;
    DCAST_INTO_V(lt, np.node());
    LColor const &c = lt->get_color();
    LColor const &s = lt->get_specular_color();
    LMatrix4 t = np.get_net_transform()->get_mat() *
                 _scene_setup->get_world_transform()->get_mat();
    LVecBase3 p = (t.xform_point(lt->get_point()));
    LVecBase3 a = lt->get_attenuation();
    Lens *lens = lt->get_lens(0);
    PN_stdfloat lnear = lens->get_near();
    PN_stdfloat lfar = lens->get_far();
    into[0].set(c[0], c[1], c[2], c[3]);
    into[1].set(s[0], s[1], s[2], s[3]);
    into[2].set(p[0], p[1], p[2], lnear);
    into[3].set(a[0], a[1], a[2], lfar);
    return;
  }
  case Shader::SMO_slight_x: {
    // The slight matrix contains COLOR, SPECULAR, POINT, DIRECTION
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    Spotlight *lt;
    DCAST_INTO_V(lt, np.node());
    Lens *lens = lt->get_lens();
    nassertv(lens != nullptr);
    LColor const &c = lt->get_color();
    LColor const &s = lt->get_specular_color();
    PN_stdfloat cutoff = ccos(deg_2_rad(lens->get_hfov() * 0.5f));
    LMatrix4 t = np.get_net_transform()->get_mat() *
                 _scene_setup->get_world_transform()->get_mat();
    LVecBase3 p = t.xform_point(lens->get_nodal_point());
    LVecBase3 d = -(t.xform_vec(lens->get_view_vector()));
    into[0].set(c[0], c[1], c[2], c[3]);
    into[1].set(s[0], s[1], s[2], s[3]);
    into[2].set(p[0], p[1], p[2], 0);
    into[3].set(d[0], d[1], d[2], cutoff);
    return;
  }
  case Shader::SMO_light_ambient: {
    LColor cur_ambient_light(0.0f, 0.0f, 0.0f, 0.0f);
    const LightAttrib *target_light = (const LightAttrib *)
      _target_rs->get_attrib_def(LightAttrib::get_class_slot());

    if (!target_light->has_any_on_light()) {
      // There are no lights at all.  This means, to follow the fixed-
      // function model, we pretend there is an all-white ambient light.
      into[0].set(1, 1, 1, 1);
    } else {
      into[0] = LCAST(float, target_light->get_ambient_contribution());
    }
    return;
  }
  case Shader::SMO_texmat_i: {
    const TexMatrixAttrib *tma;
    const TextureAttrib *ta;

    int num_stages = 0;
    if (_target_rs->get_attrib(ta) && _target_rs->get_attrib(tma)) {
      num_stages = std::min(count, (int)ta->get_num_on_stages());
    }

    int i = 0;
    for (; i < num_stages; ++i) {
      ((LMatrix4f *)into)[i] = LCAST(float, tma->get_mat(ta->get_on_stage(i)));
    }
    for (; i < count; ++i) {
      ((LMatrix4f *)into)[i] = LMatrix4f::ident_mat();
    }
    return;
  }
  case Shader::SMO_inv_texmat_i: {
    const TexMatrixAttrib *tma;
    const TextureAttrib *ta;

    int num_stages = 0;
    if (_target_rs->get_attrib(ta) && _target_rs->get_attrib(tma)) {
      num_stages = std::min(count, (int)ta->get_num_on_stages());
    }

    int i = 0;
    for (; i < num_stages; ++i) {
      ((LMatrix4f *)into)[i] = LCAST(float, tma->get_transform(ta->get_on_stage(i))->get_inverse()->get_mat());
    }
    for (; i < count; ++i) {
      ((LMatrix4f *)into)[i] = LMatrix4f::ident_mat();
    }
    return;
  }
  case Shader::SMO_texscale_i: {
    const TexMatrixAttrib *tma;
    const TextureAttrib *ta;

    int num_stages = 0;
    if (_target_rs->get_attrib(ta) && _target_rs->get_attrib(tma)) {
      num_stages = std::min(count, (int)ta->get_num_on_stages());
    }

    int i = 0;
    for (; i < num_stages; ++i) {
      LVecBase3 scale = tma->get_transform(ta->get_on_stage(i))->get_scale();
      into[i].set(scale[0], scale[1], scale[2], 0);
    }
    for (; i < count; ++i) {
      into[i].set(0, 0, 0, 1);
    }
    return;
  }
  case Shader::SMO_texcolor_i: {
    const TextureAttrib *ta;

    int num_stages = 0;
    if (_target_rs->get_attrib(ta)) {
      num_stages = std::min(count, (int)ta->get_num_on_stages());
    }

    int i = 0;
    for (; i < num_stages; ++i) {
      TextureStage *ts = ta->get_on_stage(i);
      into[i] = LCAST(float, ts->get_color());
    }
    for (; i < count; ++i) {
      into[i].set(0, 0, 0, 1);
    }
    return;
  }
  case Shader::SMO_texconst_i: {
    const TexGenAttrib *tga;
    const TextureAttrib *ta;

    int num_stages = 0;
    if (_target_rs->get_attrib(ta) && _target_rs->get_attrib(tga)) {
      num_stages = std::min(count, (int)ta->get_num_on_stages());
    }

    int i = 0;
    for (; i < num_stages; ++i) {
      LVecBase3 value = tga->get_constant_value(ta->get_on_stage(i));
      into[i].set(value[0], value[1], value[2], 1);
    }
    for (; i < count; ++i) {
      into[i].set(0, 0, 0, 1);
    }
    return;
  }
  case Shader::SMO_tex_is_alpha_i: {
    // This is a hack so we can support both F_alpha and other formats in the
    // default shader, to fix font rendering in GLES2
    const TextureAttrib *ta;

    int num_stages = 0;
    if (_target_rs->get_attrib(ta)) {
      num_stages = std::min(count, (int)ta->get_num_on_stages());
    }

    int i = 0;
    for (; i < num_stages; ++i) {
      TextureStage *ts = ta->get_on_stage(i);
      PN_stdfloat v = (ta->get_on_texture(ts)->get_format() == Texture::F_alpha);
      into[i].set(v, v, v, 0);
    }
    for (; i < count; ++i) {
      into[i].set(0, 0, 0, 0);
    }
    return;
  }
  case Shader::SMO_plane_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    const PlaneNode *plane_node;
    DCAST_INTO_V(plane_node, np.node());
    into[0] = LCAST(float, plane_node->get_plane());
    return;
  }
  case Shader::SMO_clipplane_x: {
    const ClipPlaneAttrib *cpa;
    _target_rs->get_attrib_def(cpa);
    int planenr = atoi(name->get_name().c_str());
    if (planenr >= cpa->get_num_on_planes()) {
      into[0].set(0, 0, 0, 0);
      return;
    }
    const NodePath &np = cpa->get_on_plane(planenr);
    nassertv(!np.is_empty());
    const PlaneNode *plane_node;
    DCAST_INTO_V(plane_node, np.node());

    // Transform plane to world space
    CPT(TransformState) transform = np.get_net_transform();
    LPlane plane = plane_node->get_plane();
    if (!transform->is_identity()) {
      plane.xform(transform->get_mat());
    }
    into[0] = LCAST(float, plane);
    return;
  }
  case Shader::SMO_apiview_clipplane_i: {
    const ClipPlaneAttrib *cpa;
    _target_rs->get_attrib_def(cpa);

    int num_planes = std::min(count, (int)cpa->get_num_on_planes());
    int i = 0;
    for (; i < num_planes; ++i) {
      const NodePath &plane = cpa->get_on_plane(i);
      nassertv(!plane.is_empty());
      const PlaneNode *plane_node;
      DCAST_INTO_V(plane_node, plane.node());

      CPT(TransformState) transform =
        _scene_setup->get_cs_world_transform()->compose(
          plane.get_transform(_scene_setup->get_scene_root().get_parent()));

      LPlane xformed_plane = plane_node->get_plane() * transform->get_mat();
      into[i] = LCAST(float, xformed_plane);
    }

    for (; i < count; ++i) {
      // Fill the remainder with zeroes.
      into[i].set(0, 0, 0, 0);
    }
    return;
  }
  case Shader::SMO_mat_constant_x: {
#ifdef STDFLOAT_DOUBLE
    LMatrix4 tmp;
    _target_shader->get_shader_input_matrix(name, tmp);
    *(LMatrix4f *)into = LCAST(float, tmp);
#else
    _target_shader->get_shader_input_matrix(name, *(LMatrix4f *)into);
#endif
    return;
  }
  case Shader::SMO_vec_constant_x: {
    into[0] = LCAST(float, _target_shader->get_shader_input_vector(name));
    return;
  }
  case Shader::SMO_world_to_view: {
    *(LMatrix4f *)into = LCAST(float, _scene_setup->get_world_transform()->get_mat());
    return;
  }
  case Shader::SMO_view_to_world: {
    *(LMatrix4f *)into = LCAST(float, _scene_setup->get_camera_transform()->get_mat());
    return;
  }
  case Shader::SMO_model_to_view: {
    *(LMatrix4f *)into = LCAST(float, _inv_cs_transform->compose(_internal_transform)->get_mat());
    return;
  }
  case Shader::SMO_model_to_apiview: {
    *(LMatrix4f *)into = LCAST(float, _internal_transform->get_mat());
    return;
  }
  case Shader::SMO_view_to_model: {
    *(LMatrix4f *)into = LCAST(float, _internal_transform->invert_compose(_cs_transform)->get_mat());
    return;
  }
  case Shader::SMO_apiview_to_model: {
    *(LMatrix4f *)into = LCAST(float, _internal_transform->get_inverse()->get_mat());
    return;
  }
  case Shader::SMO_apiview_to_view: {
    *(LMatrix4f *)into = LCAST(float, _inv_cs_transform->get_mat());
    return;
  }
  case Shader::SMO_view_to_apiview: {
    *(LMatrix4f *)into = LCAST(float, _cs_transform->get_mat());
    return;
  }
  case Shader::SMO_clip_to_view: {
    if (_current_lens->get_coordinate_system() == _coordinate_system) {
      *(LMatrix4f *)into = LCAST(float, _current_lens->get_projection_mat_inv(_current_stereo_channel));
    } else {
      *(LMatrix4f *)into = LCAST(float,
        _current_lens->get_projection_mat_inv(_current_stereo_channel) *
        LMatrix4::convert_mat(_current_lens->get_coordinate_system(), _coordinate_system));
    }
    return;
  }
  case Shader::SMO_view_to_clip: {
    if (_current_lens->get_coordinate_system() == _coordinate_system) {
      *(LMatrix4f *)into = LCAST(float, _current_lens->get_projection_mat(_current_stereo_channel));
    } else {
      *(LMatrix4f *)into = LCAST(float,
        LMatrix4::convert_mat(_coordinate_system, _current_lens->get_coordinate_system()) *
        _current_lens->get_projection_mat(_current_stereo_channel));
    }
    return;
  }
  case Shader::SMO_apiclip_to_view: {
    *(LMatrix4f *)into = LCAST(float, _projection_mat_inv->get_mat() * _inv_cs_transform->get_mat());
    return;
  }
  case Shader::SMO_view_to_apiclip: {
    *(LMatrix4f *)into = LCAST(float, _cs_transform->get_mat() * _projection_mat->get_mat());
    return;
  }
  case Shader::SMO_apiclip_to_apiview: {
    *(LMatrix4f *)into = LCAST(float, _projection_mat_inv->get_mat());
    return;
  }
  case Shader::SMO_apiview_to_apiclip: {
    *(LMatrix4f *)into = LCAST(float, _projection_mat->get_mat());
    return;
  }
  case Shader::SMO_view_x_to_view: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    *(LMatrix4f *)into = LCAST(float, np.get_net_transform()->get_mat() *
      _scene_setup->get_world_transform()->get_mat());
    return;
  }
  case Shader::SMO_view_to_view_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    *(LMatrix4f *)into = LCAST(float, _scene_setup->get_camera_transform()->get_mat() *
      np.get_net_transform()->get_inverse()->get_mat());
    return;
  }
  case Shader::SMO_apiview_x_to_view: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    *(LMatrix4f *)into = LCAST(float, LMatrix4::convert_mat(_internal_coordinate_system, _coordinate_system) *
      np.get_net_transform()->get_mat() *
      _scene_setup->get_world_transform()->get_mat());
    return;
  }
  case Shader::SMO_view_to_apiview_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    *(LMatrix4f *)into = LCAST(float, (_scene_setup->get_camera_transform()->get_mat() *
         np.get_net_transform()->get_inverse()->get_mat() *
         LMatrix4::convert_mat(_coordinate_system, _internal_coordinate_system)));
    return;
  }
  case Shader::SMO_clip_x_to_view: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    const LensNode *node;
    DCAST_INTO_V(node, np.node());
    const Lens *lens = node->get_lens();
    *(LMatrix4f *)into = LCAST(float, lens->get_projection_mat_inv(_current_stereo_channel) *
      LMatrix4::convert_mat(lens->get_coordinate_system(), _coordinate_system) *
      np.get_net_transform()->get_mat() *
      _scene_setup->get_world_transform()->get_mat());
    return;
  }
  case Shader::SMO_view_to_clip_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    const LensNode *node;
    DCAST_INTO_V(node, np.node());
    const Lens *lens = node->get_lens();
    *(LMatrix4f *)into = LCAST(float, _scene_setup->get_camera_transform()->get_mat() *
      np.get_net_transform()->get_inverse()->get_mat() *
      LMatrix4::convert_mat(_coordinate_system, lens->get_coordinate_system()) *
      lens->get_projection_mat(_current_stereo_channel));
    return;
  }
  case Shader::SMO_apiclip_x_to_view: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    const LensNode *node;
    DCAST_INTO_V(node, np.node());
    const Lens *lens = node->get_lens();
    *(LMatrix4f *)into = LCAST(float, calc_projection_mat(lens)->get_inverse()->get_mat() *
      get_cs_transform_for(lens->get_coordinate_system())->get_inverse()->get_mat() *
      np.get_net_transform()->get_mat() *
      _scene_setup->get_world_transform()->get_mat());
    return;
  }
  case Shader::SMO_view_to_apiclip_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertv(!np.is_empty());
    const LensNode *node;
    DCAST_INTO_V(node, np.node());
    const Lens *lens = node->get_lens();
    *(LMatrix4f *)into = LCAST(float, _scene_setup->get_camera_transform()->get_mat() *
      np.get_net_transform()->get_inverse()->get_mat() *
      get_cs_transform_for(lens->get_coordinate_system())->get_mat() *
      calc_projection_mat(lens)->get_mat());
    return;
  }
  case Shader::SMO_mat_constant_x_attrib: {
    if (_target_shader->has_shader_input(name)) {
      // There is an input specifying precisely this whole thing, with dot and
      // all.  Support this, even if only for backward compatibility.
#ifdef STDFLOAT_DOUBLE
      LMatrix4 tmp;
      _target_shader->get_shader_input_matrix(name, tmp);
      *(LMatrix4f *)into = LCAST(float, tmp);
#else
      _target_shader->get_shader_input_matrix(name, *(LMatrix4f *)into);
#endif
      return;
    }

    const NodePath &np = _target_shader->get_shader_input_nodepath(name->get_parent());
    const PandaNode *node = np.node();

    // This is the only matrix member we support from NodePath inputs.
    if (node != nullptr && node->is_of_type(LensNode::get_class_type()) &&
        name->get_basename() == "shadowViewMatrix") {
      const LensNode *lnode = (const LensNode *)node;
      const Lens *lens = lnode->get_lens();

      LMatrix4 t = _inv_cs_transform->get_mat() *
        _scene_setup->get_camera_transform()->get_mat() *
        np.get_net_transform()->get_inverse()->get_mat() *
        LMatrix4::convert_mat(_coordinate_system, lens->get_coordinate_system());

      if (!lnode->is_of_type(PointLight::get_class_type())) {
        t *= lens->get_projection_mat() * shadow_bias_mat;
      }
      *(LMatrix4f *)into = LCAST(float, t);
    }
    else {
      display_cat.error()
        << "Shader input " << *name << " requests invalid attribute "
        << name->get_basename() << " from node " << np << "\n";
      *(LMatrix4f *)into = LMatrix4f::ident_mat();
    }
    return;
  }
  case Shader::SMO_vec_constant_x_attrib: {
    if (_target_shader->has_shader_input(name)) {
      // There is an input specifying precisely this whole thing, with dot and
      // all.  Support this, even if only for backward compatibility.
      into[0] = LCAST(float, _target_shader->get_shader_input_vector(name));
      return;
    }

    const NodePath &np = _target_shader->get_shader_input_nodepath(name->get_parent());
    nassertv(!np.is_empty());

    fetch_specified_member(np, name->get_basename(), into[0]);
    return;
  }
  case Shader::SMO_light_source_i: {
    const LightAttrib *target_light;
    _target_rs->get_attrib_def(target_light);

    // We don't count ambient lights, which would be pretty silly to handle
    // via this mechanism.
    size_t num_lights = std::min((size_t)count, target_light->get_num_non_ambient_lights());

    size_t i = 0;
    for (i = 0; i < num_lights; ++i) {
      NodePath light = target_light->get_on_light(i);
      nassertv(!light.is_empty());
      fetch_specified_light(light, into);
      into += Shader::LA_COUNT;
    }
    // Apply the default OpenGL lights otherwise.
    // Special exception for light 0, which defaults to white.
    for (; i < (size_t)count; ++i) {
      fetch_specified_light(NodePath(), into);
      if (i == 0) {
        into[Shader::LA_color].set(1, 1, 1, 1);
      }
      into += Shader::LA_COUNT;
    }
    return;
  }
  case Shader::SMO_apiview_to_apiclip_light_source_i: { // shadowViewMatrix
    const LightAttrib *target_light;
    _target_rs->get_attrib_def(target_light);

    // We don't count ambient lights, which would be pretty silly to handle
    // via this mechanism.
    size_t num_lights = std::min((size_t)count, target_light->get_num_non_ambient_lights());

    size_t i = 0;
    for (i = 0; i < num_lights; ++i) {
      NodePath light = target_light->get_on_light(i);
      nassertv(!light.is_empty());

      LensNode *lnode;
      DCAST_INTO_V(lnode, light.node());
      Lens *lens = lnode->get_lens();

      LMatrix4 t = _inv_cs_transform->get_mat() *
        _scene_setup->get_camera_transform()->get_mat() *
        light.get_net_transform()->get_inverse()->get_mat() *
        LMatrix4::convert_mat(_coordinate_system, lens->get_coordinate_system());

      if (!lnode->is_of_type(PointLight::get_class_type())) {
        t *= lens->get_projection_mat() * shadow_bias_mat;
      }
      ((LMatrix4f *)into)[i] = LCAST(float, t);
    }

    // Apply just the bias matrix otherwise.
    for (; i < (size_t)count; ++i) {
      ((LMatrix4f *)into)[i] = LCAST(float, shadow_bias_mat);
    }
    return;
  }
  case Shader::SMO_light_source_i_packed: {
    // The light matrix contains COLOR, ATTENUATION, VIEWVECTOR, POSITION
    const LightAttrib *target_light;
    _target_rs->get_attrib_def(target_light);

    // We don't count ambient lights, which would be pretty silly to handle
    // via this mechanism.
    size_t num_lights = std::min((size_t)count, target_light->get_num_non_ambient_lights());

    size_t i = 0;
    for (i = 0; i < num_lights; ++i) {
      NodePath np = target_light->get_on_light(i);
      nassertv(!np.is_empty());
      PandaNode *node = np.node();
      Light *light = node->as_light();
      nassertv(light != nullptr);
      into[0] = LCAST(float, light->get_color());
      into[1] = LVecBase4f(LCAST(float, light->get_attenuation()), 0);

      LMatrix4 mat = np.get_net_transform()->get_mat() *
        _scene_setup->get_world_transform()->get_mat();

      if (node->is_of_type(DirectionalLight::get_class_type())) {
        LVecBase3 d = mat.xform_vec(((const DirectionalLight *)node)->get_direction());
        d.normalize();
        into[2] = LVecBase4f(LCAST(float, d), 0);
        into[3] = LVecBase4f(-LCAST(float, d), 0);
      }
      else if (node->is_of_type(LightLensNode::get_class_type())) {
        const Lens *lens = ((const LightLensNode *)node)->get_lens();

        LPoint3 p = mat.xform_point(lens->get_nodal_point());
        into[3] = LVecBase4f(LCAST(float, p));

        // For shadowed point light we need to store near/far.
        // For spotlight we need to store cutoff angle.
        if (node->is_of_type(Spotlight::get_class_type())) {
          PN_stdfloat cutoff = ccos(deg_2_rad(lens->get_hfov() * 0.5f));
          LVecBase3 d = -(mat.xform_vec(lens->get_view_vector()));
          into[1][3] = ((const Spotlight *)node)->get_exponent();
          into[2] = LVecBase4f(LCAST(float, d), cutoff);
        }
        else if (node->is_of_type(PointLight::get_class_type())) {
          into[1][3] = lens->get_far();
          into[3][3] = lens->get_near();

          if (node->is_of_type(SphereLight::get_class_type())) {
            into[2][3] = ((const SphereLight *)node)->get_radius();
          }
        }
      }

      into += 4;
    }
    // Apply the default OpenGL lights otherwise.
    // Special exception for light 0, which defaults to white.
    if (i == 0) {
      into[0].set(1, 1, 1, 1);
      into[1].set(1, 0, 0, 0);
      into[2].set(0, 0, 0, 0);
      into[3].set(0, 0, 0, 0);
      ++i;
    }
    for (; i < (size_t)count; ++i) {
      into[0].set(0, 0, 0, 0);
      into[1].set(1, 0, 0, 0);
      into[2].set(0, 0, 0, 0);
      into[3].set(0, 0, 0, 0);
      into += 4;
    }
    return;
  }
  case Shader::SMO_attr_pointparams: {
    const RenderModeAttrib *target_render_mode;
    _target_rs->get_attrib_def(target_render_mode);

    PN_stdfloat thickness = target_render_mode->get_thickness();
    PN_stdfloat catten = thickness;
    PN_stdfloat patten = 0.0f;
    if (target_render_mode->get_perspective()) {
      LVecBase2i pixel_size = _current_display_region->get_pixel_size();

      LVector3 height(0.0f, thickness, 1.0f);
      height = height * _projection_mat->get_mat();
      height = height * _internal_transform->get_scale()[1];
      PN_stdfloat s = height[1] * pixel_size[1];

      if (_current_lens->is_orthographic()) {
        catten = s;
        patten = 0.0f;
      } else {
        catten = 0.0f;
        patten = s;
      }
    }

    into[0].set(thickness, catten, patten, 0.0f);
    return;
  }
  default:
    nassertv(false /*should never get here*/);
    return;
  }
}

/**
 * Given a NodePath passed into a shader input that is a structure, fetches
 * the value for the given member.
 */
void GraphicsStateGuardian::
fetch_specified_member(const NodePath &np, CPT_InternalName attrib, LVecBase4f &v) {
  // This system is not ideal.  It will be improved in the future.
  static const CPT_InternalName IN_color("color");
  static const CPT_InternalName IN_ambient("ambient");
  static const CPT_InternalName IN_diffuse("diffuse");
  static const CPT_InternalName IN_specular("specular");
  static const CPT_InternalName IN_position("position");
  static const CPT_InternalName IN_halfVector("halfVector");
  static const CPT_InternalName IN_spotDirection("spotDirection");
  static const CPT_InternalName IN_spotCutoff("spotCutoff");
  static const CPT_InternalName IN_spotCosCutoff("spotCosCutoff");
  static const CPT_InternalName IN_spotExponent("spotExponent");
  static const CPT_InternalName IN_attenuation("attenuation");
  static const CPT_InternalName IN_constantAttenuation("constantAttenuation");
  static const CPT_InternalName IN_linearAttenuation("linearAttenuation");
  static const CPT_InternalName IN_quadraticAttenuation("quadraticAttenuation");

  PandaNode *node = nullptr;
  if (!np.is_empty()) {
    node = np.node();
  }

  if (attrib == IN_color) {
    if (node == nullptr) {
      v.set(0, 0, 0, 1);
      return;
    }
    Light *light = node->as_light();
    nassertv(light != nullptr);
    v = LCAST(float, light->get_color());
  }
  else if (attrib == IN_ambient) {
    if (node == nullptr) {
      v.set(0, 0, 0, 1);
      return;
    }
    Light *light = node->as_light();
    nassertv(light != nullptr);
    if (node->is_ambient_light()) {
      v = LCAST(float, light->get_color());
    } else {
      // Non-ambient lights don't currently have an ambient color in Panda3D.
      v.set(0, 0, 0, 1);
    }
  }
  else if (attrib == IN_diffuse) {
    if (node == nullptr) {
      v.set(0, 0, 0, 1);
      return;
    }
    Light *light = node->as_light();
    nassertv(light != nullptr);
    if (node->is_ambient_light()) {
      // Ambient light has no diffuse color.
      v.set(0, 0, 0, 1);
    } else {
      v = LCAST(float, light->get_color());
    }
  }
  else if (attrib == IN_specular) {
    if (node == nullptr) {
      v.set(0, 0, 0, 1);
      return;
    }
    Light *light = node->as_light();
    nassertv(light != nullptr);
    v = LCAST(float, light->get_specular_color());
  }
  else if (attrib == IN_position) {
    if (np.is_empty()) {
      v.set(0, 0, 1, 0);
    }
    else if (node->is_ambient_light()) {
      // Ambient light has no position.
      v.set(0, 0, 0, 0);
    }
    else if (node->is_of_type(DirectionalLight::get_class_type())) {
      DirectionalLight *light;
      DCAST_INTO_V(light, node);

      CPT(TransformState) transform = np.get_transform(_scene_setup->get_scene_root().get_parent());
      LVector3 dir = -(light->get_direction() * transform->get_mat());
      dir *= _scene_setup->get_cs_world_transform()->get_mat();
      v.set(dir[0], dir[1], dir[2], 0);
    }
    else {
      LightLensNode *light;
      DCAST_INTO_V(light, node);
      Lens *lens = light->get_lens();
      nassertv(lens != nullptr);

      CPT(TransformState) transform =
        _scene_setup->get_cs_world_transform()->compose(
          np.get_transform(_scene_setup->get_scene_root().get_parent()));

      const LMatrix4 &light_mat = transform->get_mat();
      LPoint3 pos = lens->get_nodal_point() * light_mat;
      v.set(pos[0], pos[1], pos[2], 1);
    }
  }
  else if (attrib == IN_halfVector) {
    if (np.is_empty()) {
      v.set(0, 0, 1, 0);
    }
    else if (node->is_ambient_light()) {
      // Ambient light has no half-vector.
      v.set(0, 0, 0, 0);
    }
    else if (node->is_of_type(DirectionalLight::get_class_type())) {
      DirectionalLight *light;
      DCAST_INTO_V(light, node);

      CPT(TransformState) transform = np.get_transform(_scene_setup->get_scene_root().get_parent());
      LVector3 dir = -(light->get_direction() * transform->get_mat());
      dir *= _scene_setup->get_cs_world_transform()->get_mat();
      dir.normalize();
      dir += LVector3(0, 0, 1);
      dir.normalize();
      v.set(dir[0], dir[1], dir[2], 1);
    }
    else {
      LightLensNode *light;
      DCAST_INTO_V(light, node);
      Lens *lens = light->get_lens();
      nassertv(lens != nullptr);

      CPT(TransformState) transform =
        _scene_setup->get_cs_world_transform()->compose(
          np.get_transform(_scene_setup->get_scene_root().get_parent()));

      const LMatrix4 &light_mat = transform->get_mat();
      LPoint3 pos = lens->get_nodal_point() * light_mat;
      pos.normalize();
      pos += LVector3(0, 0, 1);
      pos.normalize();
      v.set(pos[0], pos[1], pos[2], 1);
    }
  }
  else if (attrib == IN_spotDirection) {
    if (node == nullptr) {
      v.set(0, 0, -1, 0);
    }
    else if (node->is_ambient_light()) {
      // Ambient light has no spot direction.
      v.set(0, 0, 0, 0);
    }
    else {
      LightLensNode *light;
      DCAST_INTO_V(light, node);
      Lens *lens = light->get_lens();
      nassertv(lens != nullptr);

      CPT(TransformState) transform =
        _scene_setup->get_cs_world_transform()->compose(
          np.get_transform(_scene_setup->get_scene_root().get_parent()));

      const LMatrix4 &light_mat = transform->get_mat();
      LVector3 dir = lens->get_view_vector() * light_mat;
      v.set(dir[0], dir[1], dir[2], 0);
    }
  }
  else if (attrib == IN_spotCutoff) {
    if (node != nullptr &&
        node->is_of_type(Spotlight::get_class_type())) {
      LightLensNode *light;
      DCAST_INTO_V(light, node);
      Lens *lens = light->get_lens();
      nassertv(lens != nullptr);

      float cutoff = lens->get_hfov() * 0.5f;
      v.fill(cutoff);
    }
    else {
      // Other lights have no cut-off.
      v.fill(180);
    }
  }
  else if (attrib == IN_spotCosCutoff) {
    if (node != nullptr &&
        node->is_of_type(Spotlight::get_class_type())) {
      LightLensNode *light;
      DCAST_INTO_V(light, node);
      Lens *lens = light->get_lens();
      nassertv(lens != nullptr);

      float cutoff = lens->get_hfov() * 0.5f;
      v.fill(ccos(deg_2_rad(cutoff)));
    } else {
      // Other lights have no cut-off.
      v.fill(-1);
    }
  }
  else if (attrib == IN_spotExponent) {
    if (node == nullptr) {
      v.fill(0);
      return;
    }
    Light *light = node->as_light();
    nassertv(light != nullptr);

    v.fill(light->get_exponent());
  }
  else if (attrib == IN_attenuation) {
    if (node != nullptr) {
      Light *light = node->as_light();
      nassertv(light != nullptr);

      v = LVecBase4f(LCAST(float, light->get_attenuation()), 0);
    } else {
      v.set(1, 0, 0, 0);
    }
  }
  else if (attrib == IN_constantAttenuation) {
    if (node == nullptr) {
      v.fill(1);
      return;
    }
    Light *light = node->as_light();
    nassertv(light != nullptr);

    v.fill(light->get_attenuation()[0]);
  }
  else if (attrib == IN_linearAttenuation) {
    if (node == nullptr) {
      v.fill(0);
      return;
    }
    Light *light = node->as_light();
    nassertv(light != nullptr);

    v.fill(light->get_attenuation()[1]);
  }
  else if (attrib == IN_quadraticAttenuation) {
    if (node == nullptr) {
      v.fill(0);
      return;
    }
    Light *light = node->as_light();
    nassertv(light != nullptr);

    v.fill(light->get_attenuation()[2]);
  }
  else {
    display_cat.error()
      << "Shader input requests invalid attribute " << *attrib
      << " from node " << np << "\n";
    v.set(0, 0, 0, 1);
  }
}

/**
 * Given a NodePath passed into a shader input that is a structure, fetches
 * the value for the given member.
 */
void GraphicsStateGuardian::
fetch_specified_light(const NodePath &np, LVecBase4f *into) {
  PandaNode *node = nullptr;
  if (!np.is_empty()) {
    node = np.node();
  }

  if (node == nullptr) {
    into[Shader::LA_color].set(0, 0, 0, 1);
    into[Shader::LA_specular].set(0, 0, 0, 1);
    into[Shader::LA_ambient].set(0, 0, 0, 1);
    into[Shader::LA_diffuse].set(0, 0, 0, 1);
    into[Shader::LA_position].set(0, 0, 1, 0);
    into[Shader::LA_half_vector].set(0, 0, 1, 0);
    into[Shader::LA_spot_direction].set(0, 0, -1, 0);
    into[Shader::LA_spot_params].set(-1, 180, 0, 0);
    into[Shader::LA_attenuation].set(1, 0, 0, 0);
    *(LMatrix4f *)&into[Shader::LA_shadow_view_matrix] = LCAST(float, shadow_bias_mat);
  } else {
    Light *light = node->as_light();
    nassertv(light != nullptr);

    LVecBase4f color = LCAST(float, light->get_color());
    into[Shader::LA_color] = color;
    into[Shader::LA_specular] = LCAST(float, light->get_specular_color());

    if (node->is_ambient_light()) {
      into[Shader::LA_ambient] = color;
      into[Shader::LA_diffuse].set(0, 0, 0, 1);
      into[Shader::LA_position].set(0, 0, 0, 0);
      into[Shader::LA_half_vector].set(0, 0, 0, 0);
      into[Shader::LA_spot_direction].set(0, 0, 0, 0);
      into[Shader::LA_spot_params].set(-1, 180, 0, 0);
    } else {
      into[Shader::LA_ambient].set(0, 0, 0, 1);
      into[Shader::LA_diffuse] = color;

      CPT(TransformState) net_transform =
        np.get_transform(_scene_setup->get_scene_root().get_parent());
      CPT(TransformState) transform =
        _scene_setup->get_cs_world_transform()->compose(net_transform);
      const LMatrix4 &light_mat = transform->get_mat();

      LightLensNode *light;
      DCAST_INTO_V(light, node);
      Lens *lens = light->get_lens();
      nassertv(lens != nullptr);

      if (node->is_of_type(DirectionalLight::get_class_type())) {
        DirectionalLight *light;
        DCAST_INTO_V(light, node);

        LVector3 dir = -(light->get_direction() * light_mat);
        into[Shader::LA_position].set(dir[0], dir[1], dir[2], 0);

        dir.normalize();
        dir += LVector3(0, 0, 1);
        dir.normalize();
        into[Shader::LA_half_vector].set(dir[0], dir[1], dir[2], 1);
      }
      else {
        LPoint3 pos = lens->get_nodal_point() * light_mat;
        into[Shader::LA_position].set(pos[0], pos[1], pos[2], 1);

        pos.normalize();
        pos += LVector3(0, 0, 1);
        pos.normalize();
        into[Shader::LA_half_vector].set(pos[0], pos[1], pos[2], 1);
      }

      if (node->is_of_type(Spotlight::get_class_type())) {
        float cutoff = lens->get_hfov() * 0.5f;
        into[Shader::LA_spot_params].set(ccos(deg_2_rad(cutoff)), cutoff, light->get_exponent(), 0);
      } else {
        // spotCosCutoff, spotCutoff, spotExponent
        into[Shader::LA_spot_params].set(-1, 180, light->get_exponent(), 0);
      }

      LVector3 dir = lens->get_view_vector() * light_mat;
      into[Shader::LA_spot_direction].set(dir[0], dir[1], dir[2], 0);

      LMatrix4 t = _inv_cs_transform->get_mat() *
        _scene_setup->get_camera_transform()->get_mat() *
        net_transform->get_inverse()->get_mat() *
        LMatrix4::convert_mat(_coordinate_system, lens->get_coordinate_system());

      if (!node->is_of_type(PointLight::get_class_type())) {
        t *= lens->get_projection_mat() * shadow_bias_mat;
      }
      *(LMatrix4f *)&into[Shader::LA_shadow_view_matrix] = LCAST(float, t);
    }

    LVecBase3 atten = light->get_attenuation();
    PN_stdfloat radius = 0;
    if (node->is_of_type(SphereLight::get_class_type())) {
      radius = ((const SphereLight *)node)->get_radius();
    }
    into[Shader::LA_attenuation].set(atten[0], atten[1], atten[2], radius);
  }
}

/**
 * Like fetch_specified_value, but for texture inputs.
 */
PT(Texture) GraphicsStateGuardian::
fetch_specified_texture(Shader::ShaderTexSpec &spec, SamplerState &sampler,
                        int &view) {

  static PT(Texture) default_add_tex;
  static PT(Texture) default_normal_height_tex;

  switch (spec._part) {
  case Shader::STO_named_input:
    // Named texture input.
    if (!_target_shader->has_shader_input(spec._name)) {
      // Is this a member of something, like a node?
      const InternalName *parent = spec._name->get_parent();
      if (parent != InternalName::get_root() &&
          _target_shader->has_shader_input(parent)) {

        // Yes, grab the node.
        const string &basename = spec._name->get_basename();
        NodePath np = _target_shader->get_shader_input_nodepath(parent);

        if (basename == "shadowMap") {
          PT(Texture) tex = get_shadow_map(np);
          if (tex != nullptr) {
            sampler = tex->get_default_sampler();
          }
          return tex;

        } else {
          if (spec._stage == 0) {
            display_cat.error()
              << "Shader input " << *parent
              << " has no member named " << basename << ".\n";
            spec._stage = -1;
          }
        }
      } else {
        // This used to be legal for some reason, so don't trigger the assert.
        // Prevent flood, though, so abuse the _stage flag to indicate whether
        // we've already errored about this.
        if (spec._stage == 0) {
          display_cat.error()
            << "Shader input " << *spec._name << " is not present.\n";
          spec._stage = -1;
        }
      }
    } else {
      // Just a regular texture input.
      return _target_shader->get_shader_input_texture(spec._name, &sampler);
    }
    break;

  case Shader::STO_stage_i:
    {
      // We get the TextureAttrib directly from the _target_rs, not the
      // filtered TextureAttrib in _target_texture.
      const TextureAttrib *texattrib;
      _target_rs->get_attrib_def(texattrib);

      if (spec._stage < texattrib->get_num_on_stages()) {
        TextureStage *stage = texattrib->get_on_stage(spec._stage);
        sampler = texattrib->get_on_sampler(stage);
        view += stage->get_tex_view_offset();
        return texattrib->get_on_texture(stage);
      }
    }
    break;

  case Shader::STO_light_i_shadow_map:
    {
      const LightAttrib *target_light;
      _target_rs->get_attrib_def(target_light);

      PT(Texture) tex;

      // We don't count ambient lights, which would be pretty silly to handle
      // via this mechanism.
      size_t num_lights = target_light->get_num_non_ambient_lights();
      if (spec._stage >= 0 && (size_t)spec._stage < num_lights) {
        NodePath light = target_light->get_on_light((size_t)spec._stage);
        nassertr(!light.is_empty(), nullptr);
        Light *light_obj = light.node()->as_light();
        nassertr(light_obj != nullptr, nullptr);

        LightLensNode *lln = DCAST(LightLensNode, light.node());
        if (lln != nullptr && lln->_shadow_caster) {
          tex = get_shadow_map(light);
        } else {
          tex = get_dummy_shadow_map((Texture::TextureType)spec._desired_type);
        }
      } else {
        // There is no such light assigned.  Bind a dummy shadow map.
        tex = get_dummy_shadow_map((Texture::TextureType)spec._desired_type);
      }
      if (tex != nullptr) {
        sampler = tex->get_default_sampler();
      }
      return tex;
    }

  case Shader::STO_ff_stage_i:
    {
      // We get the TextureAttrib directly from the _target_rs, not the
      // filtered TextureAttrib in _target_texture.
      const TextureAttrib *texattrib;
      _target_rs->get_attrib_def(texattrib);

      if (spec._stage < texattrib->get_num_on_ff_stages()) {
        TextureStage *stage = texattrib->get_on_ff_stage(spec._stage);
        sampler = texattrib->get_on_sampler(stage);
        view += stage->get_tex_view_offset();
        return texattrib->get_on_texture(stage);
      }
    }
    break;

  case Shader::STO_stage_modulate_i:
    {
      const TextureAttrib *texattrib;
      if (_target_rs->get_attrib(texattrib)) {
        int si = 0;
        for (int i = 0; i < texattrib->get_num_on_stages(); ++i) {
          TextureStage *stage = texattrib->get_on_stage(i);
          TextureStage::Mode mode = stage->get_mode();

          if (mode == TextureStage::M_modulate ||
              mode == TextureStage::M_modulate_glow ||
              mode == TextureStage::M_modulate_gloss) {
            if (si++ == spec._stage) {
              sampler = texattrib->get_on_sampler(stage);
              view += stage->get_tex_view_offset();
              return texattrib->get_on_texture(stage);
            }
          }
        }
      }
    }
    break;

  case Shader::STO_stage_add_i:
    {
      const TextureAttrib *texattrib;
      if (_target_rs->get_attrib(texattrib)) {
        int si = 0;
        for (int i = 0; i < texattrib->get_num_on_stages(); ++i) {
          TextureStage *stage = texattrib->get_on_stage(i);
          TextureStage::Mode mode = stage->get_mode();

          if (mode == TextureStage::M_add) {
            if (si++ == spec._stage) {
              sampler = texattrib->get_on_sampler(stage);
              view += stage->get_tex_view_offset();
              return texattrib->get_on_texture(stage);
            }
          }
        }
      }

      if (default_add_tex == nullptr) {
        PT(Texture) tex = new Texture("default-add");
        tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_luminance);
        tex->set_clear_color(LColor(0, 0, 0, 1));
        default_add_tex = std::move(tex);
      }
      return default_add_tex;
    }

  case Shader::STO_stage_normal_i:
    {
      const TextureAttrib *texattrib;
      if (_target_rs->get_attrib(texattrib)) {
        int si = 0;
        for (int i = 0; i < texattrib->get_num_on_stages(); ++i) {
          TextureStage *stage = texattrib->get_on_stage(i);
          TextureStage::Mode mode = stage->get_mode();

          if (mode == TextureStage::M_normal ||
              mode == TextureStage::M_normal_height) {
            if (si++ == spec._stage) {
              sampler = texattrib->get_on_sampler(stage);
              view += stage->get_tex_view_offset();
              return texattrib->get_on_texture(stage);
            }
          }
        }
      }

      if (default_normal_height_tex == nullptr) {
        PT(Texture) tex = new Texture("default-normal-height");
        tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba);
        tex->set_clear_color(LColor(0.5, 0.5, 1, 0));
        default_normal_height_tex = std::move(tex);
      }
      return default_normal_height_tex;
    }

  case Shader::STO_stage_gloss_i:
    {
      const TextureAttrib *texattrib;
      if (_target_rs->get_attrib(texattrib)) {
        int si = 0;
        for (int i = 0; i < texattrib->get_num_on_stages(); ++i) {
          TextureStage *stage = texattrib->get_on_stage(i);
          TextureStage::Mode mode = stage->get_mode();

          if (mode == TextureStage::M_gloss ||
              mode == TextureStage::M_modulate_gloss ||
              mode == TextureStage::M_normal_gloss) {
            if (si++ == spec._stage) {
              sampler = texattrib->get_on_sampler(stage);
              view += stage->get_tex_view_offset();
              return texattrib->get_on_texture(stage);
            }
          }
        }
      }
    }
    break;

  case Shader::STO_stage_height_i:
    {
      const TextureAttrib *texattrib;
      if (_target_rs->get_attrib(texattrib)) {
        int si = 0;
        for (int i = 0; i < texattrib->get_num_on_stages(); ++i) {
          TextureStage *stage = texattrib->get_on_stage(i);
          TextureStage::Mode mode = stage->get_mode();

          if (mode == TextureStage::M_height ||
              mode == TextureStage::M_normal_height) {
            if (si++ == spec._stage) {
              sampler = texattrib->get_on_sampler(stage);
              view += stage->get_tex_view_offset();
              return texattrib->get_on_texture(stage);
            }
          }
        }
      }

      if (default_normal_height_tex == nullptr) {
        PT(Texture) tex = new Texture("default-normal-height");
        tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba);
        tex->set_clear_color(LColor(0.5, 0.5, 1, 0));
        default_normal_height_tex = std::move(tex);
      }
      return default_normal_height_tex;
    }

  case Shader::STO_stage_selector_i:
    {
      const TextureAttrib *texattrib;
      if (_target_rs->get_attrib(texattrib)) {
        int si = 0;
        for (int i = 0; i < texattrib->get_num_on_stages(); ++i) {
          TextureStage *stage = texattrib->get_on_stage(i);
          TextureStage::Mode mode = stage->get_mode();

          if (mode == TextureStage::M_selector) {
            if (si++ == spec._stage) {
              sampler = texattrib->get_on_sampler(stage);
              view += stage->get_tex_view_offset();
              return texattrib->get_on_texture(stage);
            }
          }
        }
      }
    }
    break;

  case Shader::STO_stage_emission_i:
    {
      const TextureAttrib *texattrib;
      if (_target_rs->get_attrib(texattrib)) {
        int si = 0;
        for (int i = 0; i < texattrib->get_num_on_stages(); ++i) {
          TextureStage *stage = texattrib->get_on_stage(i);
          TextureStage::Mode mode = stage->get_mode();

          if (mode == TextureStage::M_emission) {
            if (si++ == spec._stage) {
              sampler = texattrib->get_on_sampler(stage);
              view += stage->get_tex_view_offset();
              return texattrib->get_on_texture(stage);
            }
          }
        }
      }
    }
    break;

  default:
    nassertr(false, nullptr);
    break;
  }

  return nullptr;
}

/**
 * Return a pointer to struct ShaderPtrData
 */
const Shader::ShaderPtrData *GraphicsStateGuardian::
fetch_ptr_parameter(const Shader::ShaderPtrSpec& spec) {
  return (_target_shader->get_shader_input_ptr(spec._arg));
}

/**
 *
 */
bool GraphicsStateGuardian::
fetch_ptr_parameter(const Shader::ShaderPtrSpec& spec, Shader::ShaderPtrData &data) {
  return _target_shader->get_shader_input_ptr(spec._arg, data);
}

/**
 * Makes the specified DisplayRegion current.  All future drawing and clear
 * operations will be constrained within the given DisplayRegion.
 */
void GraphicsStateGuardian::
prepare_display_region(DisplayRegionPipelineReader *dr) {
  _current_display_region = dr->get_object();
  _current_stereo_channel = dr->get_stereo_channel();
  _current_tex_view_offset = dr->get_tex_view_offset();
  _effective_incomplete_render = _incomplete_render && _current_display_region->get_incomplete_render();

  _stereo_buffer_mask = ~0;

  Lens::StereoChannel output_channel = dr->get_stereo_channel();
  if (dr->get_window()->get_swap_eyes()) {
    // Reverse the output channel.
    switch (output_channel) {
    case Lens::SC_left:
      output_channel = Lens::SC_right;
      break;

    case Lens::SC_right:
      output_channel = Lens::SC_left;
      break;

    default:
      break;
    }
  }

  switch (output_channel) {
  case Lens::SC_left:
    _color_write_mask = dr->get_window()->get_left_eye_color_mask();
    if (_current_properties->is_stereo()) {
      _stereo_buffer_mask = ~RenderBuffer::T_right;
    }
    break;

  case Lens::SC_right:
    _color_write_mask = dr->get_window()->get_right_eye_color_mask();
    if (_current_properties->is_stereo()) {
      _stereo_buffer_mask = ~RenderBuffer::T_left;
    }
    break;

  case Lens::SC_mono:
  case Lens::SC_stereo:
    _color_write_mask = ColorWriteAttrib::C_all;
  }
}

/**
 * Resets any non-standard graphics state that might give a callback apoplexy.
 * Some drivers require that the graphics state be restored to neutral before
 * performing certain operations.  In OpenGL, for instance, this closes any
 * open vertex buffers.
 */
void GraphicsStateGuardian::
clear_before_callback() {
}

/**
 * Forgets the current graphics state and current transform, so that the next
 * call to set_state_and_transform() will have to reload everything.  This is
 * a good thing to call when you are no longer sure what the graphics state
 * is.  This should only be called from the draw thread.
 */
void GraphicsStateGuardian::
clear_state_and_transform() {
  // Re-issue the modelview and projection transforms.
  reissue_transforms();

  // Now clear the state flags to unknown.
  _state_rs = RenderState::make_empty();
  _state_mask.clear();
}

/**
 * This is simply a transparent call to GraphicsEngine::remove_window().  It
 * exists primary to support removing a window from that compiles before the
 * display module, and therefore has no knowledge of a GraphicsEngine object.
 */
void GraphicsStateGuardian::
remove_window(GraphicsOutputBase *window) {
  nassertv(_engine != nullptr);
  GraphicsOutput *win;
  DCAST_INTO_V(win, window);
  _engine->remove_window(win);
}

/**
 * Makes the current lens (whichever lens was most recently specified with
 * set_scene()) active, so that it will transform future rendered geometry.
 * Normally this is only called from the draw process, and usually it is
 * called by set_scene().
 *
 * The return value is true if the lens is acceptable, false if it is not.
 */
bool GraphicsStateGuardian::
prepare_lens() {
  return false;
}

/**
 * Given a lens, this function calculates the appropriate projection matrix
 * for this gsg.  The result depends on the peculiarities of the rendering
 * API.
 */
CPT(TransformState) GraphicsStateGuardian::
calc_projection_mat(const Lens *lens) {
  if (lens == nullptr) {
    return nullptr;
  }

  if (!lens->is_linear()) {
    return nullptr;
  }

  return TransformState::make_identity();
}

/**
 * Called before each frame is rendered, to allow the GSG a chance to do any
 * internal cleanup before beginning the frame.
 *
 * The return value is true if successful (in which case the frame will be
 * drawn and end_frame() will be called later), or false if unsuccessful (in
 * which case nothing will be drawn and end_frame() will not be called).
 */
bool GraphicsStateGuardian::
begin_frame(Thread *current_thread) {
  {
    PStatTimer timer(_prepare_pcollector);
    _prepared_objects->begin_frame(this, current_thread);
  }

  // We should reset the state to the default at the beginning of every frame.
  // Although this will incur additional overhead, particularly in a simple
  // scene, it helps ensure that states that have changed properties since
  // last time without changing attribute pointers--like textures, lighting,
  // or fog--will still be accurately updated.
  _state_rs = RenderState::make_empty();
  _state_mask.clear();

  return !_needs_reset;
}

/**
 * Called between begin_frame() and end_frame() to mark the beginning of
 * drawing commands for a "scene" (usually a particular DisplayRegion) within
 * a frame.  All 3-D drawing commands, except the clear operation, must be
 * enclosed within begin_scene() .. end_scene(). This must be called in the
 * draw thread.
 *
 * The return value is true if successful (in which case the scene will be
 * drawn and end_scene() will be called later), or false if unsuccessful (in
 * which case nothing will be drawn and end_scene() will not be called).
 */
bool GraphicsStateGuardian::
begin_scene() {
  return true;
}

/**
 * Called between begin_frame() and end_frame() to mark the end of drawing
 * commands for a "scene" (usually a particular DisplayRegion) within a frame.
 * All 3-D drawing commands, except the clear operation, must be enclosed
 * within begin_scene() .. end_scene().
 */
void GraphicsStateGuardian::
end_scene() {
  // We should clear this pointer now, so that we don't keep unneeded
  // reference counts dangling.  We keep around a "null" scene setup object
  // instead of using a null pointer to avoid special-case code in
  // set_state_and_transform.
  _scene_setup = _scene_null;

  // Undo any lighting we had enabled last scene, to force the lights to be
  // reissued, in case their parameters or positions have changed between
  // scenes.
  int i;
  for (i = 0; i < _num_lights_enabled; ++i) {
    enable_light(i, false);
  }
  _num_lights_enabled = 0;

  // Ditto for the clipping planes.
  for (i = 0; i < _num_clip_planes_enabled; ++i) {
    enable_clip_plane(i, false);
  }
  _num_clip_planes_enabled = 0;

  // Put the state into the 'unknown' state, forcing a reload.
  _state_rs = RenderState::make_empty();
  _state_mask.clear();
}

/**
 * Called after each frame is rendered, to allow the GSG a chance to do any
 * internal cleanup after rendering the frame, and before the window flips.
 */
void GraphicsStateGuardian::
end_frame(Thread *current_thread) {
  _prepared_objects->end_frame(current_thread);

  // Flush any PStatCollectors.
  _data_transferred_pcollector.flush_level();

  _primitive_batches_pcollector.flush_level();
  _primitive_batches_tristrip_pcollector.flush_level();
  _primitive_batches_trifan_pcollector.flush_level();
  _primitive_batches_tri_pcollector.flush_level();
  _primitive_batches_patch_pcollector.flush_level();
  _primitive_batches_other_pcollector.flush_level();
  _vertices_tristrip_pcollector.flush_level();
  _vertices_trifan_pcollector.flush_level();
  _vertices_tri_pcollector.flush_level();
  _vertices_patch_pcollector.flush_level();
  _vertices_other_pcollector.flush_level();
  _compute_work_groups_pcollector.flush_level();

  _state_pcollector.flush_level();
  _texture_state_pcollector.flush_level();
  _transform_state_pcollector.flush_level();
  _draw_primitive_pcollector.flush_level();

  // Evict any textures andor vbuffers that exceed our texture memory.
  _prepared_objects->_graphics_memory_lru.begin_epoch();
}

/**
 * Returns true if this GSG can implement decals using a DepthOffsetAttrib, or
 * false if that is unreliable and the three-step rendering process should be
 * used instead.
 */
bool GraphicsStateGuardian::
depth_offset_decals() {
  return true;
}

/**
 * Called during draw to begin a three-step rendering phase to draw decals.
 * The first step, begin_decal_base_first(), is called prior to drawing the
 * base geometry.  It should set up whatever internal state is appropriate, as
 * well as returning a RenderState object that should be applied to the base
 * geometry for rendering.
 */
CPT(RenderState) GraphicsStateGuardian::
begin_decal_base_first() {
  // Turn off writing the depth buffer to render the base geometry.
  static CPT(RenderState) decal_base_first;
  if (decal_base_first == nullptr) {
    decal_base_first = RenderState::make
      (DepthWriteAttrib::make(DepthWriteAttrib::M_off),
       RenderState::get_max_priority());
  }
  return decal_base_first;
}

/**
 * Called during draw to begin a three-step rendering phase to draw decals.
 * The second step, begin_decal_nested(), is called after drawing the base
 * geometry and prior to drawing any of the nested decal geometry that is to
 * be applied to the base geometry.
 */
CPT(RenderState) GraphicsStateGuardian::
begin_decal_nested() {
  // We should keep the depth buffer off during this operation, so that decals
  // on decals will render properly.
  static CPT(RenderState) decal_nested;
  if (decal_nested == nullptr) {
    decal_nested = RenderState::make
      (DepthWriteAttrib::make(DepthWriteAttrib::M_off),
       RenderState::get_max_priority());
  }
  return decal_nested;
}

/**
 * Called during draw to begin a three-step rendering phase to draw decals.
 * The third step, begin_decal_base_second(), is called after drawing the base
 * geometry and the nested decal geometry, and prior to drawing the base
 * geometry one more time (if needed).
 *
 * It should return a RenderState object appropriate for rendering the base
 * geometry the second time, or NULL if it is not necessary to re-render the
 * base geometry.
 */
CPT(RenderState) GraphicsStateGuardian::
begin_decal_base_second() {
  // Now let the depth buffer go back on, but turn off writing the color
  // buffer to render the base geometry after the second pass.  Also, turn off
  // texturing since there's no need for it now.
  static CPT(RenderState) decal_base_second;
  if (decal_base_second == nullptr) {
    decal_base_second = RenderState::make
      (ColorWriteAttrib::make(ColorWriteAttrib::C_off),
       // On reflection, we need to leave texturing on so the alpha test
       // mechanism can work (if it is enabled, e.g.  we are rendering an
       // object with M_dual transparency). TextureAttrib::make_off(),
       RenderState::get_max_priority());
  }
  return decal_base_second;
}

/**
 * Called during draw to clean up after decals are finished.
 */
void GraphicsStateGuardian::
finish_decal() {
  // No need to do anything special here.
}

/**
 * Called before a sequence of draw_primitive() functions are called, this
 * should prepare the vertex data for rendering.  It returns true if the
 * vertices are ok, false to abort this group of primitives.
 */
bool GraphicsStateGuardian::
begin_draw_primitives(const GeomPipelineReader *geom_reader,
                      const GeomVertexDataPipelineReader *data_reader,
                      size_t num_instances, bool force) {
  _data_reader = data_reader;

  if (num_instances == 0) {
    return false;
  }

  // Always draw if we have a shader, since the shader might use a different
  // mechanism for fetching vertex data.
  return _data_reader->has_vertex() || (_target_shader && _target_shader->has_shader());
}

/**
 * Draws a series of disconnected triangles.
 */
bool GraphicsStateGuardian::
draw_triangles(const GeomPrimitivePipelineReader *, bool) {
  return false;
}


/**
 * Draws a series of disconnected triangles with adjacency information.
 */
bool GraphicsStateGuardian::
draw_triangles_adj(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of triangle strips.
 */
bool GraphicsStateGuardian::
draw_tristrips(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of triangle strips with adjacency information.
 */
bool GraphicsStateGuardian::
draw_tristrips_adj(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of triangle fans.
 */
bool GraphicsStateGuardian::
draw_trifans(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of "patches", which can only be processed by a tessellation
 * shader.
 */
bool GraphicsStateGuardian::
draw_patches(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of disconnected line segments.
 */
bool GraphicsStateGuardian::
draw_lines(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of disconnected line segments with adjacency information.
 */
bool GraphicsStateGuardian::
draw_lines_adj(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of line strips.
 */
bool GraphicsStateGuardian::
draw_linestrips(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of line strips with adjacency information.
 */
bool GraphicsStateGuardian::
draw_linestrips_adj(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of disconnected points.
 */
bool GraphicsStateGuardian::
draw_points(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Called after a sequence of draw_primitive() functions are called, this
 * should do whatever cleanup is appropriate.
 */
void GraphicsStateGuardian::
end_draw_primitives() {
  _data_reader = nullptr;
}

/**
 * Resets all internal state as if the gsg were newly created.
 */
void GraphicsStateGuardian::
reset() {
  _needs_reset = false;
  _is_valid = false;

  _state_rs = RenderState::make_empty();
  _target_rs = nullptr;
  _state_mask.clear();
  _inv_state_mask = RenderState::SlotMask::all_on();
  _internal_transform = _cs_transform;
  _scene_null = new SceneSetup;
  _scene_setup = _scene_null;

  _color_write_mask = ColorWriteAttrib::C_all;

  _has_scene_graph_color = false;
  _scene_graph_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _transform_stale = true;
  _color_blend_involves_color_scale = false;
  _texture_involves_color_scale = false;
  _vertex_colors_enabled = true;
  _lighting_enabled = false;
  _num_lights_enabled = 0;
  _num_clip_planes_enabled = 0;
  _clip_planes_enabled = false;

  _color_scale_enabled = false;
  _current_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
  _has_texture_alpha_scale = false;

  _has_material_force_color = false;
  _material_force_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _light_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);

  _tex_gen_modifies_mat = false;
  _last_max_stage_index = 0;

  _is_valid = true;
}

/**
 * Simultaneously resets the render state and the transform state.
 *
 * This transform specified is the "internal" net transform, already converted
 * into the GSG's internal coordinate space by composing it to
 * get_cs_transform().  (Previously, this used to be the "external" net
 * transform, with the assumption that that GSG would convert it internally,
 * but that is no longer the case.)
 *
 * Special case: if (state==NULL), then the target state is already stored in
 * _target.
 */
void GraphicsStateGuardian::
set_state_and_transform(const RenderState *state,
                        const TransformState *trans) {
}

/**
 * Clears the framebuffer within the current DisplayRegion, according to the
 * flags indicated by the given DrawableRegion object.
 *
 * This does not set the DisplayRegion first.  You should call
 * prepare_display_region() to specify the region you wish the clear operation
 * to apply to.
 */
void GraphicsStateGuardian::
clear(DrawableRegion *clearable) {
}

/**
 * Returns a RenderBuffer object suitable for operating on the requested set
 * of buffers.  buffer_type is the union of all the desired RenderBuffer::Type
 * values.
 */
RenderBuffer GraphicsStateGuardian::
get_render_buffer(int buffer_type, const FrameBufferProperties &prop) {
  return RenderBuffer(this, buffer_type & prop.get_buffer_mask() & _stereo_buffer_mask);
}

/**
 * Returns what the cs_transform would be set to after a call to
 * set_coordinate_system(cs).  This is another way of saying the cs_transform
 * when rendering the scene for a camera with the indicated coordinate system.
 */
CPT(TransformState) GraphicsStateGuardian::
get_cs_transform_for(CoordinateSystem cs) const {
  if (_coordinate_system == cs) {
    // We've already calculated this.
    return _cs_transform;

  } else if (_internal_coordinate_system == CS_default ||
             _internal_coordinate_system == cs) {
    return TransformState::make_identity();

  } else {
    return TransformState::make_mat
      (LMatrix4::convert_mat(cs, _internal_coordinate_system));
  }
}

/**
 * Returns a transform that converts from the GSG's external coordinate system
 * (as returned by get_coordinate_system()) to its internal coordinate system
 * (as returned by get_internal_coordinate_system()).  This is used for
 * rendering.
 */
CPT(TransformState) GraphicsStateGuardian::
get_cs_transform() const {
  return _cs_transform;
}

/**
 * This is fundametically similar to do_issue_light(), with calls to
 * apply_clip_plane() and enable_clip_planes(), as appropriate.
 */
void GraphicsStateGuardian::
do_issue_clip_plane() {
  int num_enabled = 0;
  int num_on_planes = 0;

  const ClipPlaneAttrib *target_clip_plane = (const ClipPlaneAttrib *)
    _target_rs->get_attrib_def(ClipPlaneAttrib::get_class_slot());

  if (target_clip_plane != nullptr) {
    CPT(ClipPlaneAttrib) new_plane = target_clip_plane->filter_to_max(_max_clip_planes);

    num_on_planes = new_plane->get_num_on_planes();
    for (int li = 0; li < num_on_planes; li++) {
      NodePath plane = new_plane->get_on_plane(li);
      nassertv(!plane.is_empty());
      PlaneNode *plane_node;
      DCAST_INTO_V(plane_node, plane.node());
      if ((plane_node->get_clip_effect() & PlaneNode::CE_visible) != 0) {
        // Clipping should be enabled before we apply any planes.
        if (!_clip_planes_enabled) {
          enable_clip_planes(true);
          _clip_planes_enabled = true;
        }

        enable_clip_plane(num_enabled, true);
        if (num_enabled == 0) {
          begin_bind_clip_planes();
        }

        bind_clip_plane(plane, num_enabled);
        num_enabled++;
      }
    }
  }

  int i;
  for (i = num_enabled; i < _num_clip_planes_enabled; ++i) {
    enable_clip_plane(i, false);
  }
  _num_clip_planes_enabled = num_enabled;

  // If no planes were set, disable clipping
  if (num_enabled == 0) {
    if (_clip_planes_enabled) {
      enable_clip_planes(false);
      _clip_planes_enabled = false;
    }
  } else {
    end_bind_clip_planes();
  }
}

/**
 * This method is defined in the base class because it is likely that this
 * functionality will be used for all (or at least most) kinds of
 * GraphicsStateGuardians--it's not specific to any one rendering backend.
 *
 * The ColorAttribute just changes the interpretation of the color on the
 * vertices, and fiddles with _vertex_colors_enabled, etc.
 */
void GraphicsStateGuardian::
do_issue_color() {
  const ColorAttrib *target_color = (const ColorAttrib *)
    _target_rs->get_attrib_def(ColorAttrib::get_class_slot());

  switch (target_color->get_color_type()) {
  case ColorAttrib::T_flat:
    // Color attribute flat: it specifies a scene graph color that overrides
    // the vertex color.
    _scene_graph_color = target_color->get_color();
    _has_scene_graph_color = true;
    _vertex_colors_enabled = false;
    break;

  case ColorAttrib::T_off:
    // Color attribute off: it specifies that no scene graph color is in
    // effect, and vertex color is not important either.
    _scene_graph_color.set(1.0f, 1.0f, 1.0f, 1.0f);
    _has_scene_graph_color = false;
    _vertex_colors_enabled = false;
    break;

  case ColorAttrib::T_vertex:
    // Color attribute vertex: it specifies that vertex color should be
    // revealed.
    _scene_graph_color.set(1.0f, 1.0f, 1.0f, 1.0f);
    _has_scene_graph_color = false;
    _vertex_colors_enabled = true;
    break;
  }

  if (_color_scale_via_lighting) {
    _state_mask.clear_bit(LightAttrib::get_class_slot());
    _state_mask.clear_bit(MaterialAttrib::get_class_slot());

    determine_light_color_scale();
  }
}

/**
 *
 */
void GraphicsStateGuardian::
do_issue_color_scale() {
  // If the previous color scale had set a special texture, clear the texture
  // now.
  if (_has_texture_alpha_scale) {
    _state_mask.clear_bit(TextureAttrib::get_class_slot());
  }

  const ColorScaleAttrib *target_color_scale = (const ColorScaleAttrib *)
    _target_rs->get_attrib_def(ColorScaleAttrib::get_class_slot());

  _color_scale_enabled = target_color_scale->has_scale();
  _current_color_scale = target_color_scale->get_scale();
  _has_texture_alpha_scale = false;

  if (_color_blend_involves_color_scale) {
    _state_mask.clear_bit(TransparencyAttrib::get_class_slot());
  }
  if (_texture_involves_color_scale) {
    _state_mask.clear_bit(TextureAttrib::get_class_slot());
  }
  if (_color_scale_via_lighting) {
    _state_mask.clear_bit(LightAttrib::get_class_slot());
    _state_mask.clear_bit(MaterialAttrib::get_class_slot());

    determine_light_color_scale();
  }

  if (_alpha_scale_via_texture && !_has_scene_graph_color &&
      _vertex_colors_enabled && target_color_scale->has_alpha_scale()) {
    // This color scale will set a special texture--so again, clear the
    // texture.
    _state_mask.clear_bit(TextureAttrib::get_class_slot());
    _state_mask.clear_bit(TexMatrixAttrib::get_class_slot());

    _has_texture_alpha_scale = true;
  }
}

/**
 * This implementation of do_issue_light() assumes we have a limited number of
 * hardware lights available.  This function assigns each light to a different
 * hardware light id, trying to keep each light associated with the same id
 * where possible, but reusing id's when necessary.  When it is no longer
 * possible to reuse existing id's (e.g.  all id's are in use), the next
 * sequential id is assigned (if available).
 *
 * It will call apply_light() each time a light is assigned to a particular id
 * for the first time in a given frame, and it will subsequently call
 * enable_light() to enable or disable each light as the frame is rendered, as
 * well as enable_lighting() to enable or disable overall lighting.
 */
void GraphicsStateGuardian::
do_issue_light() {
  // Initialize the current ambient light total and newly enabled light list
  LColor cur_ambient_light(0.0f, 0.0f, 0.0f, 0.0f);
  int i;

  int num_enabled = 0;
  bool any_on_lights = false;

  const LightAttrib *target_light;
  _target_rs->get_attrib_def(target_light);

  if (display_cat.is_spam()) {
    display_cat.spam()
      << "do_issue_light: " << target_light << "\n";
  }
  if (target_light != nullptr) {
    // LightAttrib guarantees that the on lights are sorted, and that
    // non-ambient lights come before ambient lights.
    any_on_lights = target_light->has_any_on_light();
    size_t filtered_lights = std::min((size_t)_max_lights, target_light->get_num_non_ambient_lights());
    for (size_t li = 0; li < filtered_lights; ++li) {
      NodePath light = target_light->get_on_light(li);
      nassertv(!light.is_empty());
      Light *light_obj = light.node()->as_light();
      nassertv(light_obj != nullptr);

      // Lighting should be enabled before we apply any lights.
      if (!_lighting_enabled) {
        enable_lighting(true);
        _lighting_enabled = true;
      }

      const LColor &color = light_obj->get_color();
      // Don't bother binding the light if it has no color to contribute.
      if (color[0] != 0.0 || color[1] != 0.0 || color[2] != 0.0) {
        enable_light(num_enabled, true);
        if (num_enabled == 0) {
          begin_bind_lights();
        }

        light_obj->bind(this, light, num_enabled);
        num_enabled++;
      }
    }
  }

  for (i = num_enabled; i < _num_lights_enabled; ++i) {
    enable_light(i, false);
  }
  _num_lights_enabled = num_enabled;

  // If no lights were set, disable lighting
  if (!any_on_lights) {
    if (_color_scale_via_lighting && (_has_material_force_color || _light_color_scale != LVecBase4(1.0f, 1.0f, 1.0f, 1.0f))) {
      // Unless we need lighting anyway to apply a color or color scale.
      if (!_lighting_enabled) {
        enable_lighting(true);
        _lighting_enabled = true;
      }
      set_ambient_light(LColor(1.0f, 1.0f, 1.0f, 1.0f));

    } else {
      if (_lighting_enabled) {
        enable_lighting(false);
        _lighting_enabled = false;
      }
    }

  } else {
    // Don't forget to still enable lighting if we have only an ambient light.
    if (!_lighting_enabled) {
      enable_lighting(true);
      _lighting_enabled = true;
    }

    set_ambient_light(target_light->get_ambient_contribution());
  }

  if (num_enabled != 0) {
    end_bind_lights();
  }
}

/**
 * Copy the pixels within the indicated display region from the framebuffer
 * into texture memory.
 *
 * If z > -1, it is the cube map index into which to copy.
 */
bool GraphicsStateGuardian::
framebuffer_copy_to_texture(Texture *, int, int, const DisplayRegion *,
                            const RenderBuffer &) {
  return false;
}


/**
 * Copy the pixels within the indicated display region from the framebuffer
 * into system memory, not texture memory.  Returns true on success, false on
 * failure.
 *
 * If a future is given, the operation may be scheduled to occur in the
 * background, in which case the texture will be passed as the result of the
 * future when the operation is complete.
 *
 * This completely redefines the ram image of the indicated texture.
 */
bool GraphicsStateGuardian::
framebuffer_copy_to_ram(Texture *, int, int, const DisplayRegion *,
                        const RenderBuffer &, ScreenshotRequest *) {
  return false;
}

/**
 * Called the first time a particular light has been bound to a given id
 * within a frame, this should set up the associated hardware light with the
 * light's properties.
 */
void GraphicsStateGuardian::
bind_light(PointLight *light_obj, const NodePath &light, int light_id) {
}

/**
 * Called the first time a particular light has been bound to a given id
 * within a frame, this should set up the associated hardware light with the
 * light's properties.
 */
void GraphicsStateGuardian::
bind_light(DirectionalLight *light_obj, const NodePath &light, int light_id) {
}

/**
 * Called the first time a particular light has been bound to a given id
 * within a frame, this should set up the associated hardware light with the
 * light's properties.
 */
void GraphicsStateGuardian::
bind_light(Spotlight *light_obj, const NodePath &light, int light_id) {
}

#ifdef DO_PSTATS
/**
 * Initializes the relevant PStats data at the beginning of the frame.
 */
void GraphicsStateGuardian::
init_frame_pstats() {
  if (PStatClient::is_connected()) {
    _data_transferred_pcollector.clear_level();
    //_vertex_buffer_switch_pcollector.clear_level();
    //_index_buffer_switch_pcollector.clear_level();
    //_shader_buffer_switch_pcollector.clear_level();

    _primitive_batches_pcollector.clear_level();
    _primitive_batches_tristrip_pcollector.clear_level();
    _primitive_batches_trifan_pcollector.clear_level();
    _primitive_batches_tri_pcollector.clear_level();
    _primitive_batches_patch_pcollector.clear_level();
    _primitive_batches_other_pcollector.clear_level();
    _vertices_tristrip_pcollector.clear_level();
    _vertices_trifan_pcollector.clear_level();
    _vertices_tri_pcollector.clear_level();
    _vertices_patch_pcollector.clear_level();
    _vertices_other_pcollector.clear_level();
    _compute_work_groups_pcollector.clear_level();

    _state_pcollector.clear_level();
    _transform_state_pcollector.clear_level();
    _texture_state_pcollector.clear_level();
  }
}

/**
 * Returns a PStatThread used to represent this GL context.
 */
PStatThread GraphicsStateGuardian::
get_pstats_thread() {
  PStatClient *client = PStatClient::get_global_pstats();
  if (_pstats_gpu_thread == -1) {
    _pstats_gpu_thread = client->make_gpu_thread("GPU").get_index();
  }
  return PStatThread(client, _pstats_gpu_thread);
}
#endif  // DO_PSTATS

/**
 * Create a gamma table.
 */
void GraphicsStateGuardian::
create_gamma_table (PN_stdfloat gamma, unsigned short *red_table, unsigned short *green_table, unsigned short *blue_table) {
  int i;

  if (gamma <= 0.0) {
    // avoid divide by zero and negative exponents
    gamma = 1.0;
  }

  for (i = 0; i < 256; i++) {
    double g;
    double x;
    PN_stdfloat gamma_correction;

    x = ((double) i / 255.0);
    gamma_correction = 1.0 / gamma;
    x = pow (x, (double) gamma_correction);
    if (x > 1.00) {
      x = 1.0;
    }

    g = x * 65535.0;
    red_table [i] = (int)g;
    green_table [i] = (int)g;
    blue_table [i] = (int)g;
  }
}

/**
 * Called by clear_state_and_transform() to ensure that the current modelview
 * and projection matrices are properly loaded in the graphics state, after a
 * callback might have mucked them up.
 */
void GraphicsStateGuardian::
reissue_transforms() {
}

/**
 * Intended to be overridden by a derived class to enable or disable the use
 * of lighting overall.  This is called by do_issue_light() according to
 * whether any lights are in use or not.
 */
void GraphicsStateGuardian::
enable_lighting(bool enable) {
}

/**
 * Intended to be overridden by a derived class to indicate the color of the
 * ambient light that should be in effect.  This is called by do_issue_light()
 * after all other lights have been enabled or disabled.
 */
void GraphicsStateGuardian::
set_ambient_light(const LColor &color) {
}

/**
 * Intended to be overridden by a derived class to enable the indicated light
 * id.  A specific Light will already have been bound to this id via
 * bind_light().
 */
void GraphicsStateGuardian::
enable_light(int light_id, bool enable) {
}

/**
 * Called immediately before bind_light() is called, this is intended to
 * provide the derived class a hook in which to set up some state (like
 * transform) that might apply to several lights.
 *
 * The sequence is: begin_bind_lights() will be called, then one or more
 * bind_light() calls, then end_bind_lights().
 */
void GraphicsStateGuardian::
begin_bind_lights() {
}

/**
 * Called after before bind_light() has been called one or more times (but
 * before any geometry is issued or additional state is changed), this is
 * intended to clean up any temporary changes to the state that may have been
 * made by begin_bind_lights().
 */
void GraphicsStateGuardian::
end_bind_lights() {
}

/**
 * Intended to be overridden by a derived class to enable or disable the use
 * of clipping planes overall.  This is called by do_issue_clip_plane()
 * according to whether any planes are in use or not.
 */
void GraphicsStateGuardian::
enable_clip_planes(bool enable) {
}

/**
 * Intended to be overridden by a derived class to enable the indicated plane
 * id.  A specific PlaneNode will already have been bound to this id via
 * bind_clip_plane().
 */
void GraphicsStateGuardian::
enable_clip_plane(int plane_id, bool enable) {
}

/**
 * Called immediately before bind_clip_plane() is called, this is intended to
 * provide the derived class a hook in which to set up some state (like
 * transform) that might apply to several planes.
 *
 * The sequence is: begin_bind_clip_planes() will be called, then one or more
 * bind_clip_plane() calls, then end_bind_clip_planes().
 */
void GraphicsStateGuardian::
begin_bind_clip_planes() {
}

/**
 * Called the first time a particular clipping plane has been bound to a given
 * id within a frame, this should set up the associated hardware (or API)
 * clipping plane with the plane's properties.
 */
void GraphicsStateGuardian::
bind_clip_plane(const NodePath &plane, int plane_id) {
}

/**
 * Called after before bind_clip_plane() has been called one or more times
 * (but before any geometry is issued or additional state is changed), this is
 * intended to clean up any temporary changes to the state that may have been
 * made by begin_bind_clip_planes().
 */
void GraphicsStateGuardian::
end_bind_clip_planes() {
}

/**
 * Assigns _target_texture and _target_tex_gen based on the _target_rs.
 */
void GraphicsStateGuardian::
determine_target_texture() {
  const TextureAttrib *target_texture = (const TextureAttrib *)
    _target_rs->get_attrib_def(TextureAttrib::get_class_slot());
  const TexGenAttrib *target_tex_gen = (const TexGenAttrib *)
    _target_rs->get_attrib_def(TexGenAttrib::get_class_slot());

  nassertv(target_texture != nullptr &&
           target_tex_gen != nullptr);
  _target_texture = target_texture;
  _target_tex_gen = target_tex_gen;

  if (_has_texture_alpha_scale) {
    PT(TextureStage) stage = get_alpha_scale_texture_stage();
    PT(Texture) texture = TexturePool::get_alpha_scale_map();

    _target_texture = DCAST(TextureAttrib, _target_texture->add_on_stage(stage, texture));
    _target_tex_gen = DCAST(TexGenAttrib, _target_tex_gen->add_stage
                               (stage, TexGenAttrib::M_constant, LTexCoord3(_current_color_scale[3], 0.0f, 0.0f)));
  }

  int max_texture_stages = get_max_texture_stages();
  _target_texture = _target_texture->filter_to_max(max_texture_stages);
  nassertv(_target_texture->get_num_on_stages() <= max_texture_stages);
}

/**
 * Assigns _target_shader based on the _target_rs.
 */
void GraphicsStateGuardian::
determine_target_shader() {
  if (_target_rs->_generated_shader != nullptr) {
    _target_shader = (const ShaderAttrib *)_target_rs->_generated_shader.p();
  } else {
    _target_shader = (const ShaderAttrib *)
      _target_rs->get_attrib_def(ShaderAttrib::get_class_slot());
  }
}

/**
 * Frees some memory that was explicitly allocated within the glgsg.
 */
void GraphicsStateGuardian::
free_pointers() {
}

/**
 * This is called by the associated GraphicsWindow when close_window() is
 * called.  It should null out the _win pointer and possibly free any open
 * resources associated with the GSG.
 */
void GraphicsStateGuardian::
close_gsg() {
  // Protect from multiple calls, and also inform any other functions not to
  // try to create new stuff while we're going down.
  if (_closing_gsg) {
    return;
  }
  _closing_gsg = true;

  if (display_cat.is_debug()) {
    display_cat.debug()
      << this << " close_gsg " << get_type() << "\n";
  }

  // As tempting as it may be to try to release all the textures and geoms
  // now, we can't, because we might not be the currently-active GSG (this is
  // particularly important in OpenGL, which maintains one currently-active GL
  // state in each thread).  If we start deleting textures, we'll be
  // inadvertently deleting textures from some other OpenGL state.

  // Fortunately, it doesn't really matter, since the graphics API will be
  // responsible for cleaning up anything we don't clean up explicitly.  We'll
  // just let them drop.

  // Make sure that all the contexts belonging to the GSG are deleted.
  _prepared_objects.clear();

  free_pointers();
}

/**
 * This is called internally when it is determined that things are just fubar.
 * It temporarily deactivates the GSG just so things don't get out of hand,
 * and throws an event so the application can deal with this if it needs to.
 */
void GraphicsStateGuardian::
panic_deactivate() {
  if (_active) {
    display_cat.error()
      << "Deactivating " << get_type() << ".\n";
    set_active(false);
    throw_event("panic-deactivate-gsg", this);
  }
}

/**
 * Called whenever the color or color scale is changed, if
 * _color_scale_via_lighting is true.  This will rederive
 * _material_force_color and _light_color_scale appropriately.
 */
void GraphicsStateGuardian::
determine_light_color_scale() {
  if (_has_scene_graph_color) {
    // If we have a scene graph color, it, plus the color scale, goes directly
    // into the material; we don't color scale the lights--this allows an
    // alpha color scale to work properly.
    _has_material_force_color = true;
    _material_force_color = _scene_graph_color;
    _light_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
    if (!_color_blend_involves_color_scale && _color_scale_enabled) {
      _material_force_color.set(_scene_graph_color[0] * _current_color_scale[0],
                                _scene_graph_color[1] * _current_color_scale[1],
                                _scene_graph_color[2] * _current_color_scale[2],
                                _scene_graph_color[3] * _current_color_scale[3]);
    }

  } else if (!_vertex_colors_enabled) {
    // We don't have a scene graph color, but we don't want to enable vertex
    // colors either, so we still need to force a white material color in
    // absence of any other color.
    _has_material_force_color = true;
    _material_force_color.set(1.0f, 1.0f, 1.0f, 1.0f);
    _light_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
    if (!_color_blend_involves_color_scale && _color_scale_enabled) {
      _material_force_color.componentwise_mult(_current_color_scale);
    }

  } else {
    // Otherise, leave the materials alone, but we might still scale the
    // lights.
    _has_material_force_color = false;
    _light_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
    if (!_color_blend_involves_color_scale && _color_scale_enabled) {
      _light_color_scale = _current_color_scale;
    }
  }
}

/**
 *
 */
CPT(RenderState) GraphicsStateGuardian::
get_unlit_state() {
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make(LightAttrib::make_all_off());
  }
  return state;
}

/**
 *
 */
CPT(RenderState) GraphicsStateGuardian::
get_unclipped_state() {
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make(ClipPlaneAttrib::make_all_off());
  }
  return state;
}

/**
 *
 */
CPT(RenderState) GraphicsStateGuardian::
get_untextured_state() {
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make(TextureAttrib::make_off());
  }
  return state;
}

/**
 * Should be called when a texture is encountered that needs to have its RAM
 * image reloaded, and get_incomplete_render() is true.  This will fire off a
 * thread on the current Loader object that will request the texture to load
 * its image.  The image will be available at some point in the future.
 * @returns a future object that can be used to check its status.
 */
AsyncFuture *GraphicsStateGuardian::
async_reload_texture(TextureContext *tc) {
  nassertr(_loader != nullptr, nullptr);

  int priority = 0;
  if (_current_display_region != nullptr) {
    priority = _current_display_region->get_texture_reload_priority();
  }

  Texture *tex = tc->get_texture();
  return tex->async_ensure_ram_image(_supports_compressed_texture, priority);
}

/**
 * Returns a shadow map for the given light source.  If none exists, it is
 * created, using the given host window to create the buffer, or the current
 * window if that is set to NULL.
 */
PT(Texture) GraphicsStateGuardian::
get_shadow_map(const NodePath &light_np, GraphicsOutputBase *host) {
  PandaNode *node = light_np.node();
  bool is_point = node->is_of_type(PointLight::get_class_type());
  nassertr(node->is_of_type(DirectionalLight::get_class_type()) ||
           node->is_of_type(Spotlight::get_class_type()) ||
           is_point, nullptr);

  LightLensNode *light = (LightLensNode *)node;
  if (light == nullptr || !light->_shadow_caster) {
    // This light does not have a shadow caster.  Return a dummy shadow map
    // that is filled with a depth value of 1.
    if (node->is_of_type(PointLight::get_class_type())) {
      return get_dummy_shadow_map(Texture::TT_cube_map);
    } else {
      return get_dummy_shadow_map(Texture::TT_2d_texture);
    }
  }

  // The light's shadow map should have been created by set_shadow_caster().
  nassertr(light->_shadow_map != nullptr, nullptr);

  // See if we already have a buffer.  If not, create one.
  if (light->_sbuffers.count(this) != 0) {
    // There's already a buffer - use that.
    return light->_shadow_map;
  }

  if (display_cat.is_debug()) {
    display_cat.debug()
      << "Constructing shadow buffer for light '" << light->get_name()
      << "', size=" << light->_sb_size[0] << "x" << light->_sb_size[1]
      << ", sort=" << light->_sb_sort << "\n";
  }

  if (host == nullptr) {
    nassertr(_current_display_region != nullptr, nullptr);
    host = _current_display_region->get_window();
  }
  nassertr(host != nullptr, nullptr);

  // Nope, the light doesn't have a buffer for our GSG. Make one.
  GraphicsOutput *sbuffer = make_shadow_buffer(light, light->_shadow_map,
                                               DCAST(GraphicsOutput, host));

  // Assign display region(s) to the buffer and camera
  if (is_point) {
    for (int i = 0; i < 6; ++i) {
      PT(DisplayRegion) dr = sbuffer->make_mono_display_region(0, 1, 0, 1);
      dr->set_lens_index(i);
      dr->set_target_tex_page(i);
      dr->set_camera(light_np);
      dr->set_clear_depth_active(true);
    }
  } else {
    PT(DisplayRegion) dr = sbuffer->make_mono_display_region(0, 1, 0, 1);
    dr->set_camera(light_np);
    dr->set_clear_depth_active(true);
  }

  light->_sbuffers[this] = sbuffer;
  return light->_shadow_map;
}

/**
 * Returns a dummy shadow map that can be used for a light of the given type
 * that does not cast shadows.
 */
PT(Texture) GraphicsStateGuardian::
get_dummy_shadow_map(Texture::TextureType texture_type) const {
  if (texture_type != Texture::TT_cube_map) {
    static PT(Texture) dummy_2d;
    if (dummy_2d == nullptr) {
      dummy_2d = new Texture("dummy-shadow-2d");
      dummy_2d->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_depth_component);
      dummy_2d->set_clear_color(1);
      if (get_supports_shadow_filter()) {
        // If we have the ARB_shadow extension, enable shadow filtering.
        dummy_2d->set_minfilter(SamplerState::FT_shadow);
        dummy_2d->set_magfilter(SamplerState::FT_shadow);
      } else {
        dummy_2d->set_minfilter(SamplerState::FT_linear);
        dummy_2d->set_magfilter(SamplerState::FT_linear);
      }
    }
    return dummy_2d;
  } else {
    static PT(Texture) dummy_cube;
    if (dummy_cube == nullptr) {
      dummy_cube = new Texture("dummy-shadow-cube");
      dummy_cube->setup_cube_map(1, Texture::T_unsigned_byte, Texture::F_depth_component);
      dummy_cube->set_clear_color(1);
      // Note: cube map shadow filtering doesn't seem to work in Cg.
      // That is why it is currently disabled by default, but it can be
      // overridden in Config.prc for apps that have custom GLSL shaders.
      if (shadow_cube_map_filter && get_supports_shadow_filter()) {
        dummy_cube->set_minfilter(SamplerState::FT_shadow);
        dummy_cube->set_magfilter(SamplerState::FT_shadow);
      } else {
        dummy_cube->set_minfilter(SamplerState::FT_linear);
        dummy_cube->set_magfilter(SamplerState::FT_linear);
      }
    }
    return dummy_cube;
  }
}

/**
 * Creates a depth buffer for shadow mapping.  A derived GSG can override this
 * if it knows that a particular buffer type works best for shadow rendering.
 */
GraphicsOutput *GraphicsStateGuardian::
make_shadow_buffer(LightLensNode *light, Texture *tex, GraphicsOutput *host) {
  bool is_point = light->is_of_type(PointLight::get_class_type());

  // Determine the properties for creating the depth buffer.
  FrameBufferProperties fbp;
  fbp.set_depth_bits(shadow_depth_bits);

  WindowProperties props = WindowProperties::size(light->_sb_size);
  int flags = GraphicsPipe::BF_refuse_window;
  if (is_point) {
    flags |= GraphicsPipe::BF_size_square;
  }

  // Create the buffer.  This is a bit tricky because make_output() can only
  // be called from the app thread, but it won't cause issues as long as the
  // pipe can precertify the buffer, which it can in most cases.
  GraphicsOutput *sbuffer = get_engine()->make_output(get_pipe(),
    light->get_name(), light->_sb_sort, fbp, props, flags, this, host);

  if (sbuffer != nullptr) {
    sbuffer->add_render_texture(tex, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_depth);
  }
  return sbuffer;
}

/**
 * Ensures that an appropriate shader has been generated for the given state.
 * This is stored in the _generated_shader field on the RenderState.
 */
void GraphicsStateGuardian::
ensure_generated_shader(const RenderState *state) {
#ifdef HAVE_CG
  const ShaderAttrib *shader_attrib;
  state->get_attrib_def(shader_attrib);

  if (shader_attrib->auto_shader()) {
    if (_shader_generator == nullptr) {
      if (!_supports_basic_shaders) {
        return;
      }
      _shader_generator = new ShaderGenerator(_shader_caps, _supports_shadow_filter);
    }
    if (state->_generated_shader == nullptr ||
        state->_generated_shader_seq != _generated_shader_seq) {
      GeomVertexAnimationSpec spec;

      // Currently we overload this flag to request vertex animation for the
      // shader generator.
      const ShaderAttrib *sattr;
      state->get_attrib_def(sattr);
      if (sattr->get_flag(ShaderAttrib::F_hardware_skinning)) {
        spec.set_hardware(4, true);
      }

      // Cache the generated ShaderAttrib on the shader state.
      state->_generated_shader = _shader_generator->synthesize_shader(state, spec);
      state->_generated_shader_seq = _generated_shader_seq;
    }
  }
#endif
}

/**
 * Returns true if the GSG implements the extension identified by the given
 * string.  This currently is only implemented by the OpenGL back-end.
 */
bool GraphicsStateGuardian::
has_extension(const string &extension) const {
  return false;
}

/**
 * Returns the vendor of the video card driver
 */
string GraphicsStateGuardian::
get_driver_vendor() {
  return string();
}

/**
 * Returns GL_Renderer
 */
string GraphicsStateGuardian::get_driver_renderer() {
  return string();
}

/**
 * Returns driver version This has an implementation-defined meaning, and may
 * be "" if the particular graphics implementation does not provide a way to
 * query this information.
 */
string GraphicsStateGuardian::
get_driver_version() {
  return string();
}

/**
 * Returns major version of the video driver.  This has an implementation-
 * defined meaning, and may be -1 if the particular graphics implementation
 * does not provide a way to query this information.
 */
int GraphicsStateGuardian::
get_driver_version_major() {
  return -1;
}

/**
 * Returns the minor version of the video driver.  This has an implementation-
 * defined meaning, and may be -1 if the particular graphics implementation
 * does not provide a way to query this information.
 */
int GraphicsStateGuardian::
get_driver_version_minor() {
  return -1;
}

/**
 * Returns the major version of the shader model.
 */
int GraphicsStateGuardian::
get_driver_shader_version_major() {
  return -1;
}

/**
 * Returns the minor version of the shader model.
 */
int GraphicsStateGuardian::
get_driver_shader_version_minor() {
  return -1;
}

std::ostream &
operator << (std::ostream &out, GraphicsStateGuardian::ShaderModel sm) {
  static const char *sm_strings[] = {"none", "1.1", "2.0", "2.x", "3.0", "4.0", "5.0", "5.1"};
  nassertr(sm >= 0 && sm <= GraphicsStateGuardian::SM_51, out);
  out << sm_strings[sm];
  return out;
}
