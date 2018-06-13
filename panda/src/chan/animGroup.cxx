/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animGroup.cxx
 * @author drose
 * @date 1999-02-21
 */

#include "animGroup.h"
#include "animBundle.h"
#include "config_chan.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"


#include <algorithm>

using std::string;

TypeHandle AnimGroup::_type_handle;


/**
 * The default constructor is protected: don't try to create an AnimGroup
 * without a parent.  To create an AnimChannel hierarchy, you must first
 * create an AnimBundle, and use that to create any subsequent children.
 */
AnimGroup::
AnimGroup(const string &name) :
  Namable(name),
  _children(get_class_type()),
  _root(nullptr)
{
}

/**
 * Creates a new AnimGroup, just like this one, without copying any children.
 * The new copy is added to the indicated parent.  Intended to be called by
 * make_copy() only.
 */
AnimGroup::
AnimGroup(AnimGroup *parent, const AnimGroup &copy) :
  Namable(copy),
  _children(get_class_type())
{
  if (parent != nullptr) {
    parent->_children.push_back(this);
    _root = parent->_root;
  } else {
    _root = nullptr;
  }
}

/**
 * Creates the AnimGroup, and adds it to the indicated parent.  The only way
 * to delete it subsequently is to delete the entire hierarchy.
 */
AnimGroup::
AnimGroup(AnimGroup *parent, const string &name) :
  Namable(name),
  _children(get_class_type())
 {
  nassertv(parent != nullptr);

  parent->_children.push_back(this);
  _root = parent->_root;
}

/**
 *
 */
AnimGroup::
~AnimGroup() {
}


/**
 * Returns the number of child nodes of the group.
 */
int AnimGroup::
get_num_children() const {
  return _children.size();
}


/**
 * Returns the nth child of the group.
 */
AnimGroup *AnimGroup::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), nullptr);
  return _children[n];
}

/**
 * Returns the first child found with the indicated name, or NULL if no such
 * child exists.  This method searches only the children of this particular
 * AnimGroup; it does not recursively search the entire graph.  See also
 * find_child().
 */
AnimGroup *AnimGroup::
get_child_named(const string &name) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    AnimGroup *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
  }

  return nullptr;
}

/**
 * Returns the first descendant found with the indicated name, or NULL if no
 * such descendant exists.  This method searches the entire graph beginning at
 * this AnimGroup; see also get_child_named().
 */
AnimGroup *AnimGroup::
find_child(const string &name) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    AnimGroup *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
    AnimGroup *result = child->find_child(name);
    if (result != nullptr) {
      return result;
    }
  }

  return nullptr;
}

// An STL object to sort a list of children into alphabetical order.
class AnimGroupAlphabeticalOrder {
public:
  bool operator()(const PT(AnimGroup) &a, const PT(AnimGroup) &b) const {
    return a->get_name() < b->get_name();
  }
};

/**
 * Sorts the children nodes at each level of the hierarchy into alphabetical
 * order.  This should be done after creating the hierarchy, to guarantee that
 * the correct names will match up together when the AnimBundle is later bound
 * to a PlayerRoot.
 */
void AnimGroup::
sort_descendants() {
  sort(_children.begin(), _children.end(), AnimGroupAlphabeticalOrder());

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->sort_descendants();
  }
}


/**
 * Returns the TypeHandle associated with the ValueType we are concerned with.
 * This is provided to allow a bit of run-time checking that joints and
 * channels are matching properly in type.
 */
TypeHandle AnimGroup::
get_value_type() const {
  return TypeHandle::none();
}

/**
 * Writes a one-line description of the group.
 */
void AnimGroup::
output(std::ostream &out) const {
  out << get_type() << " " << get_name();
}

/**
 * Writes a brief description of the group and all of its descendants.
 */
void AnimGroup::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this;
  if (!_children.empty()) {
    out << " {\n";
    write_descendants(out, indent_level + 2);
    indent(out, indent_level) << "}";
  }
  out << "\n";
}

/**
 * Writes a brief description of all of the group's descendants.
 */
void AnimGroup::
write_descendants(std::ostream &out, int indent_level) const {
  Children::const_iterator ci;

  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write(out, indent_level);
  }
}

/**
 * Returns a copy of this object, and attaches it to the indicated parent
 * (which may be NULL only if this is an AnimBundle).  Intended to be called
 * by copy_subtree() only.
 */
AnimGroup *AnimGroup::
make_copy(AnimGroup *parent) const {
  return new AnimGroup(parent, *this);
}


/**
 * Returns a full copy of the subtree at this node and below.
 */
PT(AnimGroup) AnimGroup::
copy_subtree(AnimGroup *parent) const {
  PT(AnimGroup) new_group = make_copy(parent);
  nassertr(new_group->get_type() == get_type(), (AnimGroup *)this);

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->copy_subtree(new_group);
  }

  return new_group;
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void AnimGroup::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());
  // Write out the root
  manager->write_pointer(me, this->_root);
  me.add_uint16(_children.size());
  for(int i = 0; i < (int)_children.size(); i++) {
    manager->write_pointer(me, _children[i]);
  }
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void AnimGroup::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  manager->read_pointer(scan);
  _num_children = scan.get_uint16();
  for(int i = 0; i < _num_children; i++)
  {
    manager->read_pointer(scan);
  }
}

/**
 * Takes in a vector of pointes to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int AnimGroup::
complete_pointers(TypedWritable **p_list, BamReader *) {
  _root = DCAST(AnimBundle, p_list[0]);
  for (int i = 1; i < _num_children+1; i++) {
    if (p_list[i] == TypedWritable::Null) {
      chan_cat->warning() << get_type().get_name()
                          << " Ignoring null child" << std::endl;
    } else {
      _children.push_back(DCAST(AnimGroup, p_list[i]));
    }
  }
  return _num_children+1;
}

/**
 * Factory method to generate a AnimGroup object
 */
TypedWritable* AnimGroup::
make_AnimGroup(const FactoryParams &params) {
  AnimGroup *me = new AnimGroup;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate a AnimGroup object
 */
void AnimGroup::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimGroup);
}
