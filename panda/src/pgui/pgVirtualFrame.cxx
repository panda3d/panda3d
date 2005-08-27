// Filename: pgVirtualFrame.cxx
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

#include "pgVirtualFrame.h"
#include "clipPlaneAttrib.h"
#include "planeNode.h"
#include "sceneGraphReducer.h"

TypeHandle PGVirtualFrame::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGVirtualFrame::
PGVirtualFrame(const string &name) : PGItem(name)
{
  _has_clip_frame = false;
  _clip_frame.set(0.0f, 0.0f, 0.0f, 0.0f);

  setup_child_nodes();
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGVirtualFrame::
~PGVirtualFrame() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
PGVirtualFrame::
PGVirtualFrame(const PGVirtualFrame &copy) :
  PGItem(copy),
  _has_clip_frame(copy._has_clip_frame),
  _clip_frame(copy._clip_frame)
{
  setup_child_nodes();

  // Reassign the clip planes according to the clip frame.
  if (_has_clip_frame) {
    set_clip_frame(_clip_frame);
  } else {
    clear_clip_frame();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *PGVirtualFrame::
make_copy() const {
  return new PGVirtualFrame(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::r_copy_children
//       Access: Protected, Virtual
//  Description: This is called by r_copy_subgraph(); the copy has
//               already been made of this particular node (and this
//               is the copy); this function's job is to copy all of
//               the children from the original.
//
//               Note that it includes the parameter inst_map, which
//               is a map type, and is not (and cannot be) exported
//               from PANDA.DLL.  Thus, any derivative of PandaNode
//               that is not also a member of PANDA.DLL *cannot*
//               access this map, and probably should not even
//               override this function.
////////////////////////////////////////////////////////////////////
void PGVirtualFrame::
r_copy_children(const PandaNode *from, PandaNode::InstanceMap &inst_map) {
  PandaNode::r_copy_children(from, inst_map);

  // Reassign the canvas_node to point to the new copy, if it's there.
  const PGVirtualFrame *from_frame = DCAST(PGVirtualFrame, from);
  PandaNode *from_canvas_node = from_frame->get_canvas_node();
  PandaNode *from_clip_plane_node = from_frame->get_clip_plane_node();

  InstanceMap::const_iterator ci;
  ci = inst_map.find(from_canvas_node);
  if (ci != inst_map.end()) {
    remove_child(_canvas_node);
    _canvas_node = DCAST(ModelNode, (*ci).second);
  }

  ci = inst_map.find(from_clip_plane_node);
  if (ci != inst_map.end()) {
    remove_child(_clip_plane_node);
    _clip_plane_node = (*ci).second;
  }

  // Reassign the clip planes according to the clip frame.
  if (_has_clip_frame) {
    set_clip_frame(_clip_frame);
  } else {
    clear_clip_frame();
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::setup
//       Access: Published
//  Description: Creates a PGVirtualFrame with the indicated 
//               dimensions.
////////////////////////////////////////////////////////////////////
void PGVirtualFrame::
setup(float width, float height) {
  set_state(0);
  clear_state_def(0);

  set_frame(0, width, 0, height);

  float bevel = 0.05f;

  PGFrameStyle style;
  style.set_width(bevel, bevel);

  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
  style.set_type(PGFrameStyle::T_bevel_out);
  set_frame_style(0, style);

  set_clip_frame(bevel, width - 2 * bevel,
                 bevel, height - 2 * bevel);
}

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
void PGVirtualFrame::
set_clip_frame(const LVecBase4f &frame) {
  if (!_has_clip_frame || _clip_frame != frame) {
    _has_clip_frame = true;
    _clip_frame = frame;
    
    const LVector3f r = LVector3f::right();
    const LVector3f u = LVector3f::up();
    
    PT(PlaneNode) left_clip = 
      new PlaneNode("left_clip", Planef(r, r * _clip_frame[0]));
    PT(PlaneNode) right_clip = 
      new PlaneNode("right_clip", Planef(-r, r * _clip_frame[1]));
    
    PT(PlaneNode) bottom_clip = 
      new PlaneNode("bottom_clip", Planef(u, u * _clip_frame[2]));
    PT(PlaneNode) top_clip = 
      new PlaneNode("top_clip", Planef(-u, u * _clip_frame[3]));
    
    _clip_plane_node->remove_all_children();
    _clip_plane_node->add_child(left_clip);
    _clip_plane_node->add_child(right_clip);
    _clip_plane_node->add_child(bottom_clip);
    _clip_plane_node->add_child(top_clip);
    
    CPT(ClipPlaneAttrib) plane_attrib = DCAST(ClipPlaneAttrib, ClipPlaneAttrib::make());
    plane_attrib = DCAST(ClipPlaneAttrib, plane_attrib->add_on_plane(NodePath::any_path(left_clip)));
    plane_attrib = DCAST(ClipPlaneAttrib, plane_attrib->add_on_plane(NodePath::any_path(right_clip)));
    plane_attrib = DCAST(ClipPlaneAttrib, plane_attrib->add_on_plane(NodePath::any_path(bottom_clip)));
    plane_attrib = DCAST(ClipPlaneAttrib, plane_attrib->add_on_plane(NodePath::any_path(top_clip)));
    
    _canvas_node->set_attrib(plane_attrib);
    clip_frame_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::clear_clip_frame
//       Access: Published
//  Description: Removes the clip frame from the item.  This
//               disables clipping.
////////////////////////////////////////////////////////////////////
void PGVirtualFrame::
clear_clip_frame() {
  if (_has_clip_frame) {
    _has_clip_frame = false;
    
    _canvas_node->clear_attrib(ClipPlaneAttrib::get_class_type());
    clip_frame_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::clip_frame_changed
//       Access: Protected, Virtual
//  Description: Called when the user changes the clip_frame size.
////////////////////////////////////////////////////////////////////
void PGVirtualFrame::
clip_frame_changed() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGVirtualFrame::setup_child_nodes
//       Access: Private
//  Description: Creates the special canvas_node and clip_plane_node
//               for this object.
////////////////////////////////////////////////////////////////////
void PGVirtualFrame::
setup_child_nodes() {
  _canvas_node = new ModelNode("canvas");
  _canvas_node->set_preserve_transform(ModelNode::PT_local);
  // We have to preserve the clip plane attribute.
  _canvas_node->set_preserve_attributes(SceneGraphReducer::TT_other);
  add_child(_canvas_node);

  _clip_plane_node = new PandaNode("clip_planes");
  add_child(_clip_plane_node);
}
