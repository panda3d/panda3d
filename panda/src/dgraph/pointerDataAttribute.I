// Filename: pointerDataAttribute.I
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

/* okcircular */
#include "pointerDataTransition.h"

template<class PtrType>
TypeHandle PointerDataAttribute<PtrType>::_type_handle;

template<class PtrType>
PtrType* PointerDataAttribute<PtrType>::_null_ptr = (PtrType*)NULL;

////////////////////////////////////////////////////////////////////
//     Function: PointerDataAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
INLINE PointerDataAttribute<PtrType>::
PointerDataAttribute() :
  _ptr(_null_ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
INLINE PointerDataAttribute<PtrType>::
PointerDataAttribute(PtrType* ptr) :
  _ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataAttribute::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
INLINE PointerDataAttribute<PtrType>::
PointerDataAttribute(const PointerDataAttribute<PtrType> &copy) :
  NodeAttribute(copy), _ptr(copy._ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataAttribute::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
INLINE void PointerDataAttribute<PtrType>::
operator = (const PointerDataAttribute<PtrType> &copy) {
  NodeAttribute::operator = (copy);
  _ptr = copy._ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataAttribute::set_value
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
void PointerDataAttribute<PtrType>::
set_value(PtrType* ptr) {
  _ptr = ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataAttribute::get_value
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
PtrType* PointerDataAttribute<PtrType>::
get_value() const {
  return _ptr;
}


////////////////////////////////////////////////////////////////////
//     Function: NumericDataAttribute::get_handle
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
TypeHandle PointerDataAttribute<PtrType>::
get_handle() const {
  return PointerDataTransition<PtrType>::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataAttribute::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
void PointerDataAttribute<PtrType>::
output(ostream &out) const {
  out << (void*)_ptr << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataAttribute::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
void PointerDataAttribute<PtrType>::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << (void*)_ptr << endl;
}

///////////////////////////////////////////////////////////////////
//     Function: PointerDataAttribute::internal_compare_to
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
int PointerDataAttribute<PtrType>::
internal_compare_to(const NodeAttribute *other) const {
  const PointerDataAttribute<PtrType> *ot;
  DCAST_INTO_R(ot, other, false);

  if (_ptr != ot->_ptr) {
    return (_ptr > ot->_ptr) ? 1 : -1;
  }

  return 0;
}
