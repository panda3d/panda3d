// Filename: eggNode.cxx
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggNode.h"
#include "eggGroupNode.h"
#include "config_egg.h"
#include "eggTextureCollection.h"

#include <algorithm>

TypeHandle EggNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggNode::apply_texmats
//       Access: Public
//  Description: Applies the texture matrices to the UV's of the
//               vertices that reference them, and then removes the
//               texture matrices from the textures themselves.
////////////////////////////////////////////////////////////////////
void EggNode::
apply_texmats() {
  EggTextureCollection textures;
  textures.find_used_textures(this);
  r_apply_texmats(textures);
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::determine_alpha_mode
//       Access: Public, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this node that has an alpha_mode other than
//               AM_unspecified.  Returns a valid EggRenderMode pointer
//               if one is found, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggNode::
determine_alpha_mode() {
  if (_parent == (EggGroupNode *)NULL) {
    // Too bad; we're done.
    return (EggRenderMode *)NULL;
  }
  return _parent->determine_alpha_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::determine_depth_write_mode
//       Access: Public, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this node that has a depth_write_mode other than
//               DWM_unspecified.  Returns a valid EggRenderMode pointer
//               if one is found, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggNode::
determine_depth_write_mode() {
  if (_parent == (EggGroupNode *)NULL) {
    // Too bad; we're done.
    return (EggRenderMode *)NULL;
  }
  return _parent->determine_depth_write_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::determine_depth_test_mode
//       Access: Public, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this node that has a depth_test_mode other than
//               DTM_unspecified.  Returns a valid EggRenderMode pointer
//               if one is found, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggNode::
determine_depth_test_mode() {
  if (_parent == (EggGroupNode *)NULL) {
    // Too bad; we're done.
    return (EggRenderMode *)NULL;
  }
  return _parent->determine_depth_test_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::determine_draw_order
//       Access: Public, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this node that has a draw_order specified.
//               Returns a valid EggRenderMode pointer if one is found,
//               or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggNode::
determine_draw_order() {
  if (_parent == (EggGroupNode *)NULL) {
    // Too bad; we're done.
    return (EggRenderMode *)NULL;
  }
  return _parent->determine_draw_order();
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::determine_bin
//       Access: Public, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this node that has a bin specified.  Returns a
//               valid EggRenderMode pointer if one is found, or NULL
//               otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggNode::
determine_bin() {
  if (_parent == (EggGroupNode *)NULL) {
    // Too bad; we're done.
    return (EggRenderMode *)NULL;
  }
  return _parent->determine_bin();
}

#ifndef NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: EggNode::test_under_integrity
//       Access: Public
//  Description: Recursively checks the integrity of the _under_flags,
//               _parent, and _depth members of this node and all of
//               its ancestors.
////////////////////////////////////////////////////////////////////
void EggNode::
test_under_integrity() const {
  if (_parent == NULL) {
    // If we have no parent, everything should be zero.
    nassertv(_depth == 0);
    nassertv(_under_flags == 0);
  } else {
    // Otherwise, make sure we're consistent with our parent.
    _parent->test_ref_count_integrity();

    nassertv(_depth == _parent->_depth + 1);

    // We can't perform too much checking on the under_flags, since we
    // don't know which bits should have been added for this node.
    // We'll verify that at least we didn't accidentally take some
    // bits away.
    nassertv((_under_flags & _parent->_under_flags) == _parent->_under_flags);

    // Make sure we're mentioned in our parent's children list.
    EggGroupNode::iterator ci;
    ci = find(_parent->begin(), _parent->end(), this);
    nassertv(ci != _parent->end());

    // Now recurse up our parent.
    _parent->test_under_integrity();
  }
}

#endif  // NDEBUG


////////////////////////////////////////////////////////////////////
//     Function: EggNode::update_under
//       Access: Protected, Virtual
//  Description: This function is called from within EggGroupNode
//               whenever the parentage of the node has changed.  It
//               should update the depth and under_instance flags
//               accordingly.
//
//               depth_offset is the difference between the old depth
//               value and the new value.  It should be consistent
//               with the supplied depth value.  If it is not, we have
//               some error.
////////////////////////////////////////////////////////////////////
void EggNode::
update_under(int depth_offset) {
  int depth;
  if (_parent == NULL) {
    depth = 0;
    _under_flags = 0;
    _vertex_frame = NULL;
    _node_frame = NULL;
    _vertex_frame_inv = NULL;
    _node_frame_inv = NULL;
    _vertex_to_node = NULL;
  } else {
    depth = _parent->_depth + 1;
    _under_flags = _parent->_under_flags;
    _vertex_frame = _parent->_vertex_frame;
    _node_frame = _parent->_node_frame;
    _vertex_frame_inv = _parent->_vertex_frame_inv;
    _node_frame_inv = _parent->_node_frame_inv;
    _vertex_to_node = _parent->_vertex_to_node;
  }

  if (depth - _depth != depth_offset) {
    egg_cat.error() << "Cycle in egg graph or invalid egg pointer!\n";
    return;
  }
  _depth = depth;

  adjust_under();
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::adjust_under
//       Access: Protected, Virtual
//  Description: This is called within update_under() after all the
//               various under settings have been inherited directly
//               from the parent node.  It is responsible for
//               adjusting these settings to reflect states local to
//               the current node; for instance, an <Instance> node
//               will force the UF_under_instance bit on.
////////////////////////////////////////////////////////////////////
void EggNode::
adjust_under() {
}


////////////////////////////////////////////////////////////////////
//     Function: EggNode::r_transform
//       Access: Protected, Virtual
//  Description: This is called from within the egg code by
//               transform().  It applies a transformation matrix
//               to the current node in some sensible way, then
//               continues down the tree.
//
//               The first matrix is the transformation to apply; the
//               second is its inverse.  The third parameter is the
//               coordinate system we are changing to, or CS_default
//               if we are not changing coordinate systems.
////////////////////////////////////////////////////////////////////
void EggNode::
r_transform(const LMatrix4d &, const LMatrix4d &, CoordinateSystem) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::r_transform_vertices
//       Access: Protected, Virtual
//  Description: This is called from within the egg code by
//               transform_vertices_only()().  It applies a
//               transformation matrix to the current node in some
//               sensible way (if the current node is a vertex pool
//               with vertices), then continues down the tree.
////////////////////////////////////////////////////////////////////
void EggNode::
r_transform_vertices(const LMatrix4d &) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::r_mark_coordsys
//       Access: Protected, Virtual
//  Description: This is only called immediately after loading an egg
//               file from disk, to propagate the value found in the
//               CoordinateSystem entry (or the default Y-up
//               coordinate system) to all nodes that care about what
//               the coordinate system is.
////////////////////////////////////////////////////////////////////
void EggNode::
r_mark_coordsys(CoordinateSystem) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::r_flatten_transforms
//       Access: Protected, Virtual
//  Description: The recursive implementation of flatten_transforms().
////////////////////////////////////////////////////////////////////
void EggNode::
r_flatten_transforms() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggNode::r_apply_texmats
//       Access: Protected, Virtual
//  Description: The recursive implementation of apply_texmats().
////////////////////////////////////////////////////////////////////
void EggNode::
r_apply_texmats(EggTextureCollection &textures) {
}
