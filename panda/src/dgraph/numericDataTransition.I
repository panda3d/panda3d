// Filename: numericDataTransition.I
// Created by:  drose (25Jan99)
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

/* okcircular */
#include "numericDataAttribute.h"

template<class NumType>
TypeHandle NumericDataTransition<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumericDataTransition<NumType>::
NumericDataTransition() :
  _scale(1),
  _offset(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumericDataTransition<NumType>::
NumericDataTransition(NumType scale, NumType offset) :
  _scale(scale),
  _offset(offset)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumericDataTransition<NumType>::
NumericDataTransition(const NumericDataTransition<NumType> &copy) :
  NodeTransition(copy),
  _scale(copy._scale),
  _offset(copy._offset)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void NumericDataTransition<NumType>::
operator = (const NumericDataTransition<NumType> &copy) {
  NodeTransition::operator = (copy);
  _scale = copy._scale;
  _offset = copy._offset;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::is_identity
//       Access: Public, Virtual
//  Description: Returns true if this transition does not affect any
//               numbers going through it.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool NumericDataTransition<NumType>::
is_identity() const {
  return (_scale == 1.0 && _offset == 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::set_scale
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
void NumericDataTransition<NumType>::
set_scale(NumType scale) {
  _scale = scale;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::get_scale
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
NumType NumericDataTransition<NumType>::
get_scale() const {
  return _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::set_offset
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
void NumericDataTransition<NumType>::
set_offset(NumType offset) {
  _offset = offset;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::get_offset
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
NumType NumericDataTransition<NumType>::
get_offset() const {
  return _offset;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::compose
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
template<class NumType>
NodeTransition *NumericDataTransition<NumType>::
compose(const NodeTransition *other) const {
  const NumericDataTransition<NumType> *ot;
  DCAST_INTO_R(ot, other, NULL);

  if (is_identity()) {
    return (NumericDataTransition<NumType> *)ot;

  } else if (ot->is_identity()) {
    return (NumericDataTransition<NumType> *)this;

  } else {
    NodeTransition *c = make_copy();
    NumericDataTransition<NumType> *result;
    DCAST_INTO_R(result, c, NULL);

    result->_offset = _offset + _scale * ot->_offset;
    result->_scale = _scale * ot->_scale;
    return result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::invert
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
NodeTransition *NumericDataTransition<NumType>::
invert() const {
  if (is_identity()) {
    return (NumericDataTransition<NumType> *)this;

  } else if (_scale == 0) {
    // Singular; cannot invert.
    return NULL;

  } else {
    NodeTransition *c = make_copy();
    NumericDataTransition<NumType> *result;
    DCAST_INTO_R(result, c, NULL);

    result->_offset = - _offset / _scale;
    result->_scale = (NumType)1 / _scale;
    return result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::apply
//       Access: Public, Virtual
//  Description: Returns a new attribute (or possibly the same
//               attribute) that represents the effect of applying this
//               indicated transition to the indicated attribute.  The
//               source attribute may be NULL, indicating the initial
//               attribute.
////////////////////////////////////////////////////////////////////
template<class NumType>
NodeAttribute *NumericDataTransition<NumType>::
apply(const NodeAttribute *attrib) const {
  const NumericDataAttribute<NumType> *at;
  DCAST_INTO_R(at, attrib, NULL);

  if (is_identity()) {
    return (NumericDataAttribute<NumType> *)attrib;
  }

  NodeAttribute *c = make_attrib();
  NumericDataAttribute<NumType> *result;
  DCAST_INTO_R(result, c, NULL);

  result->_value = at->_value * _scale + _offset;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
void NumericDataTransition<NumType>::
output(ostream &out) const {
  out << _scale << "x + " << _offset;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
void NumericDataTransition<NumType>::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _scale << "x + " << _offset << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
int NumericDataTransition<NumType>::
internal_compare_to(const NodeTransition *other) const {
  const NumericDataTransition<NumType> *ot;
  DCAST_INTO_R(ot, other, false);

  if (_scale != ot->_scale) {
    return (_scale < ot->_scale) ? -1 : 1;
  }
  if (_offset != ot->_offset) {
    return (_offset < ot->_offset) ? -1 : 1;
  }
  return 0;
}
