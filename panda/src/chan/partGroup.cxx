/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partGroup.cxx
 * @author drose
 * @date 1999-02-22
 */

#include "partGroup.h"
#include "animGroup.h"
#include "config_chan.h"
#include "partSubset.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "transformState.h"

#include <algorithm>

using std::ostream;

TypeHandle PartGroup::_type_handle;

/**
 * Creates the PartGroup, and adds it to the indicated parent.  The only way
 * to delete it subsequently is to delete the entire hierarchy.
 */
PartGroup::
PartGroup(PartGroup *parent, const std::string &name) :
  Namable(name),
  _children(get_class_type())
{
  nassertv(parent != nullptr);

  parent->_children.push_back(this);
}

/**
 *
 */
PartGroup::
~PartGroup() {
}

/**
 * Returns true if this part is a CharacterJoint, false otherwise.  This is a
 * tiny optimization over is_of_type(CharacterType::get_class_type()).
 */
bool PartGroup::
is_character_joint() const {
  return false;
}

/**
 * Allocates and returns a new copy of the node.  Children are not copied, but
 * see copy_subgraph().
 */
PartGroup *PartGroup::
make_copy() const {
  return new PartGroup(*this);
}

/**
 * Allocates and returns a new copy of this node and of all of its children.
 */
PartGroup *PartGroup::
copy_subgraph() const {
  PartGroup *root = make_copy();

  if (root->get_type() != get_type()) {
    chan_cat.warning()
      << "Don't know how to copy " << get_type() << "\n";
  }

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    PartGroup *child = (*ci)->copy_subgraph();
    root->_children.push_back(child);
  }

  return root;
}


/**
 * Returns the number of child nodes of the group.
 */
int PartGroup::
get_num_children() const {
  return _children.size();
}


/**
 * Returns the nth child of the group.
 */
PartGroup *PartGroup::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), nullptr);
  return _children[n];
}

/**
 * Returns the first child found with the indicated name, or NULL if no such
 * child exists.  This method searches only the children of this particular
 * PartGroup; it does not recursively search the entire graph.  See also
 * find_child().
 */
PartGroup *PartGroup::
get_child_named(const std::string &name) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    PartGroup *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
  }

  return nullptr;
}

/**
 * Returns the first descendant found with the indicated name, or NULL if no
 * such descendant exists.  This method searches the entire graph beginning at
 * this PartGroup; see also get_child_named().
 */
PartGroup *PartGroup::
find_child(const std::string &name) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    PartGroup *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
    PartGroup *result = child->find_child(name);
    if (result != nullptr) {
      return result;
    }
  }

  return nullptr;
}

// An STL object to sort a list of children into alphabetical order.
class PartGroupAlphabeticalOrder {
public:
  bool operator()(const PT(PartGroup) &a, const PT(PartGroup) &b) const {
    return a->get_name() < b->get_name();
  }
};

/**
 * Sorts the children nodes at each level of the hierarchy into alphabetical
 * order.  This should be done after creating the hierarchy, to guarantee that
 * the correct names will match up together when the AnimBundle is later bound
 * to a PlayerRoot.
 */
void PartGroup::
sort_descendants() {
  std::stable_sort(_children.begin(), _children.end(), PartGroupAlphabeticalOrder());

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->sort_descendants();
  }
}

/**
 * Freezes this particular joint so that it will always hold the specified
 * transform.  Returns true if this is a joint that can be so frozen, false
 * otherwise.
 *
 * This is normally only called internally by PartBundle::freeze_joint(), but
 * you may also call it directly.
 */
bool PartGroup::
apply_freeze(const TransformState *transform) {
  return apply_freeze_matrix(transform->get_pos(), transform->get_hpr(), transform->get_scale()) || apply_freeze_scalar(transform->get_pos()[0]);
}

/**
 * Freezes this particular joint so that it will always hold the specified
 * transform.  Returns true if this is a joint that can be so frozen, false
 * otherwise.
 *
 * This is normally only called internally by PartBundle::freeze_joint(), but
 * you may also call it directly.
 */
bool PartGroup::
apply_freeze_matrix(const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale) {
  return false;
}

/**
 * Freezes this particular joint so that it will always hold the specified
 * transform.  Returns true if this is a joint that can be so frozen, false
 * otherwise.
 *
 * This is normally only called internally by PartBundle::freeze_joint(), but
 * you may also call it directly.
 */
bool PartGroup::
apply_freeze_scalar(PN_stdfloat value) {
  return false;
}


/**
 * Specifies a node to influence this particular joint so that it will always
 * hold the node's transform.  Returns true if this is a joint that can be so
 * controlled, false otherwise.
 *
 * This is normally only called internally by PartBundle::control_joint(), but
 * you may also call it directly.
 */
bool PartGroup::
apply_control(PandaNode *node) {
  return false;
}

/**
 * Undoes the effect of a previous call to apply_freeze() or apply_control().
 * Returns true if the joint was modified, false otherwise.
 *
 * This is normally only called internally by PartBundle::release_joint(), but
 * you may also call it directly.
 */
bool PartGroup::
clear_forced_channel() {
  return false;
}

/**
 * Returns the AnimChannelBase that has been forced to this joint by a
 * previous call to apply_freeze() or apply_control(), or NULL if no such
 * channel has been applied.
 */
AnimChannelBase *PartGroup::
get_forced_channel() const {
  return nullptr;
}


/**
 * Writes a brief description of the group and all of its descendants.
 */
void PartGroup::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << " {\n";
  write_descendants(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}

/**
 * Writes a brief description of the group, showing its current value, and
 * that of all of its descendants.
 */
void PartGroup::
write_with_value(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << " {\n";
  write_descendants_with_value(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}

/**
 * Returns the TypeHandle associated with the ValueType we are concerned with.
 * This is provided to allow a bit of run-time checking that joints and
 * channels are matching properly in type.
 */
TypeHandle PartGroup::
get_value_type() const {
  return TypeHandle::none();
}

/**
 * Walks the part hierarchy in tandem with the indicated anim hierarchy, and
 * returns true if the hierarchies match, false otherwise.
 *
 * If hierarchy_match_flags is 0, only an exact match is accepted; otherwise,
 * it may contain a union of PartGroup::HierarchyMatchFlags values indicating
 * conditions that will be tolerated (but warnings will still be issued).
 */
bool PartGroup::
check_hierarchy(const AnimGroup *anim, const PartGroup *,
                int hierarchy_match_flags) const {
  Thread::consider_yield();
  if (anim->get_value_type() != get_value_type()) {
    if (chan_cat.is_error()) {
      chan_cat.error()
        << "Part " << get_name() << " expects type " << get_value_type()
        << " while matching anim node has type " << anim->get_value_type()
        << ".\n";
    }
    return false;
  }

  if (chan_cat.is_info()) {
    // If we're issuing error messages, check ahead of time if the set of
    // children agrees.  If it does not, we'll write a one-line warning, and
    // then list the set of children that differ.

    bool match = true;
    if (anim->get_num_children() != get_num_children()) {
      // If the only difference is "morph", ignore it.  We treat "morph" as a
      // special case, because it's common for the model and animation files
      // to differ meaninglessly here.  Any differences here remain
      // unreported.
      if (anim->get_num_children() == get_num_children() + 1 &&
          anim->get_child_named("morph") != nullptr &&
          get_child_named("morph") == nullptr) {
        // Anim has "morph" and model does not, but there are no other
        // differences.

      } else if (get_num_children() == anim->get_num_children() + 1 &&
                 get_child_named("morph") != nullptr &&
                 anim->get_child_named("morph") == nullptr) {
        // Model has "morph" and anim does not, but there are no other
        // differences.

      } else {
        chan_cat.info()
          << "Part " << get_name() << " has " << get_num_children()
          << " children, while matching anim node has "
          << anim->get_num_children() << ":\n";
        match = false;
      }

    } else {
      for (int i = 0; match && i < get_num_children(); i++) {
        PartGroup *pc = get_child(i);
        AnimGroup *ac = anim->get_child(i);

        match = (pc->get_name() == ac->get_name());
      }
      if (!match) {
        chan_cat.info()
          << "Part " << get_name() << " has a different set of children "
          << " than matching anim node:\n";
      }
    }
    if (!match) {
      int i = 0, j = 0;
      while (i < get_num_children() &&
             j < anim->get_num_children()) {
        PartGroup *pc = get_child(i);
        AnimGroup *ac = anim->get_child(j);

        if (pc->get_name() < ac->get_name()) {
          chan_cat.info()
            << "  part has " << pc->get_name()
            << ", not in anim.\n";
          i++;
        } else if (ac->get_name() < pc->get_name()) {
          chan_cat.info()
            << "  anim has " << ac->get_name()
            << ", not in part.\n";
          j++;
        } else {
          // chan_cat.info() << "  part and anim both have " << ac->get_name()
          // << "\n";
          i++;
          j++;
        }
      }

      while (i < get_num_children()) {
        PartGroup *pc = get_child(i);
        chan_cat.info()
          << "  part has " << pc->get_name()
          << ", not in anim.\n";
        i++;
      }

      while (j < anim->get_num_children()) {
        AnimGroup *ac = anim->get_child(j);
        chan_cat.info()
          << "  anim has " << ac->get_name()
          << ", not in part.\n";
        j++;
      }
    }
  }

  // Now walk the list of children and check the matching sub-hierarchies
  // only.

  int i = 0, j = 0;
  while (i < get_num_children() &&
         j < anim->get_num_children()) {
    PartGroup *pc = get_child(i);
    AnimGroup *ac = anim->get_child(j);

    if (pc->get_name() < ac->get_name()) {
      if (pc->get_name() == "morph") {
        // Model has "morph", not in anim.  Ignore.
      } else {
        if ((hierarchy_match_flags & HMF_ok_part_extra) == 0) {
          return false;
        }
      }
      i++;
    } else if (ac->get_name() < pc->get_name()) {
      if (ac->get_name() == "morph") {
        // Anim has "morph", not in model.  Ignore.
      } else {
        if ((hierarchy_match_flags & HMF_ok_anim_extra) == 0) {
          return false;
        }
      }
      j++;
    } else {
      if (!pc->check_hierarchy(ac, this, hierarchy_match_flags)) {
        return false;
      }
      i++;
      j++;
    }
  }

  while (i < get_num_children()) {
    // There's at least one extra part.
    PartGroup *pc = get_child(i);

    if (pc->get_name() == "morph") {
      // Model has "morph", not in anim.  Ignore.
    } else {
      if ((hierarchy_match_flags & HMF_ok_part_extra) == 0) {
        return false;
      }
    }
    i++;
  }

  while (j < anim->get_num_children()) {
    // There's at least one extra anim channel.
    AnimGroup *ac = anim->get_child(j);

    if (ac->get_name() == "morph") {
      // Anim has "morph", not in model.  Ignore.
    } else {
      if ((hierarchy_match_flags & HMF_ok_anim_extra) == 0) {
        return false;
      }
    }
    j++;
  }

  return true;
}


/**
 * Recursively update this particular part and all of its descendents for the
 * current frame.  This is not really public and is not intended to be called
 * directly; it is called from the top of the tree by PartBundle::update().
 *
 * The return value is true if any part has changed, false otherwise.
 */
bool PartGroup::
do_update(PartBundle *root, const CycleData *root_cdata, PartGroup *,
          bool parent_changed, bool anim_changed, Thread *current_thread) {
  bool any_changed = false;

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    if ((*ci)->do_update(root, root_cdata, this, parent_changed,
                         anim_changed, current_thread)) {
      any_changed = true;
    }
  }

  return any_changed;
}

/**
 * Called by PartBundle::xform(), this indicates the indicated transform is
 * being applied to the root joint.
 */
void PartGroup::
do_xform(const LMatrix4 &mat, const LMatrix4 &inv_mat) {
  Children::const_iterator ci;

  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->do_xform(mat, inv_mat);
  }
}

/**
 * Should be called whenever the ChannelBlend values have changed, this
 * recursively updates the _effective_channel member in each part.
 */
void PartGroup::
determine_effective_channels(const CycleData *root_cdata) {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->determine_effective_channels(root_cdata);
  }
}


/**
 * Writes a brief description of all of the group's descendants.
 */
void PartGroup::
write_descendants(ostream &out, int indent_level) const {
  Children::const_iterator ci;

  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write(out, indent_level);
  }
}

/**
 * Writes a brief description of all of the group's descendants and their
 * values.
 */
void PartGroup::
write_descendants_with_value(ostream &out, int indent_level) const {
  Children::const_iterator ci;

  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write_with_value(out, indent_level);
  }
}

/**
 * Walks the part hierarchy, looking for a suitable channel index number to
 * use.  Available index numbers are the elements of the holes set, as well as
 * next to infinity.
 */
void PartGroup::
pick_channel_index(plist<int> &holes, int &next) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->pick_channel_index(holes, next);
  }
}


/**
 * Binds the indicated anim hierarchy to the part hierarchy, at the given
 * channel index number.
 */
void PartGroup::
bind_hierarchy(AnimGroup *anim, int channel_index, int &joint_index,
               bool is_included, BitArray &bound_joints,
               const PartSubset &subset) {
  Thread::consider_yield();
  if (subset.matches_include(get_name())) {
    is_included = true;
  } else if (subset.matches_exclude(get_name())) {
    is_included = false;
  }

  int i = 0, j = 0;
  int part_num_children = get_num_children();
  int anim_num_children = (anim == nullptr) ? 0 : anim->get_num_children();

  while (i < part_num_children && j < anim_num_children) {
    PartGroup *pc = get_child(i);
    AnimGroup *ac = anim->get_child(j);

    if (pc->get_name() < ac->get_name()) {
      // Here's a part, not in the anim.  Bind it to the special NULL anim.
      pc->bind_hierarchy(nullptr, channel_index, joint_index, is_included,
                         bound_joints, subset);
      i++;

    } else if (ac->get_name() < pc->get_name()) {
      // Here's an anim, not in the part.  Ignore it.
      j++;

    } else {
      // Here's a matched part and anim pair.
      pc->bind_hierarchy(ac, channel_index, joint_index, is_included,
                         bound_joints, subset);
      i++;
      j++;
    }
  }

  // Now pick up any more parts, not in the anim.
  while (i < part_num_children) {
    PartGroup *pc = get_child(i);
    pc->bind_hierarchy(nullptr, channel_index, joint_index, is_included,
                       bound_joints, subset);
    i++;
  }
}

/**
 * Similar to bind_hierarchy, but does not actually perform any binding.  All
 * it does is compute the BitArray bount_joints according to the specified
 * subset.  This is useful in preparation for asynchronous binding--in this
 * case, we may need to know bound_joints immediately, without having to wait
 * for the animation itself to load and bind.
 */
void PartGroup::
find_bound_joints(int &joint_index, bool is_included, BitArray &bound_joints,
                  const PartSubset &subset) {
  if (subset.matches_include(get_name())) {
    is_included = true;
  } else if (subset.matches_exclude(get_name())) {
    is_included = false;
  }

  int part_num_children = get_num_children();
  for (int i = 0; i < part_num_children; ++i) {
    PartGroup *pc = get_child(i);
    pc->find_bound_joints(joint_index, is_included, bound_joints, subset);
  }
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void PartGroup::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());
  me.add_uint16(_children.size());
  for (size_t i = 0; i < _children.size(); i++) {
    manager->write_pointer(me, _children[i]);
  }
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void PartGroup::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());

  if (manager->get_file_minor_ver() == 11) {
    // Skip over the old freeze-joint information, no longer stored here
    scan.get_bool();
    LMatrix4 mat;
    mat.read_datagram(scan);
  }

  int num_children = scan.get_uint16();
  _children.reserve(num_children);
  for (int i = 0; i < num_children; i++) {
    manager->read_pointer(scan);
    _children.push_back(nullptr);
  }
}

/**
 * Takes in a vector of pointers to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int PartGroup::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci) = DCAST(PartGroup, p_list[pi++]);
  }

  return pi;
}

/**
 * Factory method to generate a PartGroup object
 */
TypedWritable* PartGroup::
make_PartGroup(const FactoryParams &params) {
  PartGroup *me = new PartGroup;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate a PartGroup object
 */
void PartGroup::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_PartGroup);
}
