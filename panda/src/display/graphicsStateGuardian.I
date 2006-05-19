// Filename: graphicsStateGuardian.I
// Created by:  drose (24Sep99)
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


////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::LightInfo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardian::LightInfo::
LightInfo() {
  _enabled = false;
  _next_enabled = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::ClipPlaneInfo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardian::ClipPlaneInfo::
ClipPlaneInfo() {
  _enabled = false;
  _next_enabled = false;
}


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
//     Function: GraphicsStateGuardian::get_properties
//       Access: Published
//  Description: Returns the default frame buffer properties for
//               this GSG.  All windows created for this GSG will be
//               created with these properties, unless explicitly
//               overridden.
////////////////////////////////////////////////////////////////////
INLINE const FrameBufferProperties &GraphicsStateGuardian::
get_default_properties() const {
  return _default_properties;
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
//     Function: GraphicsStateGuardian::get_engine
//       Access: Published
//  Description: Returns the graphics engine that created this GSG.
////////////////////////////////////////////////////////////////////
INLINE GraphicsEngine *GraphicsStateGuardian::
get_engine() const {
  return _engine;
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
//     Function: GraphicsStateGuardian::prefers_triangle_strips
//       Access: Published
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
//       Access: Published
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
//       Access: Published
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
//       Access: Published
//  Description: Returns true if this GSG can accept textures
//               pre-compressed in the indicated format.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_compressed_texture_format(Texture::CompressionMode compression) const {
  return _compressed_texture_formats.get_bit(compression);
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
//     Function: GraphicsStateGuardian::get_supports_render_texture
//       Access: Published
//  Description: Returns true if this particular GSG can render
//               directly into a texture, or false if it must always
//               copy-to-texture at the end of each frame to achieve
//               this effect.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_render_texture() const {
  return _supports_render_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_supports_depth_texture
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               textures whose format is F_depth_component.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_depth_texture() const {
  return _supports_depth_texture;
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
//     Function: GraphicsStateGuardian::get_supports_two_sided_stencil
//       Access: Published
//  Description: Returns true if this particular GSG supports
//               two sided stencil.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
get_supports_two_sided_stencil() const {
  return _supports_two_sided_stencil;
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
//     Function: GraphicsStateGuardian::make_global_gsg
//       Access: Published
//  Description: Marks this particular GraphicsStateGuardian as the
//               "global" GSG, which is used for optimization hints by
//               operations like NodePath::flatten_strong().
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
make_global_gsg() {
  _global_gsg = this;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_global_gsg
//       Access: Published, Static
//  Description: Returns the "global" GSG, which is to say, the
//               GraphicsStateGuardian object that has most recently
//               had make_global_gsg() called for it.  It may return
//               NULL if there is no such GSG.
//
//               This object should be used for optimization hints
//               where appropriate, for instance by operations like
//               NodePath::flatten_strong().
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardian *GraphicsStateGuardian::
get_global_gsg() {
  return _global_gsg;
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
INLINE const TransformState *GraphicsStateGuardian::
get_inv_cs_transform() const {
  return _inv_cs_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_light
//       Access: Protected
//  Description: Returns the Light object that is bound to the
//               indicated id, or empty NodePath if no Light is bound.
////////////////////////////////////////////////////////////////////
INLINE NodePath GraphicsStateGuardian::
get_light(int light_id) const {
  nassertr(light_id >= 0 && light_id < (int)_light_info.size(), NodePath::fail());
  return _light_info[light_id]._light;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_clip_plane
//       Access: Protected
//  Description: Returns the PlaneNode object that is bound to the
//               indicated id, or NULL if no PlaneNode is bound.
////////////////////////////////////////////////////////////////////
INLINE NodePath GraphicsStateGuardian::
get_clip_plane(int plane_id) const {
  nassertr(plane_id >= 0 && plane_id < (int)_clip_plane_info.size(), NodePath::fail());
  return _clip_plane_info[plane_id]._plane;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_current_properties
//       Access: Protected
//  Description: Notifies the gsg that it is about to render into a
//               window/buffer with the given FrameBufferProperties
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_current_properties(FrameBufferProperties *prop) {
  _current_properties = prop;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_stencil_clear_value
//       Access: Public
//  Description: Set current stencil clear value.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_stencil_clear_value(unsigned int stencil_clear_value) {
  _stencil_clear_value = stencil_clear_value;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_stencil_clear_value
//       Access: Public
//  Description: Get current stencil clear value.
////////////////////////////////////////////////////////////////////
INLINE unsigned int GraphicsStateGuardian::
get_stencil_clear_value() {
  return _stencil_clear_value;
}

