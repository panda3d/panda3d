// Filename: vectorDataTransition.I
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

template<class VecType>
TypeHandle VectorDataTransition<VecType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType>
INLINE VectorDataTransition<VecType>::
VectorDataTransition(const VecType &value) :
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType>
INLINE VectorDataTransition<VecType>::
VectorDataTransition(const VectorDataTransition<VecType> &copy) :
  OnTransition(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType>
INLINE void VectorDataTransition<VecType>::
operator = (const VectorDataTransition<VecType> &copy) {
  OnTransition::operator = (copy);
  _value = copy._value;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::set_value
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType>
INLINE void VectorDataTransition<VecType>::
set_value(const VecType &value) {
  _value = value;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::get_value
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType>
INLINE const VecType &VectorDataTransition<VecType>::
get_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::set_value_from
//       Access: Public, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another VectorDataTransition.
////////////////////////////////////////////////////////////////////
template<class VecType>
void VectorDataTransition<VecType>::
set_value_from(const OnTransition *other) {
  const VectorDataTransition<VecType> *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType>
int VectorDataTransition<VecType>::
compare_values(const OnTransition *other) const {
  const VectorDataTransition<VecType> *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
template<class VecType>
void VectorDataTransition<VecType>::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
template<class VecType>
void VectorDataTransition<VecType>::
write_value(ostream &out, int indent_level) const {
  // Oops, this isn't defined for vectors.
  //  _value.write(out, indent_level);
  indent(out, indent_level) << _value << "\n";
}
