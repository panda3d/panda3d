// Filename: bitMaskAttribute.I
// Created by:  drose (08Jun00)
// 
////////////////////////////////////////////////////////////////////

#include <indent.h>

template<class MaskType>
TypeHandle BitMaskAttribute<MaskType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BitMaskAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH BitMaskAttribute<MaskType>::
BitMaskAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH BitMaskAttribute<MaskType>::
BitMaskAttribute(const MaskType &mask) :
  _mask(mask)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskAttribute::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH BitMaskAttribute<MaskType>::
BitMaskAttribute(const BitMaskAttribute &copy) :
  NodeAttribute(copy),
  _mask(copy._mask)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskAttribute::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH void BitMaskAttribute<MaskType>::
operator = (const BitMaskAttribute &copy) {
  NodeAttribute::operator = (copy);
  _mask = copy._mask;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskAttribute::set_mask
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH void BitMaskAttribute<MaskType>::
set_mask(const MaskType &mask) {
  _mask = mask;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskAttribute::get_maskType
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
INLINE_GRAPH const MaskType &BitMaskAttribute<MaskType>::
get_mask() const {
  return _mask;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskAttribute::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
void BitMaskAttribute<MaskType>::
output(ostream &out) const {
  out << _mask;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMaskAttribute::internal_compare_to
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MaskType>
int BitMaskAttribute<MaskType>::
internal_compare_to(const NodeAttribute *other) const {
  const BitMaskAttribute *ot;
  DCAST_INTO_R(ot, other, false);

  return _mask.compare_to(ot->_mask);
}
