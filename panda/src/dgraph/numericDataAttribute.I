// Filename: numericDataAttribute.I
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "numericDataTransition.h"

#include <indent.h>

template<class NumType>
TypeHandle NumericDataAttribute<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumericDataAttribute<NumType>::
NumericDataAttribute() {
  _value = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumericDataAttribute<NumType>::
NumericDataAttribute(NumType value) :
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumericDataAttribute<NumType>::
NumericDataAttribute(const NumericDataAttribute<NumType> &copy) :
  NodeAttribute(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void NumericDataAttribute<NumType>::
operator = (const NumericDataAttribute<NumType> &copy) {
  NodeAttribute::operator = (copy);
  _value = copy._value;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::set_value
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void NumericDataAttribute<NumType>::
set_value(NumType value) {
  _value = value;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::get_value
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType NumericDataAttribute<NumType>::
get_value() const {
  return _value;
}


////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::get_handle
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
TypeHandle NumericDataAttribute<NumType>::
get_handle() const {
  return NumericDataTransition<NumType>::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void NumericDataAttribute<NumType>::
output(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void NumericDataAttribute<NumType>::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::internal_compare_to
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
int NumericDataAttribute<NumType>::
internal_compare_to(const NodeAttribute *other) const {
  const NumericDataAttribute<NumType> *ot;
  DCAST_INTO_R(ot, other, false);

  if (_value != ot->_value) {
    return (_value < ot->_value) ? -1 : 1;
  }
  return 0;
}
