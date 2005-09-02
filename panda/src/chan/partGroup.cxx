// Filename: partGroup.cxx
// Created by:  drose (22Feb99)
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


#include "partGroup.h"
#include "animGroup.h"
#include "config_chan.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

#include <algorithm>

TypeHandle PartGroup::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::Constructor
//       Access: Public
//  Description: Creates the PartGroup, and adds it to the indicated
//               parent.  The only way to delete it subsequently is to
//               delete the entire hierarchy.
////////////////////////////////////////////////////////////////////
PartGroup::
PartGroup(PartGroup *parent, const string &name) : Namable(name) {
  nassertv(parent != NULL);

  parent->_children.push_back(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PartGroup::
~PartGroup() {
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the node.
//               Children are not copied, but see copy_subgraph().
////////////////////////////////////////////////////////////////////
PartGroup *PartGroup::
make_copy() const {
  return new PartGroup(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::copy_subgraph
//       Access: Public
//  Description: Allocates and returns a new copy of this node and of
//               all of its children.
////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////
//     Function: PartGroup::get_num_children
//       Access: Public
//  Description: Returns the number of child nodes of the group.
////////////////////////////////////////////////////////////////////
int PartGroup::
get_num_children() const {
  return _children.size();
}


////////////////////////////////////////////////////////////////////
//     Function: PartGroup::get_child
//       Access: Public
//  Description: Returns the nth child of the group.
////////////////////////////////////////////////////////////////////
PartGroup *PartGroup::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), NULL);
  return _children[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::find_child
//       Access: Public
//  Description: Returns the first descendant found with the indicated
//               name, or NULL if no such descendant exists.
////////////////////////////////////////////////////////////////////
PartGroup *PartGroup::
find_child(const string &name) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    PartGroup *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
    PartGroup *result = child->find_child(name);
    if (result != (PartGroup *)NULL) {
      return result;
    }
  }

  return (PartGroup *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::get_value_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle associated with the ValueType
//               we are concerned with.  This is provided to allow a
//               bit of run-time checking that joints and channels are
//               matching properly in type.
////////////////////////////////////////////////////////////////////
TypeHandle PartGroup::
get_value_type() const {
  return TypeHandle::none();
}


// An STL object to sort a list of children into alphabetical order.
class PartGroupAlphabeticalOrder {
public:
  bool operator()(const PT(PartGroup) &a, const PT(PartGroup) &b) const {
    return a->get_name() < b->get_name();
  }
};

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::sort_descendants
//       Access: Public
//  Description: Sorts the children nodes at each level of the
//               hierarchy into alphabetical order.  This should be
//               done after creating the hierarchy, to guarantee that
//               the correct names will match up together when the
//               AnimBundle is later bound to a PlayerRoot.
////////////////////////////////////////////////////////////////////
void PartGroup::
sort_descendants() {
  stable_sort(_children.begin(), _children.end(), PartGroupAlphabeticalOrder());

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->sort_descendants();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::check_hierarchy
//       Access: Public
//  Description: Walks the part hierarchy in tandem with the indicated
//               anim hierarchy, and returns true if the hierarchies
//               match, false otherwise.
//
//               If hierarchy_match_flags is 0, only an exact match is
//               accepted; otherwise, it may contain a union of
//               PartGroup::HierarchyMatchFlags values indicating
//               conditions that will be tolerated (but warnings will
//               still be issued).
//
//               If there is a discrepancy, it is reported to the
//               indicated output stream, if it is non-null.
////////////////////////////////////////////////////////////////////
bool PartGroup::
check_hierarchy(const AnimGroup *anim, const PartGroup *,
                int hierarchy_match_flags) const {
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
    // If we're issuing error messages, check ahead of time if the set
    // of children agrees.  If it does not, we'll write a one-line
    // warning, and then list the set of children that differ.

    bool match = true;
    if (anim->get_num_children() != get_num_children()) {
      chan_cat.info()
        << "Part " << get_name() << " has " << get_num_children()
        << " children, while matching anim node has "
        << anim->get_num_children() << ":\n";
      match = false;

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
          //      chan_cat.info() << "  part and anim both have " << ac->get_name() << "\n";
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

  // Now walk the list of children and check the matching
  // sub-hierarchies only.

  int i = 0, j = 0;
  while (i < get_num_children() &&
         j < anim->get_num_children()) {
    PartGroup *pc = get_child(i);
    AnimGroup *ac = anim->get_child(j);

    if (pc->get_name() < ac->get_name()) {
      if ((hierarchy_match_flags & HMF_ok_part_extra) == 0) {
        return false;
      }
      i++;
    } else if (ac->get_name() < pc->get_name()) {
      if ((hierarchy_match_flags & HMF_ok_anim_extra) == 0) {
        return false;
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

  if (i < get_num_children()) {
    // There's at least one extra part.
    if ((hierarchy_match_flags & HMF_ok_part_extra) == 0) {
      return false;
    }
  }

  if (j < anim->get_num_children()) {
    // There's at least one extra anim channel.
    if ((hierarchy_match_flags & HMF_ok_anim_extra) == 0) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::write
//       Access: Public, Virtual
//  Description: Writes a brief description of the group and all of
//               its descendants.
////////////////////////////////////////////////////////////////////
void PartGroup::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << " {\n";
  write_descendants(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::write_with_value
//       Access: Public, Virtual
//  Description: Writes a brief description of the group, showing its
//               current value, and that of all of its descendants.
////////////////////////////////////////////////////////////////////
void PartGroup::
write_with_value(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << " {\n";
  write_descendants_with_value(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}


////////////////////////////////////////////////////////////////////
//     Function: PartGroup::do_update
//       Access: Public, Virtual
//  Description: Recursively update this particular part and all of
//               its descendents for the current frame.  This is not
//               really public and is not intended to be called
//               directly; it is called from the top of the tree by
//               PartBundle::update().
//
//               The return value is true if any part has changed,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PartGroup::
do_update(PartBundle *root, PartGroup *,
          bool parent_changed, bool anim_changed) {
  bool any_changed = false;

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    if ((*ci)->do_update(root, this, parent_changed, anim_changed)) {
      any_changed = true;
    }
  }

  return any_changed;
}


////////////////////////////////////////////////////////////////////
//     Function: PartGroup::write_descendants
//       Access: Protected
//  Description: Writes a brief description of all of the group's
//               descendants.
////////////////////////////////////////////////////////////////////
void PartGroup::
write_descendants(ostream &out, int indent_level) const {
  Children::const_iterator ci;

  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::write_descendants_with_value
//       Access: Protected
//  Description: Writes a brief description of all of the group's
//               descendants and their values.
////////////////////////////////////////////////////////////////////
void PartGroup::
write_descendants_with_value(ostream &out, int indent_level) const {
  Children::const_iterator ci;

  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write_with_value(out, indent_level);
  }
}




////////////////////////////////////////////////////////////////////
//     Function: PartGroup::pick_channel_index
//       Access: Protected, Virtual
//  Description: Walks the part hierarchy, looking for a suitable
//               channel index number to use.  Available index numbers
//               are the elements of the holes set, as well as next to
//               infinity.
////////////////////////////////////////////////////////////////////
void PartGroup::
pick_channel_index(plist<int> &holes, int &next) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->pick_channel_index(holes, next);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PartGroup::bind_hierarchy
//       Access: Protected, Virtual
//  Description: Binds the indicated anim hierarchy to the part
//               hierarchy, at the given channel index number.
////////////////////////////////////////////////////////////////////
void PartGroup::
bind_hierarchy(AnimGroup *anim, int channel_index) {
  int i = 0, j = 0;
  int part_num_children = get_num_children();
  int anim_num_children = (anim == NULL) ? 0 : anim->get_num_children();

  while (i < part_num_children && j < anim_num_children) {
    PartGroup *pc = get_child(i);
    AnimGroup *ac = anim->get_child(j);

    if (pc->get_name() < ac->get_name()) {
      // Here's a part, not in the anim.  Bind it to the special NULL
      // anim.
      pc->bind_hierarchy(NULL, channel_index);
      i++;
    } else if (ac->get_name() < pc->get_name()) {
      j++;
    } else {
      pc->bind_hierarchy(ac, channel_index);
      i++;
      j++;
    }
  }

  // Now pick up any more parts, not in the anim.
  while (i < part_num_children) {
    PartGroup *pc = get_child(i);
    pc->bind_hierarchy(NULL, channel_index);
    i++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void PartGroup::
write_datagram(BamWriter *manager, Datagram &me)
{
  int i;
  me.add_string(get_name());
  me.add_uint16(_children.size());
  for(i = 0; i < (int)_children.size(); i++)
  {
    manager->write_pointer(me, _children[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void PartGroup::
fillin(DatagramIterator& scan, BamReader* manager)
{
  int i;
  set_name(scan.get_string());
  _num_children = scan.get_uint16();
  for(i = 0; i < _num_children; i++)
  {
    manager->read_pointer(scan);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int PartGroup::
complete_pointers(TypedWritable **p_list, BamReader*)
{
  int i;
  for(i = 0; i < _num_children; i++)
  {
    if (p_list[i] == TypedWritable::Null)
    {
      chan_cat->warning() << get_type().get_name()
                          << " Ignoring null PartGroup" << endl;
    }
    else
    {
      _children.push_back(DCAST(PartGroup, p_list[i]));
    }
  }

  return _num_children;
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::make_PartGroup
//       Access: Protected
//  Description: Factory method to generate a PartGroup object
////////////////////////////////////////////////////////////////////
TypedWritable* PartGroup::
make_PartGroup(const FactoryParams &params)
{
  PartGroup *me = new PartGroup;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a PartGroup object
////////////////////////////////////////////////////////////////////
void PartGroup::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_PartGroup);
}

