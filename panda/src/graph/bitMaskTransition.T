// Filename: bitMaskTransition.I
// Created by:  drose (08Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "bitMaskAttribute.h"

template<class MaskType>
TypeHandle BitMaskTransition<MaskType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH BitMaskTransition<MaskType>::
BitMaskTransition() {
  _and = MaskType::all_on();
  _or = MaskType::all_off();
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH BitMaskTransition<MaskType>::
BitMaskTransition(const MaskType &and, const MaskType &or) {
  _and = and;
  _or = or;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH BitMaskTransition<MaskType>::
BitMaskTransition(const BitMaskTransition &copy) :
  NodeTransition(copy),
  _and(copy._and),
  _or(copy._or)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH void BitMaskTransition<MaskType>::
operator = (const BitMaskTransition &copy) {
  NodeTransition::operator = (copy);
  _and = copy._and;
  _or = copy._or;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::set_and
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH void BitMaskTransition<MaskType>::
set_and(const MaskType &and) {
  _and = and;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::get_and
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH const MaskType &BitMaskTransition<MaskType>::
get_and() const {
  return _and;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::set_or
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH void BitMaskTransition<MaskType>::
set_or(const MaskType &or) {
  _or = or;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::get_or
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH const MaskType &BitMaskTransition<MaskType>::
get_or() const {
  return _or;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::compose
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
template<class MaskType>
NodeTransition *BitMaskTransition<MaskType>::
compose(const NodeTransition *other) const {
  const BitMaskTransition<MaskType> *ot;
  DCAST_INTO_R(ot, other, NULL);

  if (ot->_priority < _priority) {
    // The other transition is too low-priority; the result is
    // unchanged.
    return (BitMaskTransition<MaskType> *)this;
  }

  return make_with_masks(ot->_and & _and, ot->_or | _or);
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::invert
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
NodeTransition *BitMaskTransition<MaskType>::
invert() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::apply
//       Access: Public, Virtual
//  Description: Returns a new attribute (or possibly the same
//               attribute) that represents the effect of applying this
//               indicated transition to the indicated attribute.  The
//               source attribute may be NULL, indicating the initial
//               attribute.
////////////////////////////////////////////////////////////////////
template<class MaskType>
NodeAttribute *BitMaskTransition<MaskType>::
apply(const NodeAttribute *attrib) const {
  BitMaskAttribute<MaskType> *result;
  DCAST_INTO_R(result, make_attrib(), NULL);

  if (attrib == (const NodeAttribute *)NULL) {
    // If there is no root attribute, the default attribute is always
    // all bits on.
    result->_mask = (MaskType::all_on() & _and) | _or;
    return result;
  }

  if (_priority < result->_priority) {
    // The priority is too low to affect the attribute.
    return result;
  }

  const BitMaskAttribute<MaskType> *at;
  DCAST_INTO_R(at, attrib, NULL);

  result->_mask = (at->_mask & _and) | _or;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
void BitMaskTransition<MaskType>::
output(ostream &out) const {
  out << "&";
  _and.output_hex(out);
  out << " |";
  _or.output_hex(out);
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::internal_compare_to
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
int BitMaskTransition<MaskType>::
internal_compare_to(const NodeTransition *other) const {
  const BitMaskTransition<MaskType> *ot;
  DCAST_INTO_R(ot, other, false);

  int result = _and.compare_to(ot->_and);
  if (result == 0) {
    result = _or.compare_to(ot->_or);
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskTransition::make_with_masks
//       Access: Protected, Virtual
//  Description: This function creates a new BitMaskTransition of the
//               appropriate type, given the indicated and & or masks.
//
//               This is a pure virtual function and normally wouldn't
//               require a definition, but for some reason VC++
//               objects to instantiating the template if it's missing
//               any function definitions--even those for pure-virtual
//               functions.
////////////////////////////////////////////////////////////////////
template<class MaskType>
BitMaskTransition<MaskType> *BitMaskTransition<MaskType>::
make_with_masks(const MaskType &, const MaskType &) const {
  return NULL;
}
