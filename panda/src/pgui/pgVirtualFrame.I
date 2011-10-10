// Filename: pgVirtualFrame.I
// Created by:  drose (17Aug05)
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
//     Function: PGVirtualFrame::set_clip_frame
//       Access: Published
//  Description: Sets the bounding rectangle of the clip frame.
//               This is the size of the small window through which we
//               can see the virtual canvas.  Normally, this is the
//               same size as the actual frame or smaller (typically
//               it is smaller by the size of the bevel, or to make
//               room for scroll bars).
////////////////////////////////////////////////////////////////////
INLINE void PGVirtualFrame::
set_clip_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  set_clip_frame(LVecBase4(left, right, bottom, top));
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::get_clip_frame
//       Access: Published
//  Description: Returns the bounding rectangle of the clip frame.
//               See set_clip_frame().  If has_clip_frame() is
//               false, this returns the item's actual frame.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4 &PGVirtualFrame::
get_clip_frame() const {
  LightReMutexHolder holder(_lock);
  return _has_clip_frame ? _clip_frame : get_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::has_clip_frame
//       Access: Published
//  Description: Returns true if the clip frame has been set; see
//               set_clip_frame().  If it has not been set, objects in
//               the virtual frame will not be clipped.
////////////////////////////////////////////////////////////////////
INLINE bool PGVirtualFrame::
has_clip_frame() const {
  LightReMutexHolder holder(_lock);
  return _has_clip_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::set_canvas_transform
//       Access: Published
//  Description: Changes the transform of the virtual canvas.  This
//               transform is applied to all child nodes of the
//               canvas_node.
////////////////////////////////////////////////////////////////////
INLINE void PGVirtualFrame::
set_canvas_transform(const TransformState *transform) {
  LightReMutexHolder holder(_lock);
  _canvas_node->set_transform(transform);
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::get_canvas_transform
//       Access: Published
//  Description: Returns the transform of the virtual canvas.  This
//               transform is applied to all child nodes of the
//               canvas_node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *PGVirtualFrame::
get_canvas_transform() const {
  LightReMutexHolder holder(_lock);
  return _canvas_node->get_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::get_canvas_node
//       Access: Published
//  Description: Returns the special node that holds all of the
//               children that appear in the virtual canvas.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PGVirtualFrame::
get_canvas_node() const {
  LightReMutexHolder holder(_lock);
  return _canvas_node;
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::get_canvas_parent
//       Access: Published
//  Description: Returns the parent node of the canvas_node.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PGVirtualFrame::
get_canvas_parent() const {
  LightReMutexHolder holder(_lock);
  return _canvas_parent;
}
