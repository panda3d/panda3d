// Filename: pointerDataTransition.I
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "pointerDataAttribute.h"

#include <indent.h>

template<class PtrType>
TypeHandle PointerDataTransition<PtrType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
INLINE PointerDataTransition<PtrType>::
PointerDataTransition() :
  _ptr(PointerDataAttribute<PtrType>::_null_ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
INLINE PointerDataTransition<PtrType>::
PointerDataTransition(PtrType* ptr) :
  _ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
INLINE PointerDataTransition<PtrType>::
PointerDataTransition(const PointerDataTransition<PtrType> &copy) :
  NodeTransition(copy), _ptr(copy._ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
INLINE void PointerDataTransition<PtrType>::
operator = (const PointerDataTransition<PtrType> &copy) {
  NodeTransition::operator = (copy);
  _ptr = copy._ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::set_value
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
void PointerDataTransition<PtrType>::
set_value(PtrType* ptr) {
  _ptr = ptr;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::get_value
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
PtrType* PointerDataTransition<PtrType>::
get_value() const {
  return _ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::compose
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
template<class PtrType>
NodeTransition *PointerDataTransition<PtrType>::
compose(const NodeTransition *other) const {
  const PointerDataTransition<PtrType> *ot;
  DCAST_INTO_R(ot, other, NULL);

  return (NodeTransition*)other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::invert
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
NodeTransition *PointerDataTransition<PtrType>::
invert() const {
  return (NodeTransition*)this;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::apply
//       Access: Public, Virtual
//  Description: Returns a new attribute (or possibly the same
//               attribute) that represents the effect of applying this
//               indicated transition to the indicated attribute.  The
//               source attribute may be NULL, indicating the initial
//               attribute.
////////////////////////////////////////////////////////////////////
template<class PtrType>
NodeAttribute *PointerDataTransition<PtrType>::
apply(const NodeAttribute *attrib) const {
  const PointerDataAttribute<PtrType> *at;
  DCAST_INTO_R(at, attrib, NULL);

  return (NodeAttribute*)attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
void PointerDataTransition<PtrType>::
output(ostream &out) const {
  out << (void*)_ptr << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
void PointerDataTransition<PtrType>::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << (void*)_ptr << endl;
}

///////////////////////////////////////////////////////////////////
//     Function: PointerDataTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class PtrType>
int PointerDataTransition<PtrType>::
internal_compare_to(const NodeTransition *other) const {
  const PointerDataTransition<PtrType> *ot;
  DCAST_INTO_R(ot, other, false);

  if (_ptr != ot->_ptr) {
    return (_ptr > ot->_ptr) ? 1 : -1;
  }

  return 0;
}










