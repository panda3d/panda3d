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

template<class NumType>
TypeHandle NumericDataTransition<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumericDataTransition<NumType>::
NumericDataTransition(NumType value) :
  _value(value)
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
  OnTransition(copy),
  _value(copy._value)
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
  OnTransition::operator = (copy);
  _value = copy._value;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::set_value
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void NumericDataTransition<NumType>::
set_value(NumType value) {
  _value = value;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::get_value
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType NumericDataTransition<NumType>::
get_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::set_value_from
//       Access: Public, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another NumericDataTransition.
////////////////////////////////////////////////////////////////////
template<class NumType>
void NumericDataTransition<NumType>::
set_value_from(const OnTransition *other) {
  const NumericDataTransition<NumType> *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
int NumericDataTransition<NumType>::
compare_values(const OnTransition *other) const {
  const NumericDataTransition<NumType> *ot;
  DCAST_INTO_R(ot, other, false);
  if (_value < ot->_value) {
    return -1;
  } else if (ot->_value < _value) {
    return 1;
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
template<class NumType>
void NumericDataTransition<NumType>::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
template<class NumType>
void NumericDataTransition<NumType>::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
