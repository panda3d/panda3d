// Filename: vectorDataAttribute.I
// Created by:  drose (27Mar00)
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
#include "vectorDataTransition.h"

template<class VecType, class MatType>
TypeHandle VectorDataAttribute<VecType, MatType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE VectorDataAttribute<VecType, MatType>::
VectorDataAttribute() {
  _value = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE VectorDataAttribute<VecType, MatType>::
VectorDataAttribute(const VecType &value) :
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE VectorDataAttribute<VecType, MatType>::
VectorDataAttribute(const VectorDataAttribute<VecType, MatType> &copy) :
  NodeAttribute(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE void VectorDataAttribute<VecType, MatType>::
operator = (const VectorDataAttribute<VecType, MatType> &copy) {
  NodeAttribute::operator = (copy);
  _value = copy._value;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::set_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE void VectorDataAttribute<VecType, MatType>::
set_value(const VecType &value) {
  _value = value;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::get_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE const VecType &VectorDataAttribute<VecType, MatType>::
get_value() const {
  return _value;
}


////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::get_handle
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
TypeHandle VectorDataAttribute<VecType, MatType>::
get_handle() const {
  return VectorDataTransition<VecType, MatType>::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
void VectorDataAttribute<VecType, MatType>::
output(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
void VectorDataAttribute<VecType, MatType>::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataAttribute::internal_compare_to
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
int VectorDataAttribute<VecType, MatType>::
internal_compare_to(const NodeAttribute *other) const {
  const VectorDataAttribute<VecType, MatType> *ot;
  DCAST_INTO_R(ot, other, false);

  return _value.compare_to(ot->_value);
}
