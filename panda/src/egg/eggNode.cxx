/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNode.cxx
 * @author drose
 * @date 1999-01-16
 */

#include "eggNode.h"
#include "eggGroupNode.h"
#include "lightMutexHolder.h"
#include "config_egg.h"
#include "eggTextureCollection.h"
#include "dcast.h"

#include <algorithm>

extern int eggyyparse();
#include "parserDefs.h"
#include "lexerDefs.h"

TypeHandle EggNode::_type_handle;


/**
 * Rename by stripping out the prefix
 */
int EggNode::
rename_node(vector_string strip_prefix) {
  int num_renamed = 0;
  for (unsigned int ni = 0; ni < strip_prefix.size(); ++ni) {
    std::string axe_name = strip_prefix[ni];
    if (this->get_name().substr(0, axe_name.size()) == axe_name) {
      std::string new_name = this->get_name().substr(axe_name.size());
      // cout << "renaming " << this->get_name() << "->" << new_name << endl;
      this->set_name(new_name);
      num_renamed += 1;
    }
  }
  return num_renamed;
}

/**
 * Applies the texture matrices to the UV's of the vertices that reference
 * them, and then removes the texture matrices from the textures themselves.
 */
void EggNode::
apply_texmats() {
  EggTextureCollection textures;
  textures.find_used_textures(this);
  r_apply_texmats(textures);
}

/**
 * Returns true if this particular node represents a <Joint> entry or not.
 * This is a handy thing to know since Joints are sorted to the end of their
 * sibling list when writing an egg file.  See EggGroupNode::write().
 */
bool EggNode::
is_joint() const {
  return false;
}

/**
 * Returns true if this node represents a table of animation transformation
 * data, false otherwise.
 */
bool EggNode::
is_anim_matrix() const {
  return false;
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has an alpha_mode
 * other than AM_unspecified.  Returns a valid EggRenderMode pointer if one is
 * found, or NULL otherwise.
 */
EggRenderMode *EggNode::
determine_alpha_mode() {
  if (_parent == nullptr) {
    // Too bad; we're done.
    return nullptr;
  }
  return _parent->determine_alpha_mode();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has a
 * depth_write_mode other than DWM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggNode::
determine_depth_write_mode() {
  if (_parent == nullptr) {
    // Too bad; we're done.
    return nullptr;
  }
  return _parent->determine_depth_write_mode();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has a
 * depth_test_mode other than DTM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggNode::
determine_depth_test_mode() {
  if (_parent == nullptr) {
    // Too bad; we're done.
    return nullptr;
  }
  return _parent->determine_depth_test_mode();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has a
 * visibility_mode other than VM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggNode::
determine_visibility_mode() {
  if (_parent == nullptr) {
    // Too bad; we're done.
    return nullptr;
  }
  return _parent->determine_visibility_mode();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has a depth_offset
 * specified.  Returns a valid EggRenderMode pointer if one is found, or NULL
 * otherwise.
 */
EggRenderMode *EggNode::
determine_depth_offset() {
  if (_parent == nullptr) {
    // Too bad; we're done.
    return nullptr;
  }
  return _parent->determine_depth_offset();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has a draw_order
 * specified.  Returns a valid EggRenderMode pointer if one is found, or NULL
 * otherwise.
 */
EggRenderMode *EggNode::
determine_draw_order() {
  if (_parent == nullptr) {
    // Too bad; we're done.
    return nullptr;
  }
  return _parent->determine_draw_order();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has a bin specified.
 * Returns a valid EggRenderMode pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggNode::
determine_bin() {
  if (_parent == nullptr) {
    // Too bad; we're done.
    return nullptr;
  }
  return _parent->determine_bin();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup at this level or above
 * that has the "indexed" scalar set.  Returns the value of the indexed scalar
 * if it is found, or false if it is not.
 *
 * In other words, returns true if the "indexed" flag is in effect for the
 * indicated node, false otherwise.
 */
bool EggNode::
determine_indexed() {
  if (_parent == nullptr) {
    // Too bad; we're done.
    return false;
  }
  return _parent->determine_indexed();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup at this level or above
 * that has the "decal" flag set.  Returns the value of the decal flag if it
 * is found, or false if it is not.
 *
 * In other words, returns true if the "decal" flag is in effect for the
 * indicated node, false otherwise.
 */
bool EggNode::
determine_decal() {
  if (_parent == nullptr) {
    // Too bad; we're done.
    return false;
  }
  return _parent->determine_decal();
}


/**
 * Parses the egg syntax given in the indicate string as if it had been read
 * from the egg file within this object's definition.  Updates the object
 * accordingly.  Returns true if successful, false if there was some parse
 * error or if the object does not support this functionality.
 */
bool EggNode::
parse_egg(const std::string &egg_syntax) {
  EggGroupNode *group = get_parent();
  if (is_of_type(EggGroupNode::get_class_type())) {
    DCAST_INTO_R(group, this, false);
  }

  std::istringstream in(egg_syntax);

  LightMutexHolder holder(egg_lock);

  egg_init_parser(in, "", this, group);

  if (!egg_start_parse_body()) {
    egg_cleanup_parser();
    return false;
  }

  eggyyparse();
  egg_cleanup_parser();

  return (egg_error_count() == 0);
}

#ifdef _DEBUG

/**
 * Recursively checks the integrity of the _under_flags, _parent, and _depth
 * members of this node and all of its ancestors.
 */
void EggNode::
test_under_integrity() const {
  if (_parent == nullptr) {
    // If we have no parent, everything should be zero.
    nassertv(_depth == 0);
    nassertv(_under_flags == 0);
  } else {
    // Otherwise, make sure we're consistent with our parent.
    _parent->test_ref_count_integrity();

    nassertv(_depth == _parent->_depth + 1);

    // We can't perform too much checking on the under_flags, since we don't
    // know which bits should have been added for this node.  We'll verify
    // that at least we didn't accidentally take some bits away.
    nassertv((_under_flags & _parent->_under_flags) == _parent->_under_flags);

    // Make sure we're mentioned in our parent's children list.
    EggGroupNode::iterator ci;
    ci = find(_parent->begin(), _parent->end(), this);
    nassertv(ci != _parent->end());

    // Now recurse up our parent.
    _parent->test_under_integrity();
  }
}

#endif  // _DEBUG


/**
 * This function is called within parse_egg().  It should call the appropriate
 * function on the lexer to initialize the parser into the state associated
 * with this object.  If the object cannot be parsed into directly, it should
 * return false.
 */
bool EggNode::
egg_start_parse_body() {
  return false;
}

/**
 * This function is called from within EggGroupNode whenever the parentage of
 * the node has changed.  It should update the depth and under_instance flags
 * accordingly.
 *
 * depth_offset is the difference between the old depth value and the new
 * value.  It should be consistent with the supplied depth value.  If it is
 * not, we have some error.
 */
void EggNode::
update_under(int depth_offset) {
  int depth;
  if (_parent == nullptr) {
    depth = 0;
    _under_flags = 0;
    _vertex_frame = nullptr;
    _node_frame = nullptr;
    _vertex_frame_inv = nullptr;
    _node_frame_inv = nullptr;
    _vertex_to_node = nullptr;
    _node_to_vertex = nullptr;
  } else {
    _parent->test_ref_count_integrity();
    depth = _parent->_depth + 1;
    _under_flags = _parent->_under_flags;
    _vertex_frame = _parent->_vertex_frame;
    _node_frame = _parent->_node_frame;
    _vertex_frame_inv = _parent->_vertex_frame_inv;
    _node_frame_inv = _parent->_node_frame_inv;
    _vertex_to_node = _parent->_vertex_to_node;
    _node_to_vertex = _parent->_node_to_vertex;
  }

  if (depth - _depth != depth_offset) {
    egg_cat.error() << "Cycle in egg graph or invalid egg pointer!\n";
    return;
  }
  _depth = depth;

  adjust_under();
}

/**
 * This is called within update_under() after all the various under settings
 * have been inherited directly from the parent node.  It is responsible for
 * adjusting these settings to reflect states local to the current node; for
 * instance, an <Instance> node will force the UF_under_instance bit on.
 */
void EggNode::
adjust_under() {
}

/**
 * Returns true if there are any primitives (e.g.  polygons) defined within
 * this group or below, false otherwise.
 */
bool EggNode::
has_primitives() const {
  return false;
}

/**
 * Returns true if there are any primitives (e.g.  polygons) defined within
 * this group or below, but the search does not include nested joints.
 */
bool EggNode::
joint_has_primitives() const {
  return false;
}

/**
 * Returns true if any of the primitives (e.g.  polygons) defined within this
 * group or below have either face or vertex normals defined, false otherwise.
 */
bool EggNode::
has_normals() const {
  return false;
}


/**
 * This is called from within the egg code by transform().  It applies a
 * transformation matrix to the current node in some sensible way, then
 * continues down the tree.
 *
 * The first matrix is the transformation to apply; the second is its inverse.
 * The third parameter is the coordinate system we are changing to, or
 * CS_default if we are not changing coordinate systems.
 */
void EggNode::
r_transform(const LMatrix4d &, const LMatrix4d &, CoordinateSystem) {
}

/**
 * This is called from within the egg code by transform_vertices_only()().  It
 * applies a transformation matrix to the current node in some sensible way
 * (if the current node is a vertex pool with vertices), then continues down
 * the tree.
 */
void EggNode::
r_transform_vertices(const LMatrix4d &) {
}

/**
 * This is only called immediately after loading an egg file from disk, to
 * propagate the value found in the CoordinateSystem entry (or the default
 * Y-up coordinate system) to all nodes that care about what the coordinate
 * system is.
 */
void EggNode::
r_mark_coordsys(CoordinateSystem) {
}

/**
 * The recursive implementation of flatten_transforms().
 */
void EggNode::
r_flatten_transforms() {
}

/**
 * The recursive implementation of apply_texmats().
 */
void EggNode::
r_apply_texmats(EggTextureCollection &textures) {
}
