// Filename: onOffTransition.cxx
// Created by:  drose (20Mar00)
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

#include "onOffTransition.h"
#include "onOffAttribute.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle OnOffTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::compose
//       Access: Public, Virtual
//  Description: Returns a new transition that corresponds to the
//               composition of this transition with the second
//               transition (which must be of an equivalent type).
//               This may return the same pointer as either source
//               transition.  Applying the transition returned from
//               this function to an attribute attribute will produce
//               the same effect as applying each transition
//               separately.
////////////////////////////////////////////////////////////////////
NodeTransition *OnOffTransition::
compose(const NodeTransition *other) const {
  const OnOffTransition *ot;
  DCAST_INTO_R(ot, other, NULL);

  if (ot->_priority < _priority || ot->_direction == TD_identity) {
    // The other transition is identity or too low-priority; the
    // result is unchanged.
    return (OnOffTransition *)this;
  }

  // In any other case, for an OnOffTransition, the result is the same
  // as the second operand.
  return (OnOffTransition *)ot;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::invert
//       Access: Public, Virtual
//  Description: Returns a new transition that corresponds to the
//               inverse of this transition.  If the transition was
//               identity, this may return the same pointer.  Returns
//               NULL if the transition cannot be inverted.
////////////////////////////////////////////////////////////////////
NodeTransition *OnOffTransition::
invert() const {
  switch (_direction) {
  case TD_identity:
    return (OnOffTransition *)this;

  case TD_on:
    OnOffTransition *result;
    DCAST_INTO_R(result, make_copy(), NULL);
    result->_direction = TD_off;
    return result;

  case TD_off:
    return NULL;
  }

  // Invalid direction flag.
  nassertr(false, NULL);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::apply
//       Access: Public, Virtual
//  Description: Returns a new attribute (or possibly the same
//               attribute) that represents the effect of applying this
//               indicated transition to the indicated attribute.  The
//               source attribute may be NULL, indicating the initial
//               attribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *OnOffTransition::
apply(const NodeAttribute *attrib) const {
  if (_direction == TD_identity) {
    // The transition is identity; it has no effect.
    return (NodeAttribute *)attrib;
  }

  OnOffAttribute *result;
  if (attrib == (const NodeAttribute *)NULL) {
    DCAST_INTO_R(result, make_attrib(), NULL);
  } else {
    DCAST_INTO_R(result, (NodeAttribute *)attrib, NULL);
  }

  if (_priority < result->_priority || _direction == TD_identity) {
    // The priority is too low to affect the attribute, or the
    // transition is identity.
    return result;
  }

  if (result->get_ref_count() > 1) {
    // Copy on write.
    DCAST_INTO_R(result, result->make_copy(), NULL);
  }

  result->_priority = _priority;

  if (_direction == TD_on) {
    result->_is_on = true;
    result->set_value_from(this);
  } else {
    result->_is_on = false;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void OnOffTransition::
output(ostream &out) const {
  switch (_direction) {
  case TD_identity:
    out << "identity-" << get_handle();
    return;

  case TD_on:
    out << "on-" << get_handle() << ":";
    output_value(out);
    return;

  case TD_off:
    out << "off-" << get_handle();
    return;
  }

  // Invalid direction flag.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void OnOffTransition::
write(ostream &out, int indent_level) const {
  switch (_direction) {
  case TD_identity:
    indent(out, indent_level) << "identity-" << get_handle() << "\n";
    return;

  case TD_on:
    indent(out, indent_level) << "on-" << get_handle() << ":\n";
    write_value(out, indent_level + 2);
    return;

  case TD_off:
    indent(out, indent_level) << "off-" << get_handle() << "\n";
    return;
  }

  // Invalid direction flag.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description: Returns a number < 0 if this transition sorts before
//               the other transition, > 0 if it sorts after, 0 if
//               they are equivalent (except for priority).
////////////////////////////////////////////////////////////////////
int OnOffTransition::
internal_compare_to(const NodeTransition *other) const {
  const OnOffTransition *ot;
  DCAST_INTO_R(ot, other, false);

  if (_direction != ot->_direction) {
    return (_direction < ot->_direction) ? -1 : 1;
  }
  if (_direction == TD_on) {
    return compare_values(ot);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Assigns the value from the corresponding value of the
//               other transition, which is assumed to be of an
//               equivalent type.
////////////////////////////////////////////////////////////////////
void OnOffTransition::
set_value_from(const OnOffTransition *) {
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::compare_values
//       Access: Protected, Virtual
//  Description: Returns 0 if the two transitions share the same
//               value, or < 0 or > 0 otherwise.  It is given that
//               they are of equivalent types and that they are both
//               "on" transitions.
////////////////////////////////////////////////////////////////////
int OnOffTransition::
compare_values(const OnOffTransition *) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::output_value
//       Access: Protected, Virtual
//  Description: Outputs the value on one line.
////////////////////////////////////////////////////////////////////
void OnOffTransition::
output_value(ostream &) const {
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::write_value
//       Access: Protected, Virtual
//  Description: Outputs the value on multiple lines.
////////////////////////////////////////////////////////////////////
void OnOffTransition::
write_value(ostream &, int) const {
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void OnOffTransition::
write_datagram(BamWriter *manager, Datagram &me)
{
  NodeTransition::write_datagram(manager, me);
  me.add_uint8(_direction);
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void OnOffTransition::
fillin(DatagramIterator& scan, BamReader* manager)
{
  NodeTransition::fillin(scan, manager);
  _direction = (enum TransitionDirection) scan.get_uint8();
}



