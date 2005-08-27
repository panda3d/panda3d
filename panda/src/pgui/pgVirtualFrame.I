// Filename: pgVirtualFrame.I
// Created by:  drose (17Aug05)
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
set_clip_frame(float left, float right, float bottom, float top) {
  set_clip_frame(LVecBase4f(left, right, bottom, top));
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::get_clip_frame
//       Access: Published
//  Description: Returns the bounding rectangle of the clip frame.
//               See set_clip_frame().  If has_clip_frame() is
//               false, this returns the item's actual frame.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4f &PGVirtualFrame::
get_clip_frame() const {
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
  return _canvas_node;
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::get_clip_plane_node
//       Access: Published
//  Description: Returns the special node that holds all of PlaneNodes
//               that are used for applying the clip planes to the
//               canvas_node.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PGVirtualFrame::
get_clip_plane_node() const {
  return _clip_plane_node;
}
