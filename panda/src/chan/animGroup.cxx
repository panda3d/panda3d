// Filename: animGroup.cxx
// Created by:  drose (21Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "animGroup.h"
#include "animBundle.h"
#include "animChannelMatrixDynamic.h"
#include "animChannelScalarDynamic.h"
#include "config_chan.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"


#include <algorithm>

TypeHandle AnimGroup::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::Constructor
//       Access: Public
//  Description: Creates the AnimGroup, and adds it to the indicated
//               parent.  The only way to delete it subsequently is to
//               delete the entire hierarchy.
////////////////////////////////////////////////////////////////////
AnimGroup::
AnimGroup(AnimGroup *parent, const string &name) : Namable(name) {
  nassertv(parent != NULL);

  parent->_children.push_back(this);
  _root = parent->_root;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
AnimGroup::
~AnimGroup() {
}


////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::get_num_children
//       Access: Public
//  Description: Returns the number of child nodes of the group.
////////////////////////////////////////////////////////////////////
int AnimGroup::
get_num_children() const {
  return _children.size();
}


////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::get_child
//       Access: Public
//  Description: Returns the nth child of the group.
////////////////////////////////////////////////////////////////////
AnimGroup *AnimGroup::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), NULL);
  return _children[n];
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::find_child
//       Access: Public
//  Description: Returns the first descendant found with the indicated
//               name, or NULL if no such descendant exists.
////////////////////////////////////////////////////////////////////
AnimGroup *AnimGroup::
find_child(const string &name) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    AnimGroup *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
    AnimGroup *result = child->find_child(name);
    if (result != (AnimGroup *)NULL) {
      return result;
    }
  }

  return (AnimGroup *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::make_child_dynamic
//       Access: Public
//  Description: Finds the indicated child and replaces it with an
//               AnimChannelMatrixDynamic or AnimChannelScalarDynamic,
//               as appropriate, and returns the new channel.
//
//               This may be called before binding the animation to a
//               character to replace certain joints with
//               dynamically-controlled ones.
//
//               Returns NULL if the named child cannot be found.
////////////////////////////////////////////////////////////////////
AnimGroup *AnimGroup::
make_child_dynamic(const string &name) {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    AnimGroup *child = (*ci);
    if (child->get_name() == name) {
      AnimGroup *new_child = NULL;

      if (child->is_of_type(AnimChannelMatrix::get_class_type())) {
        AnimChannelMatrix *mchild = DCAST(AnimChannelMatrix, child);
        AnimChannelMatrixDynamic *new_mchild = 
          new AnimChannelMatrixDynamic(this, name);
        new_child = new_mchild;

        // Copy in the original value from frame 0.
        LMatrix4f orig_value;
        mchild->get_value(0, orig_value);
        new_mchild->set_value(orig_value);

      } else if (child->is_of_type(AnimChannelScalar::get_class_type())) {
        AnimChannelScalar *schild = DCAST(AnimChannelScalar, child);
        AnimChannelScalarDynamic *new_schild = 
          new AnimChannelScalarDynamic(this, name);
        new_child = new_schild;

        // Copy in the original value from frame 0.
        float orig_value;
        schild->get_value(0, orig_value);
        new_schild->set_value(orig_value);
      }

      if (new_child != (AnimGroup *)NULL) {
        new_child->_children.swap(child->_children);
        nassertr(_children.back() == new_child, NULL);

        // The new child was appended to the end of our children list
        // by its constructor.  Reposition it to replace the original
        // child.
#ifndef __GNUC__
        // There appears to be a compiler bug in gcc 3.2 that causes
        // the following not to compile correctly:
        (*ci) = new_child;
        _children.pop_back();
#else
        // But this longer way of achieving the same result works
        // instead:
        Children::iterator nci;
        Children new_children;
        for (nci = _children.begin(); nci != _children.end(); ++nci) {
          if ((*nci) == child) {
            new_children.push_back(new_child);
          } else if ((*nci) != new_child) {
            new_children.push_back(*nci);
          }
        }
        new_children.swap(_children);
        new_children.clear();
#endif
        return new_child;
      }
    }
    AnimGroup *result = child->make_child_dynamic(name);
    if (result != (AnimGroup *)NULL) {
      return result;
    }
  }

  return (AnimGroup *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::get_value_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle associated with the ValueType
//               we are concerned with.  This is provided to allow a
//               bit of run-time checking that joints and channels are
//               matching properly in type.
////////////////////////////////////////////////////////////////////
TypeHandle AnimGroup::
get_value_type() const {
  return TypeHandle::none();
}


// An STL object to sort a list of children into alphabetical order.
class AnimGroupAlphabeticalOrder {
public:
  bool operator()(const PT(AnimGroup) &a, const PT(AnimGroup) &b) const {
    return a->get_name() < b->get_name();
  }
};

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::sort_descendants
//       Access: Public
//  Description: Sorts the children nodes at each level of the
//               hierarchy into alphabetical order.  This should be
//               done after creating the hierarchy, to guarantee that
//               the correct names will match up together when the
//               AnimBundle is later bound to a PlayerRoot.
////////////////////////////////////////////////////////////////////
void AnimGroup::
sort_descendants() {
  sort(_children.begin(), _children.end(), AnimGroupAlphabeticalOrder());

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->sort_descendants();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::output
//       Access: Public, Virtual
//  Description: Writes a one-line description of the group.
////////////////////////////////////////////////////////////////////
void AnimGroup::
output(ostream &out) const {
  out << get_type() << " " << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::write
//       Access: Public, Virtual
//  Description: Writes a brief description of the group and all of
//               its descendants.
////////////////////////////////////////////////////////////////////
void AnimGroup::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this;
  if (!_children.empty()) {
    out << " {\n";
    write_descendants(out, indent_level + 2);
    indent(out, indent_level) << "}";
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::write_descendants
//       Access: Protected
//  Description: Writes a brief description of all of the group's
//               descendants.
////////////////////////////////////////////////////////////////////
void AnimGroup::
write_descendants(ostream &out, int indent_level) const {
  Children::const_iterator ci;

  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void AnimGroup::
write_datagram(BamWriter *manager, Datagram &me)
{
  me.add_string(get_name());
  //Write out the root
  manager->write_pointer(me, this->_root);
  me.add_uint16(_children.size());
  for(int i = 0; i < (int)_children.size(); i++)
  {
    manager->write_pointer(me, _children[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void AnimGroup::
fillin(DatagramIterator& scan, BamReader* manager)
{
  set_name(scan.get_string());
  manager->read_pointer(scan);
  _num_children = scan.get_uint16();
  for(int i = 0; i < _num_children; i++)
  {
    manager->read_pointer(scan);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int AnimGroup::
complete_pointers(TypedWritable **p_list, BamReader*)
{
  nassertr(p_list[0] != TypedWritable::Null, 0);
  _root = DCAST(AnimBundle, p_list[0]);
  for(int i = 1; i < _num_children+1; i++)
  {
    if (p_list[i] == TypedWritable::Null)
    {
      chan_cat->warning() << get_type().get_name()
                          << " Ignoring null child" << endl;
    }
    else
    {
      _children.push_back(DCAST(AnimGroup, p_list[i]));
    }
  }
  return _num_children+1;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::make_AnimGroup
//       Access: Protected
//  Description: Factory method to generate a AnimGroup object
////////////////////////////////////////////////////////////////////
TypedWritable* AnimGroup::
make_AnimGroup(const FactoryParams &params)
{
  AnimGroup *me = new AnimGroup;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimGroup::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a AnimGroup object
////////////////////////////////////////////////////////////////////
void AnimGroup::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimGroup);
}







