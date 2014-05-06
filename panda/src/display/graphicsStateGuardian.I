// Filename: graphicsStateGuardian.I
// Created by:  drose (24Sep99)
// Updated by: fperazzi, PandaSE (29Apr10) (added 
// get_max_2d_texture_array_layers and related)
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


////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_all
//       Access: Public
//  Description: Releases all prepared objects.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
release_all() {
  _prepared_objects->release_all();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_all_textures
//       Access: Public
//  Description: Frees the resources for all textures associated with
//               this GSG.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
release_all_textures() {
  return _prepared_objects->release_all_textures();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_all_geoms
//       Access: Public
//  Description: Frees the resources for all geoms associated with
//               this GSG.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
release_all_geoms() {
  return _prepared_objects->release_all_geoms();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_all_vertex_buffers
//       Access: Public
//  Description: Frees the resources for all vertex buffers associated
//               with this GSG.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
release_all_vertex_buffers() {
  return _prepared_objects->release_all_vertex_buffers();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::release_all_index_buffers
//       Access: Public
//  Description: Frees the resources for all index buffers associated
//               with this GSG.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
release_all_index_buffers() {
  return _prepared_objects->release_all_index_buffers();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_active
//       Access: Published
//  Description: Sets the active flag associated with the
//               GraphicsStateGuardian.  If the GraphicsStateGuardian
//               is marked inactive, nothing is rendered.  This is not
//               normally turned off unless there is a problem with
//               the rendering detected at a low level.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::is_active
//       Access: Published
//  Description: Returns the active flag associated with the
//               GraphicsStateGuardian.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
is_active() const {
  return _active && _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::is_valid
//       Access: Published
//  Description: Returns true if the GSG has been correctly
//               initialized within a graphics context, false if there
//               has been some problem or it hasn't been initialized
//               yet.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::needs_reset
//       Access: Public
//  Description: Returns true if the gsg is marked as needing a
//               reset.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
needs_reset() const {
  return _needs_reset;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_incomplete_render
//       Access: Public
//  Description: Sets the incomplete_render flag.  When this is
//               true, the frame will be rendered even if some of the
//               geometry or textures in the scene are not available
//               (e.g. they have been temporarily paged out).  When
//               this is false, the frame will be held up while this
//               data is reloaded.
//
//               Setting this true allows for a smoother frame rate,
//               but occasionally parts of the frame will be invisible
//               or missing (they will generally come in within a
//               second or two).  Setting this false guarantees that
//               every frame will be complete, but may cause more
//               chugs as things are loaded up at runtime.
//
//               You may want to set this false during loading
//               screens, to guarantee that all of your assets are
//               available by the time you take the loading screen
//               down.
//
//               This flag may also be set individually on each
//               DisplayRegion.  It will be considered true for a
//               given DisplayRegion only if it is true on both the
//               GSG and on the DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_incomplete_render(bool incomplete_render) {
  _incomplete_render = incomplete_render;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_incomplete_render
//       Access: Public, Virtual
//  Description: Returns the incomplete_render flag.  See
//               set_incomplete_render().
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_incomplete_render() const {
  return _incomplete_render;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_effective_incomplete_render
//       Access: Public, Virtual
//  Description: Returns true if the GSG is effectively in
//               incomplete_render state, considering both the GSG's
//               incomplete_render and its current DisplayRegion's
//               incomplete_render flags.  It only makes sense to call
//               this during the draw traversal; at other times this
//               return value will be meaningless.
//
//               See CullTraverser::get_effective_incomplete_render()
//               for this same information during the cull traversal.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_effective_incomplete_render() const {
  return _effective_incomplete_render;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_loader
//       Access: Public
//  Description: Sets the Loader object that will be used by this GSG
//               to load textures when necessary, if
//               get_incomplete_render() is true.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_loader(Loader *loader) {
  _loader = loader;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_loader
//       Access: Public, Virtual
//  Description: Returns the Loader object that will be used by this
//               GSG to load textures when necessary, if
//               get_incomplete_render() is true.
////////////////////////////////////////////////////////////////////
INLINE Loader *GraphicsStateGuardian::
get_loader() const {
  return _loader;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_pipe
//       Access: Published
//  Description: Returns the graphics pipe on which this GSG was
//               created.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipe *GraphicsStateGuardian::
get_pipe() const {
  return _pipe;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_threading_model
//       Access: Published
//  Description: Returns the threading model that was used to create
//               this GSG.
////////////////////////////////////////////////////////////////////
INLINE const GraphicsThreadingModel &GraphicsStateGuardian::
get_threading_model() const {
  return _threading_model;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::is_hardware
//       Access: Published
//  Description: Returns true if this GSG appears to be
//               hardware-accelerated, or false if it is known to be
//               software only.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
is_hardware() const {
  return _is_hardware;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::prefers_triangle_strips
//       Access: Published, Virtual
//  Description: Returns true if this GSG strongly prefers triangle
//               strips to individual triangles (such as SGI), or
//               false if it prefers to minimize the number of
//               primitive batches, even at the expense of triangle
//               strips (such as most PC hardware).
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
prefers_triangle_strips() const {
  return _prefers_triangle_strips;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_vertices_per_array
//       Access: Published, Virtual
//  Description: Returns the maximum number of vertices that should be
//               put into any one GeomVertexData object for use with
//               this GSG.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_vertices_per_array() const {
  return _max_vertices_per_array;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_vertices_per_primitive
//       Access: Published, Virtual
//  Description: Returns the maximum number of vertex indices that
//               should be put into any one GeomPrimitive object for
//               use with this GSG.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_vertices_per_primitive() const {
  return _max_vertices_per_primitive;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_texture_stages
//       Access: Published
//  Description: Returns the maximum number of simultaneous textures
//               that may be applied to geometry with multitexturing,
//               as supported by this particular GSG.  If you exceed
//               this number, the lowest-priority texture stages will
//               not be applied.  Use TextureStage::set_priority() to
//               adjust the relative importance of the different
//               texture stages.
//
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_texture_stages() const {
  if (max_texture_stages > 0) {
    return min(_max_texture_stages, (int)max_texture_stages);
  }
  return _max_texture_stages;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_texture_dimension
//       Access: Published
//  Description: Returns the largest possible texture size in any one
//               dimension supported by the GSG, or -1 if there is no
//               particular limit.
//
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_texture_dimension() const {
  return _max_texture_dimension;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_3d_texture_dimension
//       Access: Published
//  Description: Returns the largest possible texture size in any one
//               dimension for a 3-d texture, or -1 if there is no
//               particular limit.  Returns 0 if 3-d textures are not
//               supported.
//
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_3d_texture_dimension() const {
  return _max_3d_texture_dimension;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_2d_texture_array_layers
//       Access: Published
//  Description: Returns the largest possible number of pages, or -1 
//               if there is no particular limit. Returns 0 if 2-d
//               texture arrays not supported.
//               
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_2d_texture_array_layers() const {
  return _max_2d_texture_array_layers;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_cube_map_dimension
//       Access: Published
//  Description: Returns the largest possible texture size in any one
//               dimension for a cube map texture, or -1 if there is
//               no particular limit.  Returns 0 if cube map textures
//               are not supported.
//
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_cube_map_dimension() const {
  return _max_cube_map_dimension;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_texture_combine
//       Access: Published
//  Description: Returns true if this particular GSG can use the
//               TextureStage::M_combine mode, which includes all of
//               the texture blend modes specified by
//               set_combine_rgb() and/or set_combine_alpha().  If
//               this is false, you must limit yourself to using the
//               simpler blend modes.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_texture_combine() const {
  return _supports_texture_combine;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_texture_saved_result
//       Access: Published
//  Description: Returns true if this GSG can use the
//               TextureStage::CS_last_saved_result source, which
//               allows you to save the result of a TextureStage and
//               re-use it for multiple inputs.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_texture_saved_result() const {
  return _supports_texture_saved_result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_texture_dot3
//       Access: Published
//  Description: Returns true if this GSG can use the
//               TextureStage::CM_dot3_rgb or CM_dot3_rgba combine
//               modes.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_texture_dot3() const {
  return _supports_texture_dot3;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_3d_texture
//       Access: Published
//  Description: Returns true if this GSG can render 3-d (volumetric)
//               textures.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_3d_texture() const {
  return _supports_3d_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_2d_texture_array
//       Access: Published
//  Description: Returns true if this GSG can render 2-d textures
//               array.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_2d_texture_array() const {
  return _supports_2d_texture_array;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_cube_map
//       Access: Published
//  Description: Returns true if this GSG can render cube map textures.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_cube_map() const {
  return _supports_cube_map;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_tex_non_pow2
//       Access: Published
//  Description: Returns true if this GSG can handle non power of two
//               sized textures.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_tex_non_pow2() const {
  return _supports_tex_non_pow2;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_compressed_texture
//       Access: Published
//  Description: Returns true if this GSG can compress textures as it
//               loads them into texture memory, and/or accept
//               pre-compressed textures for storing.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_compressed_texture() const {
  return _supports_compressed_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_compressed_texture_format
//       Access: Published, Virtual
//  Description: Returns true if this GSG can accept textures
//               pre-compressed in the indicated format.
//               compression_mode may be any of the
//               Texture::CompressionMode enums.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_compressed_texture_format(int compression_mode) const {
  return _compressed_texture_formats.get_bit(compression_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_lights
//       Access: Published
//  Description: Returns the maximum number of simultaneous lights
//               that may be rendered on geometry, or -1 if there is
//               no particular limit.
//
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_lights() const {
  return _max_lights;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_clip_planes
//       Access: Published
//  Description: Returns the maximum number of simultaneous clip planes
//               that may be applied to geometry, or -1 if there is
//               no particular limit.
//
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_clip_planes() const {
  return _max_clip_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_vertex_transforms
//       Access: Published
//  Description: Returns the maximum number of transform matrices that
//               may be simultaneously used to transform any one
//               vertex by the graphics hardware.  If this number is
//               0, then the hardware (or the graphics backend)
//               doesn't support soft-skinned vertices (in which case
//               Panda will animate the vertices in software).
//
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_vertex_transforms() const {
  return _max_vertex_transforms;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_vertex_transform_indices
//       Access: Published
//  Description: Returns the maximum number of transforms there may be
//               in a single TransformTable for this graphics
//               hardware.  If this number is 0 (but
//               get_max_transforms() is nonzero), then the graphics
//               hardware (or API) doesn't support indexed transforms,
//               but can support direct transform references.
//
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_vertex_transform_indices() const {
  return _max_vertex_transform_indices;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_copy_texture_inverted
//       Access: Published
//  Description: Returns true if this particular GSG has the property
//               that any framebuffer-to-texture copy results in a
//               texture that is upside-down and backwards from
//               Panda's usual convention; that is, it copies into a
//               texture from the bottom up instead of from the top
//               down.
//
//               If this is true, then on offscreen GraphicsBuffer
//               created for the purposes of rendering into a texture
//               should be created with the invert flag set true, to
//               compensate.  Panda will do this automatically if you
//               create an offscreen buffer using
//               GraphicsOutput::make_texture_buffer().
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_copy_texture_inverted() const {
  // If this is set from a Config variable, that overrides.
  if (copy_texture_inverted.has_value()) {
    return copy_texture_inverted;
  }

  // Otherwise, use whatever behavior the GSG figured for itself.
  return _copy_texture_inverted;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_generate_mipmap
//       Access: Published
//  Description: Returns true if this particular GSG can generate
//               mipmaps for a texture automatically, or if they must
//               be generated in software.  If this is true, then
//               mipmaps can safely be enabled for rendered textures
//               (e.g. using the MultitexReducer).
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_generate_mipmap() const {
  return _supports_generate_mipmap;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_depth_texture
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               textures whose format is F_depth_stencil.  This
//               returns true if the GSG supports GL_DEPTH_COMPONENT
//               textures, which are considered a limited but still
//               valid case of F_depth_stencil.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_depth_texture() const {
  return _supports_depth_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_depth_stencil
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               textures whose format is F_depth_stencil.  This
//               only returns true if the GSG supports the full
//               packed depth-stencil functionality.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_depth_stencil() const {
  return _supports_depth_stencil;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_shadow_filter
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               the filter mode FT_shadow for depth textures.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_shadow_filter() const {
  return _supports_shadow_filter;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_basic_shaders
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               arbfp1+arbvp1 or above.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_basic_shaders() const {
  return _supports_basic_shaders;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_geometry_shaders
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               geometry shaders.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_geometry_shaders() const {
  return _supports_geometry_shaders;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_tessellation_shaders
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               tesselation shaders.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_tessellation_shaders() const {
  return _supports_tessellation_shaders;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_glsl
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               GLSL shaders.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_glsl() const {
  return _supports_glsl;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_stencil
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               stencil buffers at all.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_stencil() const {
  return _supports_stencil;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_two_sided_stencil
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               two sided stencil: different stencil settings for the
//               front and back side of the same polygon.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_two_sided_stencil() const {
  return _supports_two_sided_stencil;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_geometry_instancing
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               hardware geometry instancing: the ability to render
//               multiple copies of a model. In OpenGL, this is
//               done using the EXT_draw_instanced extension.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_geometry_instancing() const {
  return _supports_geometry_instancing;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_max_color_targets
//       Access: Published
//  Description: Returns the maximum number of simultaneous color
//               textures that may be attached for render-to-texture,
//               as supported by this particular GSG.  If you exceed
//               this number, the lowest-priority render targets will
//               not be applied.  Use RenderTarget::set_priority() to
//               adjust the relative importance of the different
//               render targets.
//
//               The value returned may not be meaningful until after
//               the graphics context has been fully created (e.g. the
//               window has been opened).
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_max_color_targets() const {
  if (max_color_targets > 0) {
    return min(_max_color_targets, (int)max_color_targets);
  }
  return _max_color_targets;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_maximum_simultaneous_render_targets
//       Access: Published
//  Description: Deprecated.  Use get_max_color_targets() instead,
//               which returns the exact same value.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_maximum_simultaneous_render_targets() const {
  return get_max_color_targets();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_shader_model
//       Access: Published
//  Description: Returns the ShaderModel
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_shader_model() const {
  return _shader_model;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_shader_model
//       Access: Published
//  Description: Sets the ShaderModel.  This will override the auto-
//               detected shader model during GSG reset.  Useful for
//               testing lower-end shaders.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_shader_model(int shader_model) {
  if (shader_model <= _auto_detect_shader_model) {
    _shader_model = shader_model;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_color_scale_via_lighting
//       Access: Published
//  Description: Returns true if this particular GSG can implement (or
//               would prefer to implement) set color and/or color
//               scale using materials and/or ambient lights, or
//               false if we need to actually munge the color.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_color_scale_via_lighting() const {
  return _color_scale_via_lighting;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_alpha_scale_via_texture
//       Access: Published
//  Description: Returns true if this particular GSG can implement (or
//               would prefer to implement) an alpha scale via an
//               additional Texture layer, or false if we need to
//               actually munge the alpha.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_alpha_scale_via_texture() const {
  return _alpha_scale_via_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_alpha_scale_via_texture
//       Access: Published
//  Description: This variant of get_alpha_scale_via_texture() answers
//               the question of whether the GSG can implement an
//               alpha scale via an additional Texture layer,
//               considering the current TextureAttrib that will be in
//               effect.  This considers whether there is at least one
//               additional texture slot available on the GSG.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_alpha_scale_via_texture(const TextureAttrib *tex_attrib) const {
  return _alpha_scale_via_texture &&
    (tex_attrib == (const TextureAttrib *)NULL ||
     tex_attrib->get_num_on_stages() < get_max_texture_stages());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_alpha_scale_texture_stage
//       Access: Published, Static
//  Description: Returns the TextureStage that will be used to apply
//               an alpha scale, if get_alpha_scale_via_texture()
//               returns true.
////////////////////////////////////////////////////////////////////
INLINE TextureStage *GraphicsStateGuardian::
get_alpha_scale_texture_stage() {
  if (_alpha_scale_texture_stage == (TextureStage *)NULL) {
    _alpha_scale_texture_stage = new TextureStage("alpha-scale");
    _alpha_scale_texture_stage->set_sort(1000000000);
  }
  return _alpha_scale_texture_stage;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_runtime_color_scale
//       Access: Published
//  Description: Returns true if this particular GSG can implement (or
//               would prefer to implement) set color and/or color
//               scale directly, without requiring any munging of
//               vertices or tricks with lighting.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_runtime_color_scale() const {
  return _runtime_color_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_coordinate_system
//       Access: Published
//  Description: Returns the coordinate system in effect on this
//               particular gsg.  Normally, this will be the default
//               coordinate system, but it might be set differently at
//               runtime.
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem GraphicsStateGuardian::
get_coordinate_system() const {
  return _coordinate_system;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_texture_quality_override
//       Access: Published
//  Description: Specifies the global quality_level to be imposed for
//               all Textures rendered by this GSG.  This overrides
//               the value set on individual textures via
//               Texture::set_quality_level().  Set this to
//               Texture::QL_default in order to allow the individual
//               texture quality levels to be respected.
//
//               This is mainly useful for the tinydisplay software
//               renderer.  See Texture::set_quality_level().
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_texture_quality_override(Texture::QualityLevel quality_level) {
  _texture_quality_override = quality_level;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_texture_quality_override
//       Access: Published
//  Description: Returns the global quality_level override specified
//               by set_texture_quality_override.
//
//               This is mainly useful for the tinydisplay software
//               renderer.  See Texture::set_quality_level().
////////////////////////////////////////////////////////////////////
INLINE Texture::QualityLevel GraphicsStateGuardian::
get_texture_quality_override() const {
  return _texture_quality_override;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::reset_if_new
//       Access: Public
//  Description: Calls reset() to initialize the GSG, but only if it
//               hasn't been called yet.  Returns true if the GSG was
//               new, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
reset_if_new() {
  if (_needs_reset) {
    reset();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::mark_new
//       Access: Public
//  Description: Marks the GSG as "new", so that the next call to
//               reset_if_new() will be effective.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
mark_new() {
  _needs_reset = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_external_transform
//       Access: Public
//  Description: Fetches the external net transform.  This
//               transform is generally only set when geometry is
//               about to be rendered.  Therefore, this "get" function
//               is typically only meaningful during the geometry
//               rendering process.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) GraphicsStateGuardian::
get_external_transform() const {
  return _inv_cs_transform->compose(_internal_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_internal_transform
//       Access: Public
//  Description: Fetches the external net transform.  This
//               transform is generally only set when geometry is
//               about to be rendered.  Therefore, this "get" function
//               is typically only meaningful during the geometry
//               rendering process.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) GraphicsStateGuardian::
get_internal_transform() const {
  return _internal_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_current_display_region
//       Access: Public
//  Description: Returns the current display region being rendered to,
//               as set by the last call to prepare_display_region().
////////////////////////////////////////////////////////////////////
INLINE const DisplayRegion *GraphicsStateGuardian::
get_current_display_region() const {
  return _current_display_region;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_current_stereo_channel
//       Access: Public
//  Description: Returns the current stereo channel being rendered to,
//               as set by the last call to prepare_display_region().
////////////////////////////////////////////////////////////////////
INLINE Lens::StereoChannel GraphicsStateGuardian::
get_current_stereo_channel() const {
  return _current_stereo_channel;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_current_tex_view_offset
//       Access: Public
//  Description: Returns the current tex view offset, as set by the
//               last call to prepare_display_region().  This is read
//               from the current DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
get_current_tex_view_offset() const {
  return _current_tex_view_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_current_lens
//       Access: Public
//  Description: Returns the current lens being used to render,
//               according to the scene specified via the last call to
//               set_scene().
////////////////////////////////////////////////////////////////////
INLINE const Lens *GraphicsStateGuardian::
get_current_lens() const {
  return _current_lens;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_inv_cs_transform
//       Access: Public
//  Description: Returns the inverse of the transform returned by
//               get_cs_transform().
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) GraphicsStateGuardian::
get_inv_cs_transform() const {
  return _inv_cs_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_current_properties
//       Access: Protected
//  Description: Notifies the gsg that it is about to render into a
//               window/buffer with the given FrameBufferProperties
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_current_properties(const FrameBufferProperties *prop) {
  _current_properties = prop;
}

