// Filename: qpnodePathComponent.cxx
// Created by:  drose (25Feb02)
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

#include "qpnodePathComponent.h"


// We start the key counters off at 1, since 0 is reserved for an
// empty NodePath (and also for an unassigned key).
int qpNodePathComponent::_next_key = 1;
TypeHandle qpNodePathComponent::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *qpNodePathComponent::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::get_key
//       Access: Public
//  Description: Returns an index number that is guaranteed to be
//               unique for this particular NodePathComponent, and not
//               to be reused for the lifetime of the application
//               (barring integer overflow).
////////////////////////////////////////////////////////////////////
int qpNodePathComponent::
get_key() const {
  if (is_collapsed()) {
    return get_collapsed()->get_key();
  }
  if (_key == 0) {
    // The first time someone asks for a particular component's key,
    // we make it up on the spot.  This helps keep us from wasting
    // index numbers generating a unique number for *every* component
    // in the world (we only have 4.2 billion 32-bit integers, after
    // all)
    ((qpNodePathComponent *)this)->_key = _next_key++;
  }
  return _key;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::is_top_node
//       Access: Public
//  Description: Returns true if this component represents the top
//               node in the path.
////////////////////////////////////////////////////////////////////
bool qpNodePathComponent::
is_top_node() const {
  if (is_collapsed()) {
    return get_collapsed()->is_top_node();
  }
  CDReader cdata(_cycler);
  return (cdata->_next == (qpNodePathComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::get_length
//       Access: Public
//  Description: Returns the length of the path to this node.
////////////////////////////////////////////////////////////////////
int qpNodePathComponent::
get_length() const {
  if (is_collapsed()) {
    return get_collapsed()->get_length();
  }
  CDReader cdata(_cycler);
  return cdata->_length;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::get_next
//       Access: Public
//  Description: Returns the next component in the path.
////////////////////////////////////////////////////////////////////
qpNodePathComponent *qpNodePathComponent::
get_next() const {
  if (is_collapsed()) {
    return get_collapsed()->get_next();
  }

  CDReader cdata(_cycler);
  qpNodePathComponent *next = cdata->_next;
  
  // If the next component has been collapsed, transparently update
  // the pointer to get the actual node, and store the new pointer,
  // before we return.  Collapsing can happen at any time to any
  // component in the path and we have to deal with it.
  if (next != (qpNodePathComponent *)NULL && next->is_collapsed()) {
    next = next->uncollapse();

    CDWriter cdata_w(((qpNodePathComponent *)this)->_cycler, cdata);
    cdata_w->_next = next;
  }
  
  return next;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::fix_length
//       Access: Public
//  Description: Checks that the length indicated by the component is
//               one more than the length of its predecessor.  If this
//               is broken, fixes it and returns true indicating the
//               component has been changed; otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool qpNodePathComponent::
fix_length() {
  int length_should_be = 1;
  if (!is_top_node()) {
    length_should_be = get_next()->get_length() + 1;
  }
  if (get_length() == length_should_be) {
    return false;
  }

  CDWriter cdata(_cycler);
  cdata->_length = length_should_be;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::uncollapse
//       Access: Public
//  Description: Returns this component pointer if the component is
//               not collapsed; or if it has been collapsed, returns
//               the pointer it has been collapsed into.
//
//               Collapsing can happen at any time to any component in
//               the path and we have to deal with it.  It happens
//               when a node is removed further up the path that
//               results in two instances becoming the same thing.
////////////////////////////////////////////////////////////////////
qpNodePathComponent *qpNodePathComponent::
uncollapse() {
  qpNodePathComponent *comp = this;

  while (comp->is_collapsed()) {
    comp = comp->get_collapsed();
  }

  return comp;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::set_next
//       Access: Private
//  Description: Sets the next pointer in the path.
////////////////////////////////////////////////////////////////////
void qpNodePathComponent::
set_next(qpNodePathComponent *next) {
  if (is_collapsed()) {
    get_collapsed()->set_next(next);
  } else {
    nassertv(next != (qpNodePathComponent *)NULL);
    CDWriter cdata(_cycler);
    cdata->_next = next;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::set_top_node
//       Access: Private
//  Description: Severs any connection to the next pointer in the
//               path and makes this component a top node.
////////////////////////////////////////////////////////////////////
void qpNodePathComponent::
set_top_node() {
  if (is_collapsed()) {
    get_collapsed()->set_top_node();
  } else {
    CDWriter cdata(_cycler);
    cdata->_next = (qpNodePathComponent *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::collapse_with
//       Access: Private
//  Description: Indicates that this component pointer is no longer
//               valid, and that the indicated component should be
//               used instead.  This is done whenever two
//               qpNodePathComponents have been collapsed together due
//               to an instance being removed higher up in the graph.
////////////////////////////////////////////////////////////////////
void qpNodePathComponent::
collapse_with(qpNodePathComponent *next) {
  nassertv(!is_collapsed());
  nassertv(next != (qpNodePathComponent *)NULL);
  CDWriter cdata(_cycler);

  // We indicate a component has been collapsed by setting its length
  // to zero.
  cdata->_next = next;
  cdata->_length = 0;

  if (_key != 0 && next->_key == 0) {
    // If we had a key set and the other one didn't, it inherits our
    // key.  Otherwise, we inherit the other's key.
    next->_key = _key;
  }
}
