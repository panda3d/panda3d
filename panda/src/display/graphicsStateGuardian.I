// Filename: graphicsStateGuardian.I
// Created by:  drose (24Sep99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
//     Function: GraphicsStateGuardian::is_closed
//       Access: Public
//  Description: Returns true if the window associated with this GSG
//               has been closed, and hence the resources associated
//               with this GSG have been freed.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
is_closed() const {
  return (_win == (GraphicsWindow *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_scene
//       Access: Public
//  Description: Sets the SceneSetup object that indicates the initial
//               camera position, etc.  This must be called before
//               traversal begins.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_scene(SceneSetup *scene_setup) {
  _scene_setup = scene_setup;
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
//               ClearableRegion).  Note that by default, a
//               DisplayRegion does not have any clear flags set, in
//               which case this function will do nothing.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
clear(DisplayRegion *dr) {
  DisplayRegionStack old_dr = push_display_region(dr);
  prepare_display_region();
  clear((ClearableRegion *)dr);
  pop_display_region(old_dr);
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
#ifndef NDEBUG
  if (gsg_cat.is_spam()) {
    gsg_cat.spam() << "Modifying GSG state with:\n";
    state->write(gsg_cat.spam(false), 2);
  }
#endif
  _state = _state->issue_delta_modify(state, this);
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
#ifndef NDEBUG
  if (gsg_cat.is_spam()) {
    gsg_cat.spam() << "Setting GSG state to:\n";
    state->write(gsg_cat.spam(false), 2);
  }
#endif
  _state = _state->issue_delta_set(state, this);
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
//  Description: Returns the current lens being rendered with, as set
//               by the last call to push_lens() (or restored by
//               pop_lens()).  This lens will be made active (if it is
//               not already) by a call to prepare_lens().
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
//     Function: GraphicsStateGuardian::push_frame_buffer
//       Access: Public
//  Description: Saves the contents of the frame buffer (within the
//               indicated display region only) so that rendering may
//               be performed (for instance, to render a partial in a
//               multipass algorithm) and the frame buffer later
//               restored via a matching call to pop_frame_buffer().
////////////////////////////////////////////////////////////////////
INLINE FrameBufferStack GraphicsStateGuardian::
push_frame_buffer(const RenderBuffer &buffer,
                  const DisplayRegion *dr) {
  FrameBufferStack old;
  old._frame_buffer = save_frame_buffer(buffer, dr);
  old._stack_level = _frame_buffer_stack_level;
  _frame_buffer_stack_level++;
  return old;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::pop_frame_buffer
//       Access: Public
//  Description: Restores the contents of the frame buffer as saved by
//               a previous call to push_frame_buffer().
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
pop_frame_buffer(FrameBufferStack &node) {
  nassertv(_frame_buffer_stack_level > 0);
  _frame_buffer_stack_level--;
  nassertv(node._stack_level == _frame_buffer_stack_level);
  restore_frame_buffer(node._frame_buffer);
  node._stack_level = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::push_lens
//       Access: Public
//  Description: Saves the current lens information and sets up a new
//               lens for rendering.  The return value from this
//               function must eventually be passed to a matching
//               pop_lens() call.
//
//               The new lens will not actually be made active for
//               rendering until the next call to prepare_lens().
//               This is a state-changing optimization.
////////////////////////////////////////////////////////////////////
INLINE LensStack GraphicsStateGuardian::
push_lens(const Lens *lens) {
  LensStack old;
  old._lens = _current_lens;
  old._stack_level = _lens_stack_level;
  _lens_stack_level++;
  _current_lens = lens;
  return old;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::pop_lens
//       Access: Public
//  Description: Restores the lens previously in effect, before the
//               matching call to push_lens().
//
//               The newly-restored lens will not actually be made
//               active for rendering until the next call to
//               prepare_lens().  This is a state-changing
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
pop_lens(LensStack &node) {
  nassertv(_lens_stack_level > 0);
  _lens_stack_level--;
  nassertv(node._stack_level == _lens_stack_level);
  _current_lens = node._lens;
  node._stack_level = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_lens
//       Access: Public
//  Description: Sets a new lens for rendering without bothering to
//               push or pop.  This replaces the lens most recently
//               pushed, if any.  There is no need to call
//               prepare_lens() following this call.
//
//               The return value is true if the lens is acceptable,
//               false if it is not.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsStateGuardian::
set_lens(const Lens *lens) {
  _current_lens = lens;
  return prepare_lens();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_coordinate_system
//       Access: Public
//  Description: Changes the coordinate system in effect on this
//               particular gsg.  Normally, this will be the default
//               coordinate system, but it might be set differently at
//               runtime.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_coordinate_system(CoordinateSystem cs) {
  _coordinate_system = cs;
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
//     Function: GraphicsStateGuardian::get_light
//       Access: Protected
//  Description: Returns the Light object that is bound to the
//               indicated id, or NULL if no Light is bound.
////////////////////////////////////////////////////////////////////
INLINE Light *GraphicsStateGuardian::
get_light(int light_id) const {
  nassertr(light_id >= 0 && light_id < (int)_light_info.size(), (Light *)NULL);
  return _light_info[light_id]._light;
}
