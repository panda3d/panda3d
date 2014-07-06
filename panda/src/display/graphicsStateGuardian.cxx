// Filename: graphicsStateGuardian.cxx
// Created by:  drose (02eb99)
// Updated by: fperazzi, PandaSE (05May10) (added fetch_ptr_parameter,
//  _max_2d_texture_array_layers, _supports_2d_texture_array,
//  get_supports_cg_profile)
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

#include "graphicsStateGuardian.h"
#include "graphicsEngine.h"
#include "config_display.h"
#include "textureContext.h"
#include "vertexBufferContext.h"
#include "indexBufferContext.h"
#include "renderBuffer.h"
#include "light.h"
#include "planeNode.h"
#include "ambientLight.h"
#include "throw_event.h"
#include "clockObject.h"
#include "pStatTimer.h"
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
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
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

#include <algorithm>
#include <limits.h>

PStatCollector GraphicsStateGuardian::_vertex_buffer_switch_pcollector("Vertex buffer switch:Vertex");
PStatCollector GraphicsStateGuardian::_index_buffer_switch_pcollector("Vertex buffer switch:Index");
PStatCollector GraphicsStateGuardian::_load_vertex_buffer_pcollector("Draw:Transfer data:Vertex buffer");
PStatCollector GraphicsStateGuardian::_load_index_buffer_pcollector("Draw:Transfer data:Index buffer");
PStatCollector GraphicsStateGuardian::_create_vertex_buffer_pcollector("Draw:Transfer data:Create Vertex buffer");
PStatCollector GraphicsStateGuardian::_create_index_buffer_pcollector("Draw:Transfer data:Create Index buffer");
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
PStatCollector GraphicsStateGuardian::_clear_pcollector("Draw:Clear");
PStatCollector GraphicsStateGuardian::_flush_pcollector("Draw:Flush");

PStatCollector GraphicsStateGuardian::_wait_occlusion_pcollector("Wait:Occlusion");


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


PT(TextureStage) GraphicsStateGuardian::_alpha_scale_texture_stage = NULL;

TypeHandle GraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////

GraphicsStateGuardian::
GraphicsStateGuardian(CoordinateSystem internal_coordinate_system,
                      GraphicsEngine *engine, GraphicsPipe *pipe) :
  _internal_coordinate_system(internal_coordinate_system),
  _pipe(pipe),
  _engine(engine)
{
  _coordinate_system = CS_invalid;
  _internal_transform = TransformState::make_identity();

  set_coordinate_system(get_default_coordinate_system());

  _data_reader = (GeomVertexDataPipelineReader *)NULL;
  _current_display_region = (DisplayRegion*)NULL;
  _current_stereo_channel = Lens::SC_mono;
  _current_tex_view_offset = 0;
  _current_lens = (Lens *)NULL;
  _projection_mat = TransformState::make_identity();
  _projection_mat_inv = TransformState::make_identity();

  _needs_reset = true;
  _is_valid = false;
  _current_properties = NULL;
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

  // Initially, we set this to 1 (the default--no multitexturing
  // supported).  A derived GSG may set this differently if it
  // supports multitexturing.
  _max_texture_stages = 1;

  // Also initially, we assume there are no limits on texture sizes,
  // and that 3-d and cube-map textures are not supported.
  _max_texture_dimension = -1;
  _max_3d_texture_dimension = 0;
  _max_2d_texture_array_layers = 0;
  _max_cube_map_dimension = 0;

  // Assume we don't support these fairly advanced texture combiner
  // modes.
  _supports_texture_combine = false;
  _supports_texture_saved_result = false;
  _supports_texture_dot3 = false;

  _supports_3d_texture = false;
  _supports_2d_texture_array = false;
  _supports_cube_map = false;
  _supports_tex_non_pow2 = false;
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

  // Initially, we set this to false; a GSG that knows it has this
  // property should set it to true.
  _copy_texture_inverted = false;

  // Similarly with these capabilities flags.
  _supports_multisample = false;
  _supports_generate_mipmap = false;
  _supports_depth_texture = false;
  _supports_depth_stencil = false;
  _supports_shadow_filter = false;
  _supports_basic_shaders = false;
  _supports_geometry_shaders = false;
  _supports_tessellation_shaders = false;
  _supports_glsl = false;

  _supports_stencil = false;
  _supports_stencil_wrap = false;
  _supports_two_sided_stencil = false;
  _supports_geometry_instancing = false;

  // Assume a maximum of 1 render target in absence of MRT.
  _max_color_targets = 1;

  _supported_geom_rendering = 0;

  // If this is true, then we can apply a color and/or color scale by
  // twiddling the material and/or ambient light (which could mean
  // enabling lighting even without a LightAttrib).
  _color_scale_via_lighting = color_scale_via_lighting;

  // Similarly for applying a texture to achieve uniform alpha
  // scaling.
  _alpha_scale_via_texture = alpha_scale_via_texture;

  // Few GSG's can do this, since it requires touching each vertex as
  // it is rendered.
  _runtime_color_scale = false;

  _stencil_render_states = 0;

  // The default is no shader support.
  _auto_detect_shader_model = SM_00;
  _shader_model = SM_00;

  _gamma = 1.0f;
  _texture_quality_override = Texture::QL_default;

  _shader_generator = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsStateGuardian::
~GraphicsStateGuardian() {
  remove_gsg(this);

  if (_stencil_render_states) {
    delete _stencil_render_states;
    _stencil_render_states = 0;
  }
  
  if (_shader_generator) {
    delete _shader_generator;
    _shader_generator = 0;
  }

  GeomMunger::unregister_mungers_for_gsg(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_engine
//       Access: Published
//  Description: Returns the graphics engine that created this GSG.
//               Since there is normally only one GraphicsEngine
//               object in an application, this is usually the same as
//               the global GraphicsEngine.
////////////////////////////////////////////////////////////////////
GraphicsEngine *GraphicsStateGuardian::
get_engine() const {
  nassertr(_engine != (GraphicsEngine *)NULL, GraphicsEngine::get_global_ptr());
  return _engine;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_multisample
//       Access: Published, Virtual
//  Description: Returns true if this particular GSG supports using
//               the multisample bits to provide antialiasing, and
//               also supports M_multisample and M_multisample_mask
//               transparency modes.  If this is not true for a
//               particular GSG, Panda will map the M_multisample
//               modes to M_binary.
//
//               This method is declared virtual solely so that it can
//               be queried from cullResult.cxx.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
get_supports_multisample() const {
  return _supports_multisample;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supported_geom_rendering
//       Access: Published, Virtual
//  Description: Returns the union of Geom::GeomRendering values that
//               this particular GSG can support directly.  If a Geom
//               needs to be rendered that requires some additional
//               properties, the StandardMunger and/or the
//               CullableObject will convert it as needed.
//
//               This method is declared virtual solely so that it can
//               be queried from cullableObject.cxx.
////////////////////////////////////////////////////////////////////
int GraphicsStateGuardian::
get_supported_geom_rendering() const {
  return _supported_geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_cg_profile
//       Access: Published, Virtual
//  Description: Returns true if this particular GSG supports the 
//               specified Cg Shader Profile.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
get_supports_cg_profile(const string &name) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_coordinate_system
//       Access: Published
//  Description: Changes the coordinate system in effect on this
//               particular gsg.  This is also called the "external"
//               coordinate system, since it is the coordinate system
//               used by the scene graph, external to to GSG.
//
//               Normally, this will be the default coordinate system,
//               but it might be set differently at runtime.  It will
//               automatically be copied from the current lens's
//               coordinate system as each DisplayRegion is rendered.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_coordinate_system(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_internal_coordinate_system
//       Access: Published, Virtual
//  Description: Returns the coordinate system used internally by the
//               GSG.  This may be the same as the external coordinate
//               system reported by get_coordinate_system(), or it may
//               be something different.
//
//               In any case, vertices that have been transformed
//               before being handed to the GSG (that is, vertices
//               with a contents value of C_clip_point) will be
//               expected to be in this coordinate system.
////////////////////////////////////////////////////////////////////
CoordinateSystem GraphicsStateGuardian::
get_internal_coordinate_system() const {
  return _internal_coordinate_system;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_prepared_objects
//       Access: Public, Virtual
//  Description: Returns the set of texture and geom objects that have
//               been prepared with this GSG (and possibly other GSG's
//               that share objects).
////////////////////////////////////////////////////////////////////
PreparedGraphicsObjects *GraphicsStateGuardian::
get_prepared_objects() {
  return _prepared_objects;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_gamma
//       Access: Published, Virtual
//  Description: Set gamma.  Returns true on success.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
set_gamma(PN_stdfloat gamma) {
  _gamma = gamma;  

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_gamma
//       Access: Published
//  Description: Get the current gamma setting.
////////////////////////////////////////////////////////////////////
PN_stdfloat GraphicsStateGuardian::
get_gamma(PN_stdfloat gamma) {
  return _gamma;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::restore_gamma
//       Access: Published, Virtual
//  Description: Restore original gamma setting.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
restore_gamma() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::traverse_prepared_textures
//       Access: Public
//  Description: Calls the indicated function on all
//               currently-prepared textures, or until the callback
//               function returns false.
////////////////////////////////////////////////////////////////////
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

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_flash_texture
//       Access: Published
//  Description: Sets the "flash texture".  This is a debug feature;
//               when enabled, the specified texture will begin
//               flashing in the scene, helping you to find it
//               visually.
//
//               The texture also flashes with a color code: blue for
//               mipmap level 0, yellow for mipmap level 1, and red
//               for mipmap level 2 or higher (even for textures that
//               don't have mipmaps).  This gives you an idea of the
//               choice of the texture size.  If it is blue, the
//               texture is being drawn the proper size or magnified;
//               if it is yellow, it is being minified a little bit;
//               and if it red, it is being minified considerably.  If
//               you see a red texture when you are right in front of
//               it, you should consider reducing the size of the
//               texture to avoid wasting texture memory.
//
//               Not all rendering backends support the flash_texture
//               feature.  Presently, it is only supported by OpenGL.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_flash_texture(Texture *tex) {
  _flash_texture = tex;
}
#endif  // NDEBUG

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::clear_flash_texture
//       Access: Published
//  Description: Resets the "flash texture", so that no textures will
//               flash.  See set_flash_texture().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
clear_flash_texture() {
  _flash_texture = NULL;
}
#endif  // NDEBUG

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_flash_texture
//       Access: Published
//  Description: Returns the current "flash texture", if any, or NULL
//               if none.  See set_flash_texture().
////////////////////////////////////////////////////////////////////
Texture *GraphicsStateGuardian::
get_flash_texture() const {
  return _flash_texture;
}
#endif  // NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_scene
//       Access: Published
//  Description: Sets the SceneSetup object that indicates the initial
//               camera position, etc.  This must be called before
//               traversal begins.  Returns true if the scene is
//               acceptable, false if something's wrong.  This should
//               be called in the draw thread only.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
set_scene(SceneSetup *scene_setup) {
  _scene_setup = scene_setup;
  _current_lens = scene_setup->get_lens();
  if (_current_lens == (Lens *)NULL) {
    return false;
  }

  set_coordinate_system(_current_lens->get_coordinate_system());

  _projection_mat = calc_projection_mat(_current_lens);
  if (_projection_mat == 0) {
    return false;
  }
  _projection_mat_inv = _projection_mat->get_inverse();
  return prepare_lens();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_scene
//       Access: Published, Virtual
//  Description: Returns the current SceneSetup object.
////////////////////////////////////////////////////////////////////
SceneSetup *GraphicsStateGuardian::
get_scene() const {
  return _scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_texture
//       Access: Public, Virtual
//  Description: Creates whatever structures the GSG requires to
//               represent the texture internally, and returns a
//               newly-allocated TextureContext object with this data.
//               It is the responsibility of the calling function to
//               later call release_texture() with this same pointer
//               (which will also delete the pointer).
//
//               This function should not be called directly to
//               prepare a texture.  Instead, call Texture::prepare().
////////////////////////////////////////////////////////////////////
TextureContext *GraphicsStateGuardian::
prepare_texture(Texture *) {
  return (TextureContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::update_texture
//       Access: Public, Virtual
//  Description: Ensures that the current Texture data is refreshed
//               onto the GSG.  This means updating the texture
//               properties and/or re-uploading the texture image, if
//               necessary.  This should only be called within the
//               draw thread.
//
//               If force is true, this function will not return until
//               the texture has been fully uploaded.  If force is
//               false, the function may choose to upload a simple
//               version of the texture instead, if the texture is not
//               fully resident (and if get_incomplete_render() is
//               true).
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
update_texture(TextureContext *, bool) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_texture
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_texture(), including deleting the
//               TextureContext itself, if it is non-NULL.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
release_texture(TextureContext *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::extract_texture_data
//       Access: Public, Virtual
//  Description: This method should only be called by the
//               GraphicsEngine.  Do not call it directly; call
//               GraphicsEngine::extract_texture_data() instead.
//
//               This method will be called in the draw thread to
//               download the texture memory's image into its
//               ram_image value.  It returns true on success, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
extract_texture_data(Texture *) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_geom
//       Access: Public, Virtual
//  Description: Prepares the indicated Geom for retained-mode
//               rendering, by creating whatever structures are
//               necessary in the GSG (for instance, vertex buffers).
//               Returns the newly-allocated GeomContext that can be
//               used to render the geom.
////////////////////////////////////////////////////////////////////
GeomContext *GraphicsStateGuardian::
prepare_geom(Geom *) {
  return (GeomContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_geom
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_geom(), including deleting the GeomContext
//               itself, if it is non-NULL.
//
//               This function should not be called directly to
//               prepare a Geom.  Instead, call Geom::prepare().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
release_geom(GeomContext *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_shader
//       Access: Public, Virtual
//  Description: Compile a vertex/fragment shader body.
////////////////////////////////////////////////////////////////////
ShaderContext *GraphicsStateGuardian::
prepare_shader(Shader *shader) {
  return (ShaderContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_shader
//       Access: Public, Virtual
//  Description: Releases the resources allocated by prepare_shader
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
release_shader(ShaderContext *sc) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_vertex_buffer
//       Access: Public, Virtual
//  Description: Prepares the indicated buffer for retained-mode
//               rendering.
////////////////////////////////////////////////////////////////////
VertexBufferContext *GraphicsStateGuardian::
prepare_vertex_buffer(GeomVertexArrayData *) {
  return (VertexBufferContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_vertex_buffer
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_data(), including deleting the
//               VertexBufferContext itself, if necessary.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
release_vertex_buffer(VertexBufferContext *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_index_buffer
//       Access: Public, Virtual
//  Description: Prepares the indicated buffer for retained-mode
//               rendering.
////////////////////////////////////////////////////////////////////
IndexBufferContext *GraphicsStateGuardian::
prepare_index_buffer(GeomPrimitive *) {
  return (IndexBufferContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_index_buffer
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_data(), including deleting the
//               IndexBufferContext itself, if necessary.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
release_index_buffer(IndexBufferContext *) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_occlusion_query
//       Access: Public, Virtual
//  Description: Returns true if this GSG supports an occlusion query.
//               If this is true, then begin_occlusion_query() and
//               end_occlusion_query() may be called to bracket a
//               sequence of draw_triangles() (or whatever) calls to
//               measure pixels that pass the depth test.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
get_supports_occlusion_query() const {
  return _supports_occlusion_query;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_occlusion_query
//       Access: Public, Virtual
//  Description: Begins a new occlusion query.  After this call, you
//               may call begin_draw_primitives() and
//               draw_triangles()/draw_whatever() repeatedly.
//               Eventually, you should call end_occlusion_query()
//               before the end of the frame; that will return a new
//               OcclusionQueryContext object that will tell you how
//               many pixels represented by the bracketed geometry
//               passed the depth test.
//
//               It is not valid to call begin_occlusion_query()
//               between another begin_occlusion_query()
//               .. end_occlusion_query() sequence.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
begin_occlusion_query() {
  nassertv(_current_occlusion_query == (OcclusionQueryContext *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_occlusion_query
//       Access: Public, Virtual
//  Description: Ends a previous call to begin_occlusion_query().
//               This call returns the OcclusionQueryContext object
//               that will (eventually) report the number of pixels
//               that passed the depth test between the call to
//               begin_occlusion_query() and end_occlusion_query().
////////////////////////////////////////////////////////////////////
PT(OcclusionQueryContext) GraphicsStateGuardian::
end_occlusion_query() {
  nassertr(_current_occlusion_query != (OcclusionQueryContext *)NULL, NULL);
  PT(OcclusionQueryContext) result = _current_occlusion_query;
  _current_occlusion_query = NULL;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_geom_munger
//       Access: Public, Virtual
//  Description: Looks up or creates a GeomMunger object to munge
//               vertices appropriate to this GSG for the indicated
//               state.
////////////////////////////////////////////////////////////////////
PT(GeomMunger) GraphicsStateGuardian::
get_geom_munger(const RenderState *state, Thread *current_thread) {
  // We can cast the RenderState to a non-const object because we are
  // only updating a cache within the RenderState, not really changing
  // any of its properties.
  RenderState *nc_state = ((RenderState *)state);

  // Before we even look up the map, see if the _last_mi value points
  // to this GSG.  This is likely because we tend to visit the same
  // state multiple times during a frame.  Also, this might well be
  // the only GSG in the world anyway.
  if (!nc_state->_mungers.empty()) {
    RenderState::Mungers::const_iterator mi = nc_state->_last_mi;
    if (!(*mi).first.was_deleted() && (*mi).first == this) {
      if ((*mi).second->is_registered()) {
        return (*mi).second;
      }
    }
  }

  // Nope, we have to look it up in the map.
  RenderState::Mungers::iterator mi = nc_state->_mungers.find(this);
  if (mi != nc_state->_mungers.end() && !(*mi).first.was_deleted()) {
    if ((*mi).second->is_registered()) {
      nc_state->_last_mi = mi;
      return (*mi).second;
    }
    // This GeomMunger is no longer registered.  Remove it from the
    // map.
    nc_state->_mungers.erase(mi);
  }

  // Nothing in the map; create a new entry.
  PT(GeomMunger) munger = make_geom_munger(nc_state, current_thread);
  nassertr(munger != (GeomMunger *)NULL && munger->is_registered(), munger);

  mi = nc_state->_mungers.insert(RenderState::Mungers::value_type(this, munger)).first;
  nc_state->_last_mi = mi;

  return munger;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::make_geom_munger
//       Access: Public, Virtual
//  Description: Creates a new GeomMunger object to munge vertices
//               appropriate to this GSG for the indicated state.
////////////////////////////////////////////////////////////////////
PT(GeomMunger) GraphicsStateGuardian::
make_geom_munger(const RenderState *state, Thread *current_thread) {
  // The default implementation returns no munger at all, but
  // presumably, every kind of GSG needs some special munging action,
  // so real GSG's will override this to return something more
  // useful.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::compute_distance_to
//       Access: Public, Virtual
//  Description: This function may only be called during a render
//               traversal; it will compute the distance to the
//               indicated point, assumed to be in eye coordinates,
//               from the camera plane.
////////////////////////////////////////////////////////////////////
PN_stdfloat GraphicsStateGuardian::
compute_distance_to(const LPoint3 &point) const {
  switch (_coordinate_system) {
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
      << (int)_coordinate_system << "\n";
    return 0.0f;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::fetch_specified_value
//       Access: Public
//  Description: The gsg contains a large number of useful matrices:
//
//                  * the world transform,
//                  * the modelview matrix,
//                  * the cs_transform,
//                  * etc, etc.
//
//               A shader can request any of these values, and
//               furthermore, it can request that various compositions,
//               inverses, and transposes be performed.  The
//               ShaderMatSpec is a data structure indicating what
//               datum is desired and what conversions to perform.
//               This routine, fetch_specified_value, is responsible for
//               doing the actual retrieval and conversions.
//
//               Some values, like the following, aren't matrices:
//
//                  * window size
//                  * texture coordinates of card center
//
//               This routine can fetch these values as well, by
//               shoehorning them into a matrix.  In this way, we avoid
//               the need for a separate routine to fetch these values.
//
//               The "altered" bits indicate what parts of the
//               state_and_transform have changed since the last 
//               time this particular ShaderMatSpec was evaluated.
//               This may allow data to be cached and not reevaluated.
//
////////////////////////////////////////////////////////////////////
const LMatrix4 *GraphicsStateGuardian::
fetch_specified_value(Shader::ShaderMatSpec &spec, int altered) {
  LVecBase3 v;
  
  if (altered & spec._dep[0]) {
    const LMatrix4 *t = fetch_specified_part(spec._part[0], spec._arg[0], spec._cache[0]);
    if (t != &spec._cache[0]) {
      spec._cache[0] = *t;
    }
  }
  if (altered & spec._dep[1]) {
    const LMatrix4 *t = fetch_specified_part(spec._part[1], spec._arg[1], spec._cache[1]);
    if (t != &spec._cache[1]) {
      spec._cache[1] = *t;
    }
  }
  
  switch(spec._func) {
  case Shader::SMF_compose:
    spec._value.multiply(spec._cache[0], spec._cache[1]);
    return &spec._value;
  case Shader::SMF_transform_dlight:
    spec._value = spec._cache[0];
    v = spec._cache[1].xform_vec(spec._cache[0].get_row3(2));
    v.normalize();
    spec._value.set_row(2, v);
    v = spec._cache[1].xform_vec(spec._cache[0].get_row3(3));
    v.normalize();
    spec._value.set_row(3, v);
    return &spec._value;
  case Shader::SMF_transform_plight:
    spec._value = spec._cache[0];
    spec._value.set_row(2, spec._cache[1].xform_point(spec._cache[0].get_row3(2)));
    return &spec._value;
  case Shader::SMF_transform_slight:
    spec._value = spec._cache[0];
    spec._value.set_row(2, spec._cache[1].xform_point(spec._cache[0].get_row3(2)));
    v = spec._cache[1].xform_vec(spec._cache[0].get_row3(3));
    v.normalize();
    spec._value.set_row(3, v);
    return &spec._value;
  case Shader::SMF_first:
    return &spec._cache[0];
  default:
    // should never get here
    spec._value = LMatrix4::ident_mat();
    return &spec._value;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::fetch_specified_part
//       Access: Public
//  Description: See fetch_specified_value
////////////////////////////////////////////////////////////////////
const LMatrix4 *GraphicsStateGuardian::
fetch_specified_part(Shader::ShaderMatInput part, InternalName *name, LMatrix4 &t) {
  switch(part) {
  case Shader::SMO_identity: {
    return &LMatrix4::ident_mat();
  }
  case Shader::SMO_window_size: {
    t = LMatrix4::translate_mat(_current_display_region->get_pixel_width(),
                                 _current_display_region->get_pixel_height(),
                                 0.0);
    return &t;
  }
  case Shader::SMO_pixel_size: {
    t = LMatrix4::translate_mat(_current_display_region->get_pixel_width(),
                                 _current_display_region->get_pixel_height(),
                                 0.0);
    return &t;
  }
  case Shader::SMO_frame_time: {
    PN_stdfloat time = ClockObject::get_global_clock()->get_frame_time();
    t = LMatrix4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, time, time, time, time);
    return &t;
  }
  case Shader::SMO_frame_delta: {
    PN_stdfloat dt = ClockObject::get_global_clock()->get_dt();
    t = LMatrix4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, dt, dt, dt, dt);
    return &t;
  }
  case Shader::SMO_texpad_x: {
    Texture *tex = _target_shader->get_shader_input_texture(name);
    nassertr(tex != 0, &LMatrix4::zeros_mat());
    int sx = tex->get_x_size() - tex->get_pad_x_size();
    int sy = tex->get_y_size() - tex->get_pad_y_size();
    int sz = tex->get_z_size() - tex->get_pad_z_size();
    double cx = (sx * 0.5) / tex->get_x_size();
    double cy = (sy * 0.5) / tex->get_y_size();
    double cz = (sz * 0.5) / tex->get_z_size();
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,cx,cy,cz,0);
    return &t;
  }
  case Shader::SMO_texpix_x: {
    Texture *tex = _target_shader->get_shader_input_texture(name);
    nassertr(tex != 0, &LMatrix4::zeros_mat());
    double px = 1.0 / tex->get_x_size();
    double py = 1.0 / tex->get_y_size();
    double pz = 1.0 / tex->get_z_size();
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,px,py,pz,0);
    return &t;
  }
  case Shader::SMO_attr_material: {
    const MaterialAttrib *target_material = DCAST(MaterialAttrib, _target_rs->get_attrib_def(MaterialAttrib::get_class_slot()));
    // Material matrix contains AMBIENT, DIFFUSE, EMISSION, SPECULAR+SHININESS
    if (target_material->is_off()) {
      t = LMatrix4(1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0);
      return &t;
    }
    Material *m = target_material->get_material();
    LVecBase4 const &amb = m->get_ambient();
    LVecBase4 const &dif = m->get_diffuse();
    LVecBase4 const &emm = m->get_emission();
    LVecBase4 spc = m->get_specular();
    spc[3] = m->get_shininess();
    t = LMatrix4(amb[0],amb[1],amb[2],amb[3],
                  dif[0],dif[1],dif[2],dif[3],
                  emm[0],emm[1],emm[2],emm[3],
                  spc[0],spc[1],spc[2],spc[3]);
    return &t;
  }
  case Shader::SMO_attr_color: {
    const ColorAttrib *target_color = DCAST(ColorAttrib, _target_rs->get_attrib_def(ColorAttrib::get_class_slot()));
    if (target_color->get_color_type() != ColorAttrib::T_flat) {
      return &LMatrix4::ones_mat();
    }
    LVecBase4 c = target_color->get_color();
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,c[0],c[1],c[2],c[3]);
    return &t;
  }
  case Shader::SMO_attr_colorscale: {
    const ColorScaleAttrib *target_color = DCAST(ColorScaleAttrib, _target_rs->get_attrib_def(ColorScaleAttrib::get_class_slot()));
    if (target_color->is_identity()) {
      return &LMatrix4::ones_mat();
    }
    LVecBase4 cs = target_color->get_scale();
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,cs[0],cs[1],cs[2],cs[3]);
    return &t;
  }
  case Shader::SMO_attr_fog: {
    const FogAttrib *target_fog = DCAST(FogAttrib, _target_rs->get_attrib_def(FogAttrib::get_class_slot()));
    Fog *fog = target_fog->get_fog();
    if (fog == (Fog*) NULL) {
      return &LMatrix4::ones_mat();
    }
    PN_stdfloat start, end;
    fog->get_linear_range(start, end);
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,fog->get_exp_density(),start,end,1.0f/(end-start));
    return &t;
  }
  case Shader::SMO_attr_fogcolor: {
    const FogAttrib *target_fog = DCAST(FogAttrib, _target_rs->get_attrib_def(FogAttrib::get_class_slot()));
    Fog *fog = target_fog->get_fog();
    if (fog == (Fog*) NULL) {
      return &LMatrix4::ones_mat();
    }
    LVecBase4 c = fog->get_color();
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,c[0],c[1],c[2],c[3]);
    return &t;
  }
  case Shader::SMO_alight_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::zeros_mat());
    AmbientLight *lt;
    DCAST_INTO_R(lt, np.node(), &LMatrix4::zeros_mat());
    LColor const &c = lt->get_color();
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,c[0],c[1],c[2],c[3]);
    return &t;
  }
  case Shader::SMO_satten_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::ones_mat());
    Spotlight *lt;
    DCAST_INTO_R(lt, np.node(), &LMatrix4::ones_mat());
    LVecBase3 const &a = lt->get_attenuation();
    PN_stdfloat x = lt->get_exponent();
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,a[0],a[1],a[2],x);
    return &t;
  }
  case Shader::SMO_dlight_x: {
    // The dlight matrix contains COLOR, SPECULAR, DIRECTION, PSEUDOHALFANGLE
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::zeros_mat());
    DirectionalLight *lt;
    DCAST_INTO_R(lt, np.node(), &LMatrix4::zeros_mat());
    LColor const &c = lt->get_color();
    LColor const &s = lt->get_specular_color();
    t = np.get_net_transform()->get_mat() *
      get_scene()->get_world_transform()->get_mat();
    LVecBase3 d = -(t.xform_vec(lt->get_direction()));
    d.normalize();
    LVecBase3 h = d + LVecBase3(0,-1,0);
    h.normalize();
    t = LMatrix4(c[0],c[1],c[2],c[3],s[0],s[1],s[2],c[3],d[0],d[1],d[2],0,h[0],h[1],h[2],0);
    return &t;
  }
  case Shader::SMO_plight_x: {
    // The plight matrix contains COLOR, SPECULAR, POINT, ATTENUATION
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::ones_mat());
    PointLight *lt;
    DCAST_INTO_R(lt, np.node(), &LMatrix4::zeros_mat());
    LColor const &c = lt->get_color();
    LColor const &s = lt->get_specular_color();
    t = np.get_net_transform()->get_mat() *
      get_scene()->get_world_transform()->get_mat();
    LVecBase3 p = (t.xform_point(lt->get_point()));
    LVecBase3 a = lt->get_attenuation();
    t = LMatrix4(c[0],c[1],c[2],c[3],s[0],s[1],s[2],s[3],p[0],p[1],p[2],0,a[0],a[1],a[2],0);
    return &t;
  }
  case Shader::SMO_slight_x: {
    // The slight matrix contains COLOR, SPECULAR, POINT, DIRECTION
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::zeros_mat());
    Spotlight *lt;
    DCAST_INTO_R(lt, np.node(), &LMatrix4::zeros_mat());
    Lens *lens = lt->get_lens();
    nassertr(lens != (Lens *)NULL, &LMatrix4::zeros_mat());
    LColor const &c = lt->get_color();
    LColor const &s = lt->get_specular_color();
    PN_stdfloat cutoff = ccos(deg_2_rad(lens->get_hfov() * 0.5f));
    t = np.get_net_transform()->get_mat() *
      get_scene()->get_world_transform()->get_mat();
    LVecBase3 p = t.xform_point(lens->get_nodal_point());
    LVecBase3 d = -(t.xform_vec(lens->get_view_vector()));
    t = LMatrix4(c[0],c[1],c[2],c[3],s[0],s[1],s[2],s[3],p[0],p[1],p[2],0,d[0],d[1],d[2],cutoff);
    return &t;
  }
  case Shader::SMO_texmat_x: {
    const TexMatrixAttrib *tma = DCAST(TexMatrixAttrib, _target_rs->get_attrib_def(TexMatrixAttrib::get_class_slot()));
    const TextureAttrib *ta = DCAST(TextureAttrib, _target_rs->get_attrib_def(TextureAttrib::get_class_slot()));
    int stagenr = atoi(name->get_name().c_str());
    if (stagenr >= ta->get_num_on_stages()) {
      return &LMatrix4::ident_mat();
    }
    return &tma->get_mat(ta->get_on_stage(stagenr));
  }
  case Shader::SMO_plane_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::zeros_mat());
    nassertr(np.node()->is_of_type(PlaneNode::get_class_type()), &LMatrix4::zeros_mat());
    LPlane p = DCAST(PlaneNode, np.node())->get_plane();
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,p[0],p[1],p[2],p[3]);
    return &t;
  }
  case Shader::SMO_clipplane_x: {
    const ClipPlaneAttrib *cpa = DCAST(ClipPlaneAttrib, _target_rs->get_attrib_def(ClipPlaneAttrib::get_class_slot()));
    int planenr = atoi(name->get_name().c_str());
    if (planenr >= cpa->get_num_on_planes()) {
      return &LMatrix4::zeros_mat();
    }
    const NodePath &np = cpa->get_on_plane(planenr);
    nassertr(!np.is_empty(), &LMatrix4::zeros_mat());
    nassertr(np.node()->is_of_type(PlaneNode::get_class_type()), &LMatrix4::zeros_mat());
    LPlane p (DCAST(PlaneNode, np.node())->get_plane());
    p.xform(np.get_net_transform()->get_mat()); // World-space
    t = LMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,p[0],p[1],p[2],p[3]);
    return &t;
  }
  case Shader::SMO_mat_constant_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::ident_mat());
    return &(np.node()->get_transform()->get_mat());
  }
  case Shader::SMO_vec_constant_x: {
    const LVecBase4 &input = _target_shader->get_shader_input_vector(name);
    const PN_stdfloat *data = input.get_data();
    t = LMatrix4(data[0],data[1],data[2],data[3],
                 data[0],data[1],data[2],data[3],
                 data[0],data[1],data[2],data[3],
                 data[0],data[1],data[2],data[3]);
    return &t;
  }
  case Shader::SMO_world_to_view: {
    return &(get_scene()->get_world_transform()->get_mat());
    break;
  }
  case Shader::SMO_view_to_world: {
    return &(get_scene()->get_camera_transform()->get_mat());
  }
  case Shader::SMO_model_to_view: {
    return &(get_external_transform()->get_mat());
  }
  case Shader::SMO_view_to_model: {
    // DANGER: SLOW AND NOT CACHEABLE!
    t.invert_from(get_external_transform()->get_mat());
    return &t;
  }
  case Shader::SMO_apiview_to_view: {
    return &(_inv_cs_transform->get_mat());
  }
  case Shader::SMO_view_to_apiview: {
    return &(_cs_transform->get_mat());
  }
  case Shader::SMO_clip_to_view: {
    if (_current_lens->get_coordinate_system() == _coordinate_system) {
      return &(_current_lens->get_projection_mat_inv(_current_stereo_channel));
    } else {
      t = _current_lens->get_projection_mat_inv(_current_stereo_channel) *
        LMatrix4::convert_mat(_current_lens->get_coordinate_system(), _coordinate_system);
      return &t;
    }
  }
  case Shader::SMO_view_to_clip: {
    if (_current_lens->get_coordinate_system() == _coordinate_system) {
      return &(_current_lens->get_projection_mat(_current_stereo_channel));
    } else {
      t = LMatrix4::convert_mat(_coordinate_system, _current_lens->get_coordinate_system()) *
        _current_lens->get_projection_mat(_current_stereo_channel);
      return &t;
    }
  }
  case Shader::SMO_apiclip_to_view: {
    t = _projection_mat_inv->get_mat() * _inv_cs_transform->get_mat();
    return &t;
  }
  case Shader::SMO_view_to_apiclip: {
    t = _cs_transform->get_mat() * _projection_mat->get_mat();
    return &t;
  }
  case Shader::SMO_view_x_to_view: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::ident_mat());
    t = np.get_net_transform()->get_mat() *
      get_scene()->get_world_transform()->get_mat();
    return &t;
  }
  case Shader::SMO_view_to_view_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::ident_mat());
    t = get_scene()->get_camera_transform()->get_mat() *
      invert(np.get_net_transform()->get_mat());
    return &t;
  }
  case Shader::SMO_apiview_x_to_view: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::ident_mat());
    t = LMatrix4::convert_mat(_internal_coordinate_system, _coordinate_system) *
      np.get_net_transform()->get_mat() *
      get_scene()->get_world_transform()->get_mat();
    return &t;
  }
  case Shader::SMO_view_to_apiview_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::ident_mat());
    t = (get_scene()->get_camera_transform()->get_mat() *
         invert(np.get_net_transform()->get_mat()) *
         LMatrix4::convert_mat(_coordinate_system, _internal_coordinate_system));
    return &t;
  }
  case Shader::SMO_clip_x_to_view: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::ident_mat());
    nassertr(np.node()->is_of_type(LensNode::get_class_type()), &LMatrix4::ident_mat());
    Lens *lens = DCAST(LensNode, np.node())->get_lens();
    t = lens->get_projection_mat_inv(_current_stereo_channel) *
      LMatrix4::convert_mat(lens->get_coordinate_system(), _coordinate_system) *
      np.get_net_transform()->get_mat() *
      get_scene()->get_world_transform()->get_mat();
    return &t;
  }
  case Shader::SMO_view_to_clip_x: {
    const NodePath &np = _target_shader->get_shader_input_nodepath(name);
    nassertr(!np.is_empty(), &LMatrix4::ident_mat());
    nassertr(np.node()->is_of_type(LensNode::get_class_type()), &LMatrix4::ident_mat());
    Lens *lens = DCAST(LensNode, np.node())->get_lens();
    t = get_scene()->get_camera_transform()->get_mat() *
      invert(np.get_net_transform()->get_mat()) *
      LMatrix4::convert_mat(_coordinate_system, lens->get_coordinate_system()) *
      lens->get_projection_mat(_current_stereo_channel);
    return &t;
  }
  case Shader::SMO_apiclip_x_to_view: {
    // NOT IMPLEMENTED
    return &LMatrix4::ident_mat();
  }
  case Shader::SMO_view_to_apiclip_x: {
    // NOT IMPLEMENTED
    return &LMatrix4::ident_mat();
  }
  default:
    // should never get here
    return &LMatrix4::ident_mat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::fetch_ptr_parameter
//       Access: Public
//  Description: Return a pointer to struct ShaderPtrData
////////////////////////////////////////////////////////////////////
const Shader::ShaderPtrData *GraphicsStateGuardian::
fetch_ptr_parameter(const Shader::ShaderPtrSpec& spec) {
  return (_target_shader->get_shader_input_ptr(spec._arg));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_display_region
//       Access: Public, Virtual
//  Description: Makes the specified DisplayRegion current.  All
//               future drawing and clear operations will be
//               constrained within the given DisplayRegion.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::clear_before_callback
//       Access: Public, Virtual
//  Description: Resets any non-standard graphics state that might
//               give a callback apoplexy.  Some drivers require that
//               the graphics state be restored to neutral before
//               performing certain operations.  In OpenGL, for
//               instance, this closes any open vertex buffers.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
clear_before_callback() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::clear_state_and_transform
//       Access: Public, Virtual
//  Description: Forgets the current graphics state and current
//               transform, so that the next call to
//               set_state_and_transform() will have to reload
//               everything.  This is a good thing to call when you
//               are no longer sure what the graphics state is.  This
//               should only be called from the draw thread.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
clear_state_and_transform() {
  // Re-issue the modelview and projection transforms.
  reissue_transforms();

  // Now clear the state flags to unknown.
  _state_rs = RenderState::make_empty();
  _state_mask.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::remove_window
//       Access: Public, Virtual
//  Description: This is simply a transparent call to
//               GraphicsEngine::remove_window().  It exists primary
//               to support removing a window from that compiles
//               before the display module, and therefore has no
//               knowledge of a GraphicsEngine object.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
remove_window(GraphicsOutputBase *window) {
  nassertv(_engine != (GraphicsEngine *)NULL);
  GraphicsOutput *win;
  DCAST_INTO_V(win, window);
  _engine->remove_window(win);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prepare_lens
//       Access: Public, Virtual
//  Description: Makes the current lens (whichever lens was most
//               recently specified with set_scene()) active, so
//               that it will transform future rendered geometry.
//               Normally this is only called from the draw process,
//               and usually it is called by set_scene().
//
//               The return value is true if the lens is acceptable,
//               false if it is not.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
prepare_lens() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::calc_projection_mat
//       Access: Public, Virtual
//  Description: Given a lens, this function calculates the appropriate
//               projection matrix for this gsg.  The result depends
//               on the peculiarities of the rendering API.
////////////////////////////////////////////////////////////////////
CPT(TransformState) GraphicsStateGuardian::
calc_projection_mat(const Lens *lens) {
  if (lens == (Lens *)NULL) {
    return NULL;
  }

  if (!lens->is_linear()) {
    return NULL;
  }

  return TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_frame
//       Access: Public, Virtual
//  Description: Called before each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup before
//               beginning the frame.
//
//               The return value is true if successful (in which case
//               the frame will be drawn and end_frame() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_frame() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
begin_frame(Thread *current_thread) {
  _prepared_objects->begin_frame(this, current_thread);

  // We should reset the state to the default at the beginning of
  // every frame.  Although this will incur additional overhead,
  // particularly in a simple scene, it helps ensure that states that
  // have changed properties since last time without changing
  // attribute pointers--like textures, lighting, or fog--will still
  // be accurately updated.
  _state_rs = RenderState::make_empty();
  _state_mask.clear();

  return !_needs_reset;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_scene
//       Access: Published, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the beginning of drawing commands for a "scene"
//               (usually a particular DisplayRegion) within a frame.
//               All 3-D drawing commands, except the clear operation,
//               must be enclosed within begin_scene() .. end_scene().
//               This must be called in the draw thread.
//
//               The return value is true if successful (in which case
//               the scene will be drawn and end_scene() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_scene() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
begin_scene() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_scene
//       Access: Published, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the end of drawing commands for a "scene" (usually a
//               particular DisplayRegion) within a frame.  All 3-D
//               drawing commands, except the clear operation, must be
//               enclosed within begin_scene() .. end_scene().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
end_scene() {
  // We should clear this pointer now, so that we don't keep unneeded
  // reference counts dangling.  We keep around a "null" scene setup
  // object instead of using a null pointer to avoid special-case code
  // in set_state_and_transform.
  _scene_setup = _scene_null;

  // Undo any lighting we had enabled last scene, to force the lights
  // to be reissued, in case their parameters or positions have
  // changed between scenes.
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_frame
//       Access: Public, Virtual
//  Description: Called after each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup after
//               rendering the frame, and before the window flips.
////////////////////////////////////////////////////////////////////
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

  _state_pcollector.flush_level();
  _texture_state_pcollector.flush_level();
  _transform_state_pcollector.flush_level();
  _draw_primitive_pcollector.flush_level();

  // Evict any textures and/or vbuffers that exceed our texture memory.
  _prepared_objects->_graphics_memory_lru.begin_epoch();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::depth_offset_decals
//       Access: Public, Virtual
//  Description: Returns true if this GSG can implement decals using a
//               DepthOffsetAttrib, or false if that is unreliable
//               and the three-step rendering process should be used
//               instead.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
depth_offset_decals() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_decal_base_first
//       Access: Public, Virtual
//  Description: Called during draw to begin a three-step rendering
//               phase to draw decals.  The first step,
//               begin_decal_base_first(), is called prior to drawing the
//               base geometry.  It should set up whatever internal
//               state is appropriate, as well as returning a
//               RenderState object that should be applied to the base
//               geometry for rendering.
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
begin_decal_base_first() {
  // Turn off writing the depth buffer to render the base geometry.
  static CPT(RenderState) decal_base_first;
  if (decal_base_first == (const RenderState *)NULL) {
    decal_base_first = RenderState::make
      (DepthWriteAttrib::make(DepthWriteAttrib::M_off),
       RenderState::get_max_priority());
  }
  return decal_base_first;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_decal_nested
//       Access: Public, Virtual
//  Description: Called during draw to begin a three-step rendering
//               phase to draw decals.  The second step,
//               begin_decal_nested(), is called after drawing the
//               base geometry and prior to drawing any of the nested
//               decal geometry that is to be applied to the base
//               geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
begin_decal_nested() {
  // We should keep the depth buffer off during this operation, so
  // that decals on decals will render properly.
  static CPT(RenderState) decal_nested;
  if (decal_nested == (const RenderState *)NULL) {
    decal_nested = RenderState::make
      (DepthWriteAttrib::make(DepthWriteAttrib::M_off),
       RenderState::get_max_priority());
  }
  return decal_nested;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_decal_base_second
//       Access: Public, Virtual
//  Description: Called during draw to begin a three-step rendering
//               phase to draw decals.  The third step,
//               begin_decal_base_second(), is called after drawing the
//               base geometry and the nested decal geometry, and
//               prior to drawing the base geometry one more time (if
//               needed).
//
//               It should return a RenderState object appropriate for
//               rendering the base geometry the second time, or NULL
//               if it is not necessary to re-render the base
//               geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
begin_decal_base_second() {
  // Now let the depth buffer go back on, but turn off writing the
  // color buffer to render the base geometry after the second pass.
  // Also, turn off texturing since there's no need for it now.
  static CPT(RenderState) decal_base_second;
  if (decal_base_second == (const RenderState *)NULL) {
    decal_base_second = RenderState::make
      (ColorWriteAttrib::make(ColorWriteAttrib::C_off),
       // On reflection, we need to leave texturing on so the alpha
       // test mechanism can work (if it is enabled, e.g. we are
       // rendering an object with M_dual transparency).
       //       TextureAttrib::make_off(),
       RenderState::get_max_priority());
  }
  return decal_base_second;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::finish_decal
//       Access: Public, Virtual
//  Description: Called during draw to clean up after decals are
//               finished.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
finish_decal() {
  // No need to do anything special here.
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_draw_primitives()
//       Access: Public, Virtual
//  Description: Called before a sequence of draw_primitive()
//               functions are called, this should prepare the vertex
//               data for rendering.  It returns true if the vertices
//               are ok, false to abort this group of primitives.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
begin_draw_primitives(const GeomPipelineReader *geom_reader,
                      const GeomMunger *munger,
                      const GeomVertexDataPipelineReader *data_reader,
                      bool force) {
  _munger = munger;
  _data_reader = data_reader;
  return _data_reader->has_vertex();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_triangles
//       Access: Public, Virtual
//  Description: Draws a series of disconnected triangles.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
draw_triangles(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_tristrips
//       Access: Public, Virtual
//  Description: Draws a series of triangle strips.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
draw_tristrips(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_trifans
//       Access: Public, Virtual
//  Description: Draws a series of triangle fans.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
draw_trifans(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_patches
//       Access: Public, Virtual
//  Description: Draws a series of "patches", which can only be
//               processed by a tessellation shader.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
draw_patches(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_lines
//       Access: Public, Virtual
//  Description: Draws a series of disconnected line segments.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
draw_lines(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_linestrips
//       Access: Public, Virtual
//  Description: Draws a series of line strips.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
draw_linestrips(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::draw_points
//       Access: Public, Virtual
//  Description: Draws a series of disconnected points.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
draw_points(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_draw_primitives()
//       Access: Public, Virtual
//  Description: Called after a sequence of draw_primitive()
//               functions are called, this should do whatever cleanup
//               is appropriate.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
end_draw_primitives() {
  _munger = NULL;
  _data_reader = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
reset() {
  _needs_reset = false;
  _is_valid = false;

  _state_rs = RenderState::make_empty();
  _target_rs = NULL;
  _state_mask.clear();
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

  if (_stencil_render_states) {
    delete _stencil_render_states;
    _stencil_render_states = 0;
  }
  _stencil_render_states = new StencilRenderStates (this);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_state_and_transform
//       Access: Public
//  Description: Simultaneously resets the render state and the
//               transform state.
//
//               This transform specified is the "internal" net
//               transform, already converted into the GSG's internal
//               coordinate space by composing it to
//               get_cs_transform().  (Previously, this used to be the
//               "external" net transform, with the assumption that
//               that GSG would convert it internally, but that is no
//               longer the case.)
//
//               Special case: if (state==NULL), then the target
//               state is already stored in _target.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_state_and_transform(const RenderState *state,
                        const TransformState *trans) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::clear
//       Access: Public
//  Description: Clears the framebuffer within the current
//               DisplayRegion, according to the flags indicated by
//               the given DrawableRegion object.
//
//               This does not set the DisplayRegion first.  You
//               should call prepare_display_region() to specify the
//               region you wish the clear operation to apply to.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
clear(DrawableRegion *clearable) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_render_buffer
//       Access: Public
//  Description: Returns a RenderBuffer object suitable for operating
//               on the requested set of buffers.  buffer_type is the
//               union of all the desired RenderBuffer::Type values.
////////////////////////////////////////////////////////////////////
RenderBuffer GraphicsStateGuardian::
get_render_buffer(int buffer_type, const FrameBufferProperties &prop) {
  return RenderBuffer(this, buffer_type & prop.get_buffer_mask() & _stereo_buffer_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_cs_transform_for
//       Access: Public, Virtual
//  Description: Returns what the cs_transform would be set to after a
//               call to set_coordinate_system(cs).  This is another
//               way of saying the cs_transform when rendering the
//               scene for a camera with the indicated coordinate
//               system.
////////////////////////////////////////////////////////////////////
CPT(TransformState) GraphicsStateGuardian::
get_cs_transform_for(CoordinateSystem cs) const {
  if (_internal_coordinate_system == CS_default ||
      _internal_coordinate_system == cs) {
    return TransformState::make_identity();

  } else {
    return TransformState::make_mat
      (LMatrix4::convert_mat(cs, _internal_coordinate_system));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_cs_transform
//       Access: Public, Virtual
//  Description: Returns a transform that converts from the GSG's
//               external coordinate system (as returned by
//               get_coordinate_system()) to its internal coordinate
//               system (as returned by
//               get_internal_coordinate_system()).  This is used for
//               rendering.
////////////////////////////////////////////////////////////////////
CPT(TransformState) GraphicsStateGuardian::
get_cs_transform() const {
  return _cs_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::do_issue_clip_plane
//       Access: Public
//  Description: This is fundametically similar to do_issue_light(), with
//               calls to apply_clip_plane() and enable_clip_planes(),
//               as appropriate.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
do_issue_clip_plane() {
  int num_enabled = 0;
  int num_on_planes = 0;

  const ClipPlaneAttrib *target_clip_plane = DCAST(ClipPlaneAttrib, _target_rs->get_attrib_def(ClipPlaneAttrib::get_class_slot()));
  if (target_clip_plane != (ClipPlaneAttrib *)NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::do_issue_color
//       Access: Public
//  Description: This method is defined in the base class because it
//               is likely that this functionality will be used for
//               all (or at least most) kinds of
//               GraphicsStateGuardians--it's not specific to any one
//               rendering backend.
//
//               The ColorAttribute just changes the interpretation of
//               the color on the vertices, and fiddles with
//               _vertex_colors_enabled, etc.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
do_issue_color() {
  const ColorAttrib *target_color = DCAST(ColorAttrib, _target_rs->get_attrib_def(ColorAttrib::get_class_slot()));
  switch (target_color->get_color_type()) {
  case ColorAttrib::T_flat:
    // Color attribute flat: it specifies a scene graph color that
    // overrides the vertex color.
    _scene_graph_color = target_color->get_color();
    _has_scene_graph_color = true;
    _vertex_colors_enabled = false;
    break;

  case ColorAttrib::T_off:
    // Color attribute off: it specifies that no scene graph color is
    // in effect, and vertex color is not important either.
    _scene_graph_color.set(1.0f, 1.0f, 1.0f, 1.0f);
    _has_scene_graph_color = false;
    _vertex_colors_enabled = false;
    break;

  case ColorAttrib::T_vertex:
    // Color attribute vertex: it specifies that vertex color should
    // be revealed.
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::do_issue_color_scale
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
do_issue_color_scale() {
  // If the previous color scale had set a special texture, clear the
  // texture now.
  if (_has_texture_alpha_scale) {
    _state_mask.clear_bit(TextureAttrib::get_class_slot());
  }

  const ColorScaleAttrib *target_color_scale = DCAST(ColorScaleAttrib, _target_rs->get_attrib_def(ColorScaleAttrib::get_class_slot()));
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
      target_color_scale->has_alpha_scale()) {
    // This color scale will set a special texture--so again, clear
    // the texture.
    _state_mask.clear_bit(TextureAttrib::get_class_slot());
    _state_mask.clear_bit(TexMatrixAttrib::get_class_slot());

    _has_texture_alpha_scale = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::do_issue_light
//       Access: Protected, Virtual
//  Description: This implementation of do_issue_light() assumes
//               we have a limited number of hardware lights
//               available.  This function assigns each light to a
//               different hardware light id, trying to keep each
//               light associated with the same id where possible, but
//               reusing id's when necessary.  When it is no longer
//               possible to reuse existing id's (e.g. all id's are in
//               use), the next sequential id is assigned (if
//               available).
//
//               It will call apply_light() each time a light is
//               assigned to a particular id for the first time in a
//               given frame, and it will subsequently call
//               enable_light() to enable or disable each light as the
//               frame is rendered, as well as enable_lighting() to
//               enable or disable overall lighting.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
do_issue_light() {
  // Initialize the current ambient light total and newly enabled
  // light list
  LColor cur_ambient_light(0.0f, 0.0f, 0.0f, 0.0f);
  int i;

  int num_enabled = 0;
  int num_on_lights = 0;

  const LightAttrib *target_light = DCAST(LightAttrib, _target_rs->get_attrib_def(LightAttrib::get_class_slot()));
  if (display_cat.is_spam()) {
    display_cat.spam()
      << "do_issue_light: " << target_light << "\n";
  }
  if (target_light != (LightAttrib *)NULL) {
    CPT(LightAttrib) new_light = target_light->filter_to_max(_max_lights);
    if (display_cat.is_spam()) {
      new_light->write(display_cat.spam(false), 2);
    }

    num_on_lights = new_light->get_num_on_lights();
    for (int li = 0; li < num_on_lights; li++) {
      NodePath light = new_light->get_on_light(li);
      nassertv(!light.is_empty());
      Light *light_obj = light.node()->as_light();
      nassertv(light_obj != (Light *)NULL);

      // Lighting should be enabled before we apply any lights.
      if (!_lighting_enabled) {
        enable_lighting(true);
        _lighting_enabled = true;
      }

      if (light_obj->get_type() == AmbientLight::get_class_type()) {
        // Ambient lights don't require specific light ids; simply add
        // in the ambient contribution to the current total
        cur_ambient_light += light_obj->get_color();

      } else {
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
  }

  for (i = num_enabled; i < _num_lights_enabled; ++i) {
    enable_light(i, false);
  }
  _num_lights_enabled = num_enabled;

  // If no lights were set, disable lighting
  if (num_on_lights == 0) {
    if (_color_scale_via_lighting && (_has_material_force_color || _light_color_scale != LVecBase4(1.0f, 1.0f, 1.0f, 1.0f))) {
      // Unless we need lighting anyway to apply a color or color
      // scale.
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
    set_ambient_light(cur_ambient_light);
  }

  if (num_enabled != 0) {
    end_bind_lights();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::framebuffer_copy_to_texture
//       Access: Public, Virtual
//  Description: Copy the pixels within the indicated display
//               region from the framebuffer into texture memory.
//
//               If z > -1, it is the cube map index into which to
//               copy.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
framebuffer_copy_to_texture(Texture *, int, int, const DisplayRegion *,
                            const RenderBuffer &) {
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::framebuffer_copy_to_ram
//       Access: Public, Virtual
//  Description: Copy the pixels within the indicated display region
//               from the framebuffer into system memory, not texture
//               memory.  Returns true on success, false on failure.
//
//               This completely redefines the ram image of the
//               indicated texture.
////////////////////////////////////////////////////////////////////
bool GraphicsStateGuardian::
framebuffer_copy_to_ram(Texture *, int, int, const DisplayRegion *,
                        const RenderBuffer &) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
bind_light(PointLight *light_obj, const NodePath &light, int light_id) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
bind_light(DirectionalLight *light_obj, const NodePath &light, int light_id) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
bind_light(Spotlight *light_obj, const NodePath &light, int light_id) {
}

#ifdef DO_PSTATS
////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::init_frame_pstats
//       Access: Public, Static
//  Description: Initializes the relevant PStats data at the beginning
//               of the frame.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
init_frame_pstats() {
  if (PStatClient::is_connected()) {
    _data_transferred_pcollector.clear_level();
    _vertex_buffer_switch_pcollector.clear_level();
    _index_buffer_switch_pcollector.clear_level();

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

    _state_pcollector.clear_level();
    _transform_state_pcollector.clear_level();
    _texture_state_pcollector.clear_level();
  }
}
#endif  // DO_PSTATS


////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::create_gamma_table
//       Access: Public, Static
//  Description: Create a gamma table.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::reissue_transforms
//       Access: Protected, Virtual
//  Description: Called by clear_state_and_transform() to ensure that
//               the current modelview and projection matrices are
//               properly loaded in the graphics state, after a
//               callback might have mucked them up.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
reissue_transforms() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::enable_lighting
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable or disable the use of lighting overall.  This
//               is called by do_issue_light() according to whether any
//               lights are in use or not.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
enable_lighting(bool enable) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_ambient_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               indicate the color of the ambient light that should
//               be in effect.  This is called by do_issue_light() after
//               all other lights have been enabled or disabled.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
set_ambient_light(const LColor &color) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::enable_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated light id.  A specific Light will
//               already have been bound to this id via bind_light().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
enable_light(int light_id, bool enable) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_bind_lights
//       Access: Protected, Virtual
//  Description: Called immediately before bind_light() is called,
//               this is intended to provide the derived class a hook
//               in which to set up some state (like transform) that
//               might apply to several lights.
//
//               The sequence is: begin_bind_lights() will be called,
//               then one or more bind_light() calls, then
//               end_bind_lights().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
begin_bind_lights() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_bind_lights
//       Access: Protected, Virtual
//  Description: Called after before bind_light() has been called one
//               or more times (but before any geometry is issued or
//               additional state is changed), this is intended to
//               clean up any temporary changes to the state that may
//               have been made by begin_bind_lights().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
end_bind_lights() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::enable_clip_planes
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable or disable the use of clipping planes overall.
//               This is called by do_issue_clip_plane() according to
//               whether any planes are in use or not.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
enable_clip_planes(bool enable) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::enable_clip_plane
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated plane id.  A specific PlaneNode
//               will already have been bound to this id via
//               bind_clip_plane().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
enable_clip_plane(int plane_id, bool enable) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_bind_clip_planes
//       Access: Protected, Virtual
//  Description: Called immediately before bind_clip_plane() is called,
//               this is intended to provide the derived class a hook
//               in which to set up some state (like transform) that
//               might apply to several planes.
//
//               The sequence is: begin_bind_clip_planes() will be
//               called, then one or more bind_clip_plane() calls,
//               then end_bind_clip_planes().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
begin_bind_clip_planes() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::bind_clip_plane
//       Access: Public, Virtual
//  Description: Called the first time a particular clipping plane has been
//               bound to a given id within a frame, this should set
//               up the associated hardware (or API) clipping plane
//               with the plane's properties.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
bind_clip_plane(const NodePath &plane, int plane_id) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_bind_clip_planes
//       Access: Protected, Virtual
//  Description: Called after before bind_clip_plane() has been called one
//               or more times (but before any geometry is issued or
//               additional state is changed), this is intended to
//               clean up any temporary changes to the state that may
//               have been made by begin_bind_clip_planes().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
end_bind_clip_planes() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::determine_target_texture
//       Access: Protected
//  Description: Assigns _target_texture and _target_tex_gen
//               based on the _target_rs.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
determine_target_texture() {
  const TextureAttrib *target_texture = DCAST(TextureAttrib, _target_rs->get_attrib_def(TextureAttrib::get_class_slot()));
  const TexGenAttrib *target_tex_gen = DCAST(TexGenAttrib, _target_rs->get_attrib_def(TexGenAttrib::get_class_slot()));

  nassertv(target_texture != (TextureAttrib *)NULL &&
           target_tex_gen != (TexGenAttrib *)NULL);
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::free_pointers
//       Access: Protected, Virtual
//  Description: Frees some memory that was explicitly allocated
//               within the glgsg.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
free_pointers() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::close_gsg
//       Access: Protected, Virtual
//  Description: This is called by the associated GraphicsWindow when
//               close_window() is called.  It should null out the
//               _win pointer and possibly free any open resources
//               associated with the GSG.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
close_gsg() {
  // Protect from multiple calls, and also inform any other functions
  // not to try to create new stuff while we're going down.
  if (_closing_gsg) {
    return;
  }
  _closing_gsg = true;

  if (display_cat.is_debug()) {
    display_cat.debug()
      << this << " close_gsg " << get_type() << "\n";
  }
  free_pointers();

  // As tempting as it may be to try to release all the textures and
  // geoms now, we can't, because we might not be the currently-active
  // GSG (this is particularly important in OpenGL, which maintains
  // one currently-active GL state in each thread).  If we start
  // deleting textures, we'll be inadvertently deleting textures from
  // some other OpenGL state.

  // Fortunately, it doesn't really matter, since the graphics API
  // will be responsible for cleaning up anything we don't clean up
  // explicitly.  We'll just let them drop.

  // However, if any objects have recently been released, we have to
  // ensure they are actually deleted properly.
  Thread *current_thread = Thread::get_current_thread();
  _prepared_objects->begin_frame(this, current_thread);
  _prepared_objects->end_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::panic_deactivate
//       Access: Protected
//  Description: This is called internally when it is determined that
//               things are just fubar.  It temporarily deactivates
//               the GSG just so things don't get out of hand, and
//               throws an event so the application can deal with this
//               if it needs to.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
panic_deactivate() {
  if (_active) {
    display_cat.error()
      << "Deactivating " << get_type() << ".\n";
    set_active(false);
    throw_event("panic-deactivate-gsg", this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::determine_light_color_scale
//       Access: Protected
//  Description: Called whenever the color or color scale is changed,
//               if _color_scale_via_lighting is true.  This will
//               rederive _material_force_color and _light_color_scale
//               appropriately.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
determine_light_color_scale() {
  if (_has_scene_graph_color) {
    // If we have a scene graph color, it, plus the color scale, goes
    // directly into the material; we don't color scale the
    // lights--this allows an alpha color scale to work properly.
    _has_material_force_color = true;
    _material_force_color = _scene_graph_color;
    _light_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
    if (!_color_blend_involves_color_scale && _color_scale_enabled) {
      _material_force_color.set(_scene_graph_color[0] * _current_color_scale[0],
                                _scene_graph_color[1] * _current_color_scale[1],
                                _scene_graph_color[2] * _current_color_scale[2],
                                _scene_graph_color[3] * _current_color_scale[3]);
    }

  } else {
    // Otherise, leave the materials alone, but we might still scale
    // the lights.
    _has_material_force_color = false;
    _light_color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
    if (!_color_blend_involves_color_scale && _color_scale_enabled) {
      _light_color_scale = _current_color_scale;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_unlit_state
//       Access: Protected, Static
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
get_unlit_state() {
  static CPT(RenderState) state = NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make(LightAttrib::make_all_off());
  }
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_unclipped_state
//       Access: Protected, Static
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
get_unclipped_state() {
  static CPT(RenderState) state = NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make(ClipPlaneAttrib::make_all_off());
  }
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_untextured_state
//       Access: Protected, Static
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderState) GraphicsStateGuardian::
get_untextured_state() {
  static CPT(RenderState) state = NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make(TextureAttrib::make_off());
  }
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::async_reload_texture
//       Access: Protected
//  Description: Should be called when a texture is encountered that
//               needs to have its RAM image reloaded, and
//               get_incomplete_render() is true.  This will fire off
//               a thread on the current Loader object that will
//               request the texture to load its image.  The image
//               will be available at some point in the future (no
//               event will be generated).
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardian::
async_reload_texture(TextureContext *tc) {
  nassertv(_loader != (Loader *)NULL);

  int priority = 0;
  if (_current_display_region != (DisplayRegion *)NULL) {
    priority = _current_display_region->get_texture_reload_priority();
  }

  string task_name = string("reload:") + tc->get_texture()->get_name();
  PT(AsyncTaskManager) task_mgr = _loader->get_task_manager();
  
  // See if we are already loading this task.
  AsyncTaskCollection orig_tasks = task_mgr->find_tasks(task_name);
  int num_tasks = orig_tasks.get_num_tasks();
  for (int ti = 0; ti < num_tasks; ++ti) {
    AsyncTask *task = orig_tasks.get_task(ti);
    if (task->is_exact_type(TextureReloadRequest::get_class_type()) &&
        DCAST(TextureReloadRequest, task)->get_texture() == tc->get_texture()) {
      // This texture is already queued to be reloaded.  Don't queue
      // it again, just make sure the priority is updated, and return.
      task->set_priority(max(task->get_priority(), priority));
      return;
    }
  }

  // This texture has not yet been queued to be reloaded.  Queue it up
  // now.
  PT(AsyncTask) request = 
    new TextureReloadRequest(task_name,
                             _prepared_objects, tc->get_texture(),
                             _supports_compressed_texture);
  request->set_priority(priority);
  _loader->load_async(request);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::make_shadow_buffer
//       Access: Protected
//  Description: Creates a depth buffer for shadow mapping. This
//               is a convenience function for the ShaderGenerator;
//               putting this directly in the ShaderGenerator would
//               cause circular dependency issues.
//               Returns the depth texture.
////////////////////////////////////////////////////////////////////
PT(Texture) GraphicsStateGuardian::
make_shadow_buffer(const NodePath &light_np, GraphicsOutputBase *host) {
  // Make sure everything is valid.
  nassertr(light_np.node()->is_of_type(DirectionalLight::get_class_type()) ||
           light_np.node()->is_of_type(PointLight::get_class_type()) ||
           light_np.node()->is_of_type(Spotlight::get_class_type()), NULL);

  PT(LightLensNode) light = DCAST(LightLensNode, light_np.node());
  if (light == NULL || !light->_shadow_caster) {
    return NULL;
  }

  bool is_point = light->is_of_type(PointLight::get_class_type());

  nassertr(light->_sbuffers.count(this) == 0, NULL);

  display_cat.debug() << "Constructing shadow buffer for light '" << light->get_name()
    << "', size=" << light->_sb_xsize << "x" << light->_sb_ysize
    << ", sort=" << light->_sb_sort << "\n";

  // Setup some flags and properties
  FrameBufferProperties fbp;
  fbp.set_depth_bits(1); // We only need depth
  WindowProperties props = WindowProperties::size(light->_sb_xsize, light->_sb_ysize);
  int flags = GraphicsPipe::BF_refuse_window;
  if (is_point) {
    flags |= GraphicsPipe::BF_size_square;
  }

  // Create the buffer
  PT(GraphicsOutput) sbuffer = get_engine()->make_output(get_pipe(), light->get_name(),
      light->_sb_sort, fbp, props, flags, this, DCAST(GraphicsOutput, host));
  nassertr(sbuffer != NULL, NULL);

  // Create a texture and fill it in with some data to workaround an OpenGL error
  PT(Texture) tex = new Texture(light->get_name());
  if (is_point) {
    if (light->_sb_xsize != light->_sb_ysize) {
      display_cat.error()
        << "PointLight shadow buffers must have an equal width and height!\n";
    }
    tex->setup_cube_map(light->_sb_xsize, Texture::T_unsigned_byte, Texture::F_depth_component);
  } else {
    tex->setup_2d_texture(light->_sb_xsize, light->_sb_ysize, Texture::T_unsigned_byte, Texture::F_depth_component);
  }
  tex->make_ram_image();
  sbuffer->add_render_texture(tex, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_depth);

  // Set the wrap mode
  if (is_point) {
    tex->set_wrap_u(Texture::WM_clamp);
    tex->set_wrap_v(Texture::WM_clamp);
  } else {
    tex->set_wrap_u(Texture::WM_border_color);
    tex->set_wrap_v(Texture::WM_border_color);
    tex->set_border_color(LVecBase4(1, 1, 1, 1));
  }

  if (get_supports_shadow_filter()) {
    // If we have the ARB_shadow extension, enable shadow filtering.
    tex->set_minfilter(Texture::FT_shadow);
    tex->set_magfilter(Texture::FT_shadow);
  } else {
    // We only accept linear - this tells the GPU to use hardware PCF.
    tex->set_minfilter(Texture::FT_linear);
    tex->set_magfilter(Texture::FT_linear);
  }

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

  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_driver_vendor
//       Access: Public, Virtual
//  Description: Returns the vendor of the video card driver 
////////////////////////////////////////////////////////////////////
string GraphicsStateGuardian::
get_driver_vendor() {
  return string("0");
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_driver_vendor
//       Access: Public, Virtual
//  Description: Returns GL_Renderer
////////////////////////////////////////////////////////////////////
string GraphicsStateGuardian::get_driver_renderer() {
  return string("0");
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_driver_version
//       Access: Public, Virtual
//  Description: Returns driver version
//               This has an implementation-defined meaning, and may
//               be "0" if the particular graphics implementation
//               does not provide a way to query this information.
////////////////////////////////////////////////////////////////////
string GraphicsStateGuardian::
get_driver_version() {
  return string("0");
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_driver_version_major
//       Access: Public, Virtual
//  Description: Returns major version of the video driver.
//               This has an implementation-defined meaning, and may
//               be -1 if the particular graphics implementation
//               does not provide a way to query this information.
////////////////////////////////////////////////////////////////////
int GraphicsStateGuardian::
get_driver_version_major() {
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_driver_version_minor
//       Access: Public, Virtual
//  Description: Returns the minor version of the video driver.
//               This has an implementation-defined meaning, and may
//               be -1 if the particular graphics implementation
//               does not provide a way to query this information.
////////////////////////////////////////////////////////////////////
int GraphicsStateGuardian::
get_driver_version_minor() {
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_driver_shader_version_major
//       Access: Public, Virtual
//  Description: Returns the major version of the shader model.
////////////////////////////////////////////////////////////////////
int GraphicsStateGuardian::
get_driver_shader_version_major() {
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_driver_shader_version_minor
//       Access: Public, Virtual
//  Description: Returns the minor version of the shader model.
////////////////////////////////////////////////////////////////////
int GraphicsStateGuardian::
get_driver_shader_version_minor() {
  return -1;
}
