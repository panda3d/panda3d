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
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_properties
//       Access: Published
//  Description: Returns the frame buffer properties requested for
//               this GSG.  All windows created for this GSG must be
//               created with the same properties.
////////////////////////////////////////////////////////////////////
INLINE const FrameBufferProperties &GraphicsStateGuardian::
get_properties() const {
  return _properties;
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
//     Function: GraphicsStateGuardian::set_scene
//       Access: Public
//  Description: Sets the SceneSetup object that indicates the initial
//               camera position, etc.  This must be called before
//               traversal begins.  Returns true if the scene is
//               acceptable, false if something's wrong.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
set_scene(SceneSetup *scene_setup) {
  _scene_setup = scene_setup;
  _current_lens = scene_setup->get_lens();
  if (_current_lens == (Lens *)NULL) {
    return false;
  }
  return prepare_lens();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_scene
//       Access: Public
//  Description: Returns the SceneSetup object.
////////////////////////////////////////////////////////////////////
INLINE SceneSetup *GraphicsStateGuardian::
get_scene() const {
  return _scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::clear
//       Access: Public
//  Description: Clears the framebuffer within the indicated
//               DisplayRegion, according to the flags indicated by
//               the DisplayRegion object (inheriting from
//               DrawableRegion).  Note that by default, a
//               DisplayRegion does not have any clear flags set, in
//               which case this function will do nothing.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
clear(DisplayRegion *dr) {
  DisplayRegionStack old_dr = push_display_region(dr);
  prepare_display_region();
  clear((DrawableRegion *)dr);
  pop_display_region(old_dr);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::force_normals
//       Access: Public
//  Description: Temporarily forces the GSG to issue normals to the
//               graphics pipe.  Normally, the GSG will issue normals
//               only if lighting is on.
//
//               This call must be matched with exactly one call to
//               undo_force_normals().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
force_normals() {
  nassertr(_force_normals >= 0, _force_normals);
  _force_normals++;
  return _force_normals;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::undo_force_normals
//       Access: Public
//  Description: Undoes the effect of a previous call to
//               force_normals().
//
//               This call must be matched with one-to-one with a
//               previous call to force_normals().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsStateGuardian::
undo_force_normals() {
  _force_normals--;
  nassertr(_force_normals >= 0, _force_normals);
  return _force_normals;
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
//     Function: GraphicsStateGuardian::modify_state
//       Access: Public
//  Description: Applies the attributes indicated in the state set to
//               the current state, and issues the changes to the
//               graphics hardware.
//
//               Any transitions not mentioned are left unchanged.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
modify_state(const RenderState *state) {
  if (!state->is_empty()) {
#ifndef NDEBUG
    if (gsg_cat.is_spam()) {
      gsg_cat.spam() << "Modifying GSG state with:\n";
      state->write(gsg_cat.spam(false), 2);
    }
#endif
    _state_pcollector.add_level(1);
    _state = _state->issue_delta_modify(state, this);
    finish_modify_state();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_state
//       Access: Public
//  Description: Applies the attributes indicated in the state set to
//               the current state, and issues the changes to the
//               graphics hardware.
//
//               The state is taken to be a complete description of
//               what the graphics state should be; any transitions
//               not mentioned are implicitly reset to their initial
//               values.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_state(const RenderState *state) {
  if (state != _state) {
#ifndef NDEBUG
    if (gsg_cat.is_spam()) {
      gsg_cat.spam() << "Setting GSG state to:\n";
      state->write(gsg_cat.spam(false), 2);
    }
#endif
    _state_pcollector.add_level(1);
    _state = _state->issue_delta_set(state, this);
    finish_modify_state();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_transform
//       Access: Public
//  Description: Sets the world transform that will be applied to
//               subsequent geometry.  This is normally called only
//               during the draw process, immediately before issuing
//               geometry commands.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_transform(const TransformState *transform) {
  if (transform != _transform) {
    _state_pcollector.add_level(1);
    _transform = transform;
    issue_transform(transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_current_display_region
//       Access: Public
//  Description: Returns the current display region being rendered to,
//               as set by the last call to push_display_region() (or
//               restored by pop_display_region()).  This display
//               region will be made active (if it is not already) by
//               a call to prepare_display_region().
////////////////////////////////////////////////////////////////////
INLINE const DisplayRegion *GraphicsStateGuardian::
get_current_display_region(void) const {
  return _current_display_region;
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
//     Function: GraphicsStateGuardian::push_display_region
//       Access: Public
//  Description: Saves the current display region information and sets
//               up a new display region for rendering.  The return
//               value from this function must eventually be passed to
//               a matching pop_display_region() call.
//
//               The new display region will not actually be made
//               active for rendering until the next call to
//               prepare_display_region().  This is a state-changing
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE DisplayRegionStack GraphicsStateGuardian::
push_display_region(const DisplayRegion *dr) {
  DisplayRegionStack old;
  old._display_region = _current_display_region;
  old._stack_level = _display_region_stack_level;
  _display_region_stack_level++;
  _current_display_region = dr;
  return old;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::pop_display_region
//       Access: Public
//  Description: Restores the display region previously in effect,
//               before the matching call to push_display_region().
//
//               The newly-restored display region will not actually
//               be made active for rendering until the next call to
//               prepare_display_region().  This is a state-changing
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
pop_display_region(DisplayRegionStack &node) {
  nassertv(_display_region_stack_level > 0);
  _display_region_stack_level--;
  nassertv(node._stack_level == _display_region_stack_level);
  _current_display_region = node._display_region;
  node._stack_level = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_coordinate_system
//       Access: Public
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
//     Function: GraphicsStateGuardian::get_internal_coordinate_system
//       Access: Public
//  Description: Returns the coordinate system used internally by the
//               GSG, if any one particular coordinate system is used.
//               The default, CS_default, indicates that the GSG can
//               use any coordinate system.
//
//               If this returns other than CS_default, the
//               GraphicsEngine will automatically convert all
//               transforms into the indicated coordinate system.
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem GraphicsStateGuardian::
get_internal_coordinate_system() const {
  return _coordinate_system;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_cs_transform
//       Access: Public
//  Description: Returns a transform that converts from the GSG's
//               external coordinate system (as returned by
//               get_coordinate_system()) to its internal coordinate
//               system (as returned by
//               get_internal_coordinate_system()).  This is used for
//               rendering.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *GraphicsStateGuardian::
get_cs_transform() const {
  return _cs_transform;
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
INLINE PlaneNode *GraphicsStateGuardian::
get_clip_plane(int plane_id) const {
  nassertr(plane_id >= 0 && plane_id < (int)_clip_plane_info.size(), (PlaneNode *)NULL);
  return _clip_plane_info[plane_id]._plane;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_properties
//       Access: Protected
//  Description: Changes the frame buffer properties structure
//               representing this GSG's capabilities.  Normally, this
//               should only be called by a derived class wishing to
//               indicate that the originally-requested capabilities
//               could not be granted.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_properties(const FrameBufferProperties &properties) {
  _properties = properties;
}
