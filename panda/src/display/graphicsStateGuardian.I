// Filename: graphicsStateGuardian.I
// Created by:  drose (24Sep99)
// 
////////////////////////////////////////////////////////////////////

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::set_render_traverser
//       Access: Public
//  Description: Sets the traverser that will be used to render the
//               scene graph.  If this is unset, nothing will be
//               rendered.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsStateGuardian::
set_render_traverser(RenderTraverser *rt) {
  _render_traverser = rt;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_render_traverser
//       Access: Public
//  Description: Returns the traverser that will be used to render the
//               scene graph.
////////////////////////////////////////////////////////////////////
INLINE RenderTraverser *GraphicsStateGuardian::
get_render_traverser() const {
  return _render_traverser;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_state
//       Access: Public
//  Description: Returns the current state of the GSG.  This state may
//               be saved and used to restore the GSG to its current
//               state later; however, this save/restore operation
//               should be used with caution, as it could grow to be
//               expensive.
////////////////////////////////////////////////////////////////////
INLINE const NodeAttributes &GraphicsStateGuardian::
get_state() const {
  return _state;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_current_projection_node
//       Access: Public
//  Description: Returns the node currently being used as the
//               projection node (i.e. the camera) for this scene, as
//               set by the last call to render_subgraph().
////////////////////////////////////////////////////////////////////
INLINE ProjectionNode *GraphicsStateGuardian::
get_current_projection_node(void) const { 
  return _current_projection_node;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::get_current_root_node
//       Access: Public
//  Description: Returns the root node of the entire scene graph (not
//               the current subgraph) currently being rendered, as
//               set by the last call to render_scene().
////////////////////////////////////////////////////////////////////
INLINE const Node *GraphicsStateGuardian::
get_current_root_node(void) const {
  return _current_root_node;
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
INLINE CPT(DisplayRegion) GraphicsStateGuardian::
get_current_display_region(void) const {
  return _current_display_region;
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
push_display_region(CPT(DisplayRegion) dr) {
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
                  CPT(DisplayRegion) dr) {
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
