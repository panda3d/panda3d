/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJoint.cxx
 * @author drose
 * @date 1999-02-23
 */

#include "characterJoint.h"
#include "config_char.h"
#include "jointVertexTransform.h"
#include "characterJointEffect.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle CharacterJoint::_type_handle;

/**
 * For internal use only.
 */
CharacterJoint::
CharacterJoint() :
  _character(nullptr)
{
}

/**
 *
 */
CharacterJoint::
CharacterJoint(const CharacterJoint &copy) :
  MovingPartMatrix(copy),
  _character(nullptr),
  _net_transform(copy._net_transform),
  _initial_net_transform_inverse(copy._initial_net_transform_inverse),
  _skinning_matrix(copy._skinning_matrix)
{
  // We don't copy the sets of transform nodes.
}

/**
 *
 */
CharacterJoint::
CharacterJoint(Character *character,
               PartBundle *root, PartGroup *parent, const std::string &name,
               const LMatrix4 &default_value) :
  MovingPartMatrix(parent, name, default_value),
  _character(character)
{
  Thread *current_thread = Thread::get_current_thread();

  // Now that we've constructed and we're in the tree, let's call
  // update_internals() to get our _net_transform set properly.
  update_internals(root, parent, true, false, current_thread);

  // And then compute its inverse.  This is needed to track changes in
  // _net_transform as the joint moves, so we can recompute _skinning_matrix,
  // which maps vertices from their initial positions to their animated
  // positions.
  _initial_net_transform_inverse = invert(_net_transform);
  _skinning_matrix = LMatrix4::ident_mat();
}

/**
 *
 */
CharacterJoint::
~CharacterJoint() {
  nassertv(_vertex_transforms.empty());
  nassertv(_character == nullptr);
}

/**
 * Returns true if this part is a CharacterJoint, false otherwise.  This is a
 * tiny optimization over is_of_type(CharacterType::get_class_type()).
 */
bool CharacterJoint::
is_character_joint() const {
  return true;
}

/**
 * Allocates and returns a new copy of the node.  Children are not copied, but
 * see copy_subgraph().
 */
PartGroup *CharacterJoint::
make_copy() const {
  return new CharacterJoint(*this);
}

/**
 * This is called by do_update() whenever the part or some ancestor has
 * changed values.  It is a hook for derived classes to update whatever cache
 * they may have that depends on these.
 *
 * The return value is true if the part has changed as a result of the update,
 * or false otherwise.
 *
 * In the case of a CharacterJoint, of course, it means to recompute the joint
 * angles and associated transforms for this particular joint.
 */
bool CharacterJoint::
update_internals(PartBundle *root, PartGroup *parent, bool self_changed,
                 bool parent_changed, Thread *current_thread) {
  nassertr(parent != nullptr, false);

  bool net_changed = false;
  if (parent->is_character_joint()) {
    // The joint is not a toplevel joint; its parent therefore affects its net
    // transform.
    if (parent_changed || self_changed) {
      CharacterJoint *parent_joint = DCAST(CharacterJoint, parent);

      _net_transform = _value * parent_joint->_net_transform;
      net_changed = true;
    }

  } else {
    // The joint is a toplevel joint, so therefore it gets its root transform
    // from the bundle.
    if (self_changed) {
      _net_transform = _value * root->get_root_xform();
      net_changed = true;
    }
  }

  if (net_changed) {
    if (!_net_transform_nodes.empty()) {
      CPT(TransformState) t = TransformState::make_mat(_net_transform);

      NodeList::iterator ai;
      for (ai = _net_transform_nodes.begin();
           ai != _net_transform_nodes.end();
           ++ai) {
        PandaNode *node = *ai;
        node->set_transform(t, current_thread);
      }
    }

    // Recompute the transform used by any vertices animated by this joint.
    _skinning_matrix = _initial_net_transform_inverse * _net_transform;

    // Also tell our related JointVertexTransforms that we've changed their
    // underlying matrix.
    VertexTransforms::iterator vti;
    for (vti = _vertex_transforms.begin(); vti != _vertex_transforms.end(); ++vti) {
      (*vti)->mark_modified(current_thread);
    }
  }

  if (self_changed && !_local_transform_nodes.empty()) {
    CPT(TransformState) t = TransformState::make_mat(_value);

    NodeList::iterator ai;
    for (ai = _local_transform_nodes.begin();
         ai != _local_transform_nodes.end();
         ++ai) {
      PandaNode *node = *ai;
      node->set_transform(t, current_thread);
    }
  }

  return self_changed || net_changed;
}

/**
 * Called by PartBundle::xform(), this indicates the indicated transform is
 * being applied to the root joint.
 */
void CharacterJoint::
do_xform(const LMatrix4 &mat, const LMatrix4 &inv_mat) {
  _initial_net_transform_inverse = inv_mat * _initial_net_transform_inverse;

  MovingPartMatrix::do_xform(mat, inv_mat);
}

/**
 * Recursively initializes the joint for IK, calculating the current net
 * position and lengths.  Returns true if there were any effectors under this
 * node, false otherwise.
 */
bool CharacterJoint::
r_init_ik(const LPoint3 &parent_pos) {
  _ik_pos = _net_transform.get_row3(3);
  _ik_length = (_ik_pos - parent_pos).length();

  if (PartGroup::r_init_ik(_ik_pos)) {
    _ik_weight = 1;
    return true;
  } else {
    _ik_weight = 0;
    return false;
  }
}

/**
 * Executes a forward IK pass on the given points (which are set up by
 * r_setup_ik_points).
 */
void CharacterJoint::
r_forward_ik(const LPoint3 &parent_pos) {
  LVector3 delta = _ik_pos - parent_pos;
  PN_stdfloat dist = delta.length();

  if (!IS_NEARLY_ZERO(dist)) {
    LVector3 dir = delta / dist;

    _ik_pos = parent_pos + dir * _ik_length;
  }
  else {
    // It has length 0, so we just inherit the parent position.
    _ik_pos = parent_pos;
  }

  for (PartGroup *child : _children) {
    child->r_forward_ik(_ik_pos);
  }
}

/**
 * Executes a reverse IK pass on the given points (which are set up by
 * r_setup_ik_points).  Returns true if there were any effectors under this
 * joint, in which case the new position of this joint is stored in out_pos.
 */
bool CharacterJoint::
r_reverse_ik(LPoint3 &parent_pos) {
  LPoint3 effective_pos(0);
  int num_effectors = 0;

  for (PartGroup *child : _children) {
    LPoint3 desired_pos = _ik_pos;
    if (!child->r_reverse_ik(desired_pos)) {
      // No effectors under this joint, so it's not of interest to the reverse
      // pass.
      continue;
    }

    effective_pos += desired_pos;
    ++num_effectors;
  }

  if (num_effectors == 0) {
    return false;
  }

  _ik_pos = effective_pos * (1.0 / num_effectors);

  PN_stdfloat base_length = _ik_length;
  if (IS_NEARLY_ZERO(base_length)) {
    parent_pos = _ik_pos;
  } else {
    LVector3 delta = _ik_pos - parent_pos;
    PN_stdfloat length = delta.length();
    LVector3 dir = delta / length;
    parent_pos = _ik_pos - dir * base_length;
  }

  return true;
}

/**
 * Recursively applies the IK position changes computed by r_forward_ik and
 * r_reverse_ik onto _net_transform.
 */
void CharacterJoint::
r_apply_ik(const LMatrix4 &parent_net_transform) {
  _net_transform = _value * parent_net_transform;

  // We can only apply one rotation to this joint, so when there are multiple
  // children, we have a problem.  What we do is figure out which children have
  // effectors under them (ik_weight > 0), and rotate to their average.
  LVector3 cur_vec(0, 0, 0);
  LVector3 new_vec(0, 0, 0);
  PN_stdfloat total_weight = 0;

  LMatrix4 inverse_xform;
  inverse_xform.invert_from(_net_transform);

  for (PartGroup *child : _children) {
    if (!child->is_character_joint()) {
      continue;
    }

    CharacterJoint *child_joint = (CharacterJoint *)child;
    //if (child_joint->_ik_weight == 0) {
    //  continue;
    //}

    // Get current position of joint relative to parent
    LPoint3 cur_pos = child_joint->_value.get_row3(3);
    cur_vec += cur_pos.normalized();

    // Get desired position relative to parent
    LPoint3 child_pos = child_joint->_ik_pos;
    LPoint3 rel_pos = inverse_xform.xform_point(child_pos);
    new_vec += rel_pos.normalized();

    total_weight += 1;
  }

  if (total_weight > 0) {
    PN_stdfloat factor = 1.0 / total_weight;
    cur_vec *= factor;
    new_vec *= factor;

    LVector3 w = cur_vec.cross(new_vec);
    if (w.length_squared() != 0) {
      w.normalize();
      PN_stdfloat angle = new_vec.signed_angle_deg(cur_vec, w);
      if (!IS_NEARLY_ZERO(angle)) {
        LMatrix3 r = LMatrix3::rotate_mat(-angle, w);

        _value.set_upper_3(r * _value.get_upper_3());
      }
    }
    else if (cur_vec.dot(new_vec) < 0) {
      // Exactly antiparallel.  We have an infinite number of axes around which we
      // could rotate to get the desired matrix.  Can we pick one arbitrarily?

      w = new_vec.cross(LVector3(1, 0, 0));
      if (IS_NEARLY_ZERO(w.length_squared())) {
        w = new_vec.cross(LVector3(0, 1, 0));
      }

      LMatrix3 r = LMatrix3::rotate_mat(-180, w);

      _value.set_upper_3(r * _value.get_upper_3());
    }
  }

  //TODO: don't duplicate all the logic from update_internals.
  _net_transform = _value * parent_net_transform;

  Thread *current_thread = Thread::get_current_thread();
  if (!_net_transform_nodes.empty()) {
    CPT(TransformState) t = TransformState::make_mat(_net_transform);

    NodeList::iterator ai;
    for (ai = _net_transform_nodes.begin();
         ai != _net_transform_nodes.end();
         ++ai) {
      PandaNode *node = *ai;
      node->set_transform(t, current_thread);
    }
  }

  _skinning_matrix = _initial_net_transform_inverse * _net_transform;

  // Also tell our related JointVertexTransforms that we've changed their
  // underlying matrix.
  VertexTransforms::iterator vti;
  for (vti = _vertex_transforms.begin(); vti != _vertex_transforms.end(); ++vti) {
    (*vti)->mark_modified(current_thread);
  }

  for (PartGroup *child : _children) {
    child->r_apply_ik(_net_transform);
  }
}

/**
 * Adds the indicated node to the list of nodes that will be updated each
 * frame with the joint's net transform from the root.  Returns true if the
 * node is successfully added, false if it had already been added.
 *
 * A CharacterJointEffect for this joint's Character will automatically be
 * added to the specified node.
 */
bool CharacterJoint::
add_net_transform(PandaNode *node) {
  if (_character != nullptr) {
    node->set_effect(CharacterJointEffect::make(_character));
  }
  CPT(TransformState) t = TransformState::make_mat(_net_transform);
  node->set_transform(t, Thread::get_current_thread());
  return _net_transform_nodes.insert(node).second;
}

/**
 * Removes the indicated node from the list of nodes that will be updated each
 * frame with the joint's net transform from the root.  Returns true if the
 * node is successfully removed, false if it was not on the list.
 *
 * If the node has a CharacterJointEffect that matches this joint's Character,
 * it will be cleared.
 */
bool CharacterJoint::
remove_net_transform(PandaNode *node) {
  CPT(RenderEffect) effect = node->get_effect(CharacterJointEffect::get_class_type());
  if (effect != nullptr &&
      DCAST(CharacterJointEffect, effect)->matches_character(_character)) {
    node->clear_effect(CharacterJointEffect::get_class_type());
  }

  return (_net_transform_nodes.erase(node) > 0);
}

/**
 * Returns true if the node is on the list of nodes that will be updated each
 * frame with the joint's net transform from the root, false otherwise.
 */
bool CharacterJoint::
has_net_transform(PandaNode *node) const {
  return (_net_transform_nodes.count(node) > 0);
}

/**
 * Removes all nodes from the list of nodes that will be updated each frame
 * with the joint's net transform from the root.
 */
void CharacterJoint::
clear_net_transforms() {
  NodeList::iterator ai;
  for (ai = _net_transform_nodes.begin();
       ai != _net_transform_nodes.end();
       ++ai) {
    PandaNode *node = *ai;

    CPT(RenderEffect) effect = node->get_effect(CharacterJointEffect::get_class_type());
    if (effect != nullptr &&
        DCAST(CharacterJointEffect, effect)->matches_character(_character)) {
      node->clear_effect(CharacterJointEffect::get_class_type());
    }
  }

  _net_transform_nodes.clear();
}

/**
 * Returns a list of the net transforms set for this node.  Note that this
 * returns a list of NodePaths, even though the net transforms are actually a
 * list of PandaNodes.
 */
NodePathCollection CharacterJoint::
get_net_transforms() {
  NodePathCollection npc;

  NodeList::iterator ai;
  for (ai = _net_transform_nodes.begin();
       ai != _net_transform_nodes.end();
       ++ai) {
    PandaNode *node = *ai;
    npc.add_path(NodePath::any_path(node));
  }

  return npc;
}

/**
 * Adds the indicated node to the list of nodes that will be updated each
 * frame with the joint's local transform from its parent.  Returns true if
 * the node is successfully added, false if it had already been added.
 *
 * The Character pointer should be the Character object that owns this joint;
 * this will be used to create a CharacterJointEffect for this node.  If it is
 * NULL, no such effect will be created.
 *
 * A CharacterJointEffect for this joint's Character will automatically be
 * added to the specified node.
 */
bool CharacterJoint::
add_local_transform(PandaNode *node) {
  if (_character != nullptr) {
    node->set_effect(CharacterJointEffect::make(_character));
  }
  CPT(TransformState) t = TransformState::make_mat(_value);
  node->set_transform(t, Thread::get_current_thread());
  return _local_transform_nodes.insert(node).second;
}

/**
 * Removes the indicated node from the list of nodes that will be updated each
 * frame with the joint's local transform from its parent.  Returns true if
 * the node is successfully removed, false if it was not on the list.
 *
 * If the node has a CharacterJointEffect that matches this joint's Character,
 * it will be cleared.
 */
bool CharacterJoint::
remove_local_transform(PandaNode *node) {
  CPT(RenderEffect) effect = node->get_effect(CharacterJointEffect::get_class_type());
  if (effect != nullptr &&
      DCAST(CharacterJointEffect, effect)->matches_character(_character)) {
    node->clear_effect(CharacterJointEffect::get_class_type());
  }

  return (_local_transform_nodes.erase(node) > 0);
}

/**
 * Returns true if the node is on the list of nodes that will be updated each
 * frame with the joint's local transform from its parent, false otherwise.
 */
bool CharacterJoint::
has_local_transform(PandaNode *node) const {
  return (_local_transform_nodes.count(node) > 0);
}

/**
 * Removes all nodes from the list of nodes that will be updated each frame
 * with the joint's local transform from its parent.
 */
void CharacterJoint::
clear_local_transforms() {
  NodeList::iterator ai;
  for (ai = _local_transform_nodes.begin();
       ai != _local_transform_nodes.end();
       ++ai) {
    PandaNode *node = *ai;

    CPT(RenderEffect) effect = node->get_effect(CharacterJointEffect::get_class_type());
    if (effect != nullptr &&
        DCAST(CharacterJointEffect, effect)->matches_character(_character)) {
      node->clear_effect(CharacterJointEffect::get_class_type());
    }
  }

  _local_transform_nodes.clear();
}

/**
 * Returns a list of the local transforms set for this node.  Note that this
 * returns a list of NodePaths, even though the local transforms are actually
 * a list of PandaNodes.
 */
NodePathCollection CharacterJoint::
get_local_transforms() {
  NodePathCollection npc;

  NodeList::iterator ai;
  for (ai = _local_transform_nodes.begin();
       ai != _local_transform_nodes.end();
       ++ai) {
    PandaNode *node = *ai;
    npc.add_path(NodePath::any_path(node));
  }

  return npc;
}

/**
 * Copies the joint's current transform into the indicated matrix.
 */
void CharacterJoint::
get_transform(LMatrix4 &transform) const {
  transform = _value;
}

CPT(TransformState) CharacterJoint::
get_transform_state() const {
    return TransformState::make_mat( _value );
}

/**
 * Copies the joint's current net transform (composed from the root of the
 * character joint hierarchy) into the indicated matrix.
 */
void CharacterJoint::
get_net_transform(LMatrix4 &transform) const {
  transform = _net_transform;
}

/**
 * Returns the Character that owns this joint.
 */
Character *CharacterJoint::
get_character() const {
  return _character;
}

/**
 * Changes the Character that owns this joint.
 */
void CharacterJoint::
set_character(Character *character) {
  if (character != _character) {

    if (character != nullptr) {
      // Change or set a _character pointer on each joint's exposed node.
      NodeList::iterator ai;
      for (ai = _net_transform_nodes.begin();
           ai != _net_transform_nodes.end();
           ++ai) {
        PandaNode *node = *ai;
        node->set_effect(CharacterJointEffect::make(character));
      }
      for (ai = _local_transform_nodes.begin();
           ai != _local_transform_nodes.end();
           ++ai) {
        PandaNode *node = *ai;
        node->set_effect(CharacterJointEffect::make(character));
      }

    } else {  // (character == (Character *)NULL)
      // Clear the _character pointer on each joint's exposed node.
      NodeList::iterator ai;
      for (ai = _net_transform_nodes.begin();
           ai != _net_transform_nodes.end();
           ++ai) {
        PandaNode *node = *ai;

        CPT(RenderEffect) effect = node->get_effect(CharacterJointEffect::get_class_type());
        if (effect != nullptr &&
            DCAST(CharacterJointEffect, effect)->matches_character(_character)) {
          node->clear_effect(CharacterJointEffect::get_class_type());
        }
      }
      for (ai = _local_transform_nodes.begin();
           ai != _local_transform_nodes.end();
           ++ai) {
        PandaNode *node = *ai;

        CPT(RenderEffect) effect = node->get_effect(CharacterJointEffect::get_class_type());
        if (effect != nullptr &&
            DCAST(CharacterJointEffect, effect)->matches_character(_character)) {
          node->clear_effect(CharacterJointEffect::get_class_type());
        }
      }
    }
  }

  _character = character;
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void CharacterJoint::
write_datagram(BamWriter *manager, Datagram &me) {
  NodeList::iterator ni;
  MovingPartMatrix::write_datagram(manager, me);

  manager->write_pointer(me, _character);

  me.add_uint16(_net_transform_nodes.size());
  for (ni = _net_transform_nodes.begin();
       ni != _net_transform_nodes.end();
       ni++) {
    manager->write_pointer(me, (*ni));
  }

  me.add_uint16(_local_transform_nodes.size());
  for (ni = _local_transform_nodes.begin();
       ni != _local_transform_nodes.end();
       ni++) {
    manager->write_pointer(me, (*ni));
  }

  _initial_net_transform_inverse.write_datagram(me);
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void CharacterJoint::
fillin(DatagramIterator &scan, BamReader *manager) {
  int i;
  MovingPartMatrix::fillin(scan, manager);

  if (manager->get_file_minor_ver() >= 4) {
    manager->read_pointer(scan);
  }

  _num_net_nodes = scan.get_uint16();
  for(i = 0; i < _num_net_nodes; i++) {
    manager->read_pointer(scan);
  }

  _num_local_nodes = scan.get_uint16();
  for(i = 0; i < _num_local_nodes; i++) {
    manager->read_pointer(scan);
  }

  _initial_net_transform_inverse.read_datagram(scan);
}

/**
 * Takes in a vector of pointers to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int CharacterJoint::
complete_pointers(TypedWritable **p_list, BamReader* manager) {
  int pi = MovingPartMatrix::complete_pointers(p_list, manager);

  if (manager->get_file_minor_ver() >= 4) {
    _character = DCAST(Character, p_list[pi++]);
  } else {
    _character = nullptr;
  }

  int i;
  for (i = 0; i < _num_net_nodes; i++) {
    PandaNode *node = DCAST(PandaNode, p_list[pi++]);
    _net_transform_nodes.insert(node);
  }

  for (i = 0; i < _num_local_nodes; i++) {
    PandaNode *node = DCAST(PandaNode, p_list[pi++]);
    _local_transform_nodes.insert(node);
  }

  return pi;
}

/**
 * Factory method to generate a CharacterJoint object
 */
TypedWritable* CharacterJoint::
make_CharacterJoint(const FactoryParams &params) {
  CharacterJoint *me = new CharacterJoint;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate a CharacterJoint object
 */
void CharacterJoint::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CharacterJoint);
}
