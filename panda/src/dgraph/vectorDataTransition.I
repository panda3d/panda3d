// Filename: vectorDataTransition.I
// Created by:  drose (25Jan99)
// 
////////////////////////////////////////////////////////////////////

/* okcircular */
#include "vectorDataAttribute.h"

template<class VecType, class MatType>
TypeHandle VectorDataTransition<VecType, MatType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE VectorDataTransition<VecType, MatType>::
VectorDataTransition() {
  _matrix = MatType::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE VectorDataTransition<VecType, MatType>::
VectorDataTransition(const MatType &matrix) : 
  _matrix(matrix)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE VectorDataTransition<VecType, MatType>::
VectorDataTransition(const VectorDataTransition<VecType, MatType> &copy) :
  NodeTransition(copy),
  _matrix(copy._matrix)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE void VectorDataTransition<VecType, MatType>::
operator = (const VectorDataTransition<VecType, MatType> &copy) {
  NodeTransition::operator = (copy);
  _matrix = copy._matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::is_identity
//       Access: Public, Virtual
//  Description: Returns true if this transition does not affect any
//               numbers going through it.
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
INLINE bool VectorDataTransition<VecType, MatType>::
is_identity() const {
  return _matrix.almost_equal(MatType::ident_mat());
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::set_matrix
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
void VectorDataTransition<VecType, MatType>::
set_matrix(const MatType &matrix) {
  _matrix = matrix;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::get_matrix
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
const MatType &VectorDataTransition<VecType, MatType>::
get_matrix() const {
  return _matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::compose
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
template<class VecType, class MatType>
NodeTransition *VectorDataTransition<VecType, MatType>::
compose(const NodeTransition *other) const {
  const VectorDataTransition<VecType, MatType> *ot;
  DCAST_INTO_R(ot, other, NULL);

  if (is_identity()) {
    return (VectorDataTransition<VecType, MatType> *)ot;

  } else if (ot->is_identity()) {
    return (VectorDataTransition<VecType, MatType> *)this;

  } else {
    NodeTransition *c = make_copy();
    VectorDataTransition<VecType, MatType> *result;
    DCAST_INTO_R(result, c, NULL);

    result->_matrix = ot->_matrix * _matrix;
    return result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::invert
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
NodeTransition *VectorDataTransition<VecType, MatType>::
invert() const {
  if (is_identity()) {
    return (VectorDataTransition<VecType, MatType> *)this;

  } else {
    PT(NodeTransition) c = make_copy();
    VectorDataTransition<VecType, MatType> *result;
    DCAST_INTO_R(result, c, NULL);
 
    bool invertible = result->_matrix.invert_from(_matrix);
    if (!invertible) {
      return NULL;
    }
    return result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::apply
//       Access: Public, Virtual
//  Description: Returns a new attribute (or possibly the same
//               attribute) that represents the effect of applying this
//               indicated transition to the indicated attribute.  The
//               source attribute may be NULL, indicating the initial
//               attribute.
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
NodeAttribute *VectorDataTransition<VecType, MatType>::
apply(const NodeAttribute *attrib) const {
  const VectorDataAttribute<VecType, MatType> *at;
  DCAST_INTO_R(at, attrib, NULL);

  if (is_identity()) {
    return (VectorDataAttribute<VecType, MatType> *)attrib;
  }

  NodeAttribute *c = make_attrib();
  VectorDataAttribute<VecType, MatType> *result;
  DCAST_INTO_R(result, c, NULL);

  result->_value = at->_value * _matrix;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
void VectorDataTransition<VecType, MatType>::
output(ostream &out) const {
  out << _matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
void VectorDataTransition<VecType, MatType>::
write(ostream &out, int indent_level) const {
  _matrix.write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: VectorDataTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
int VectorDataTransition<VecType, MatType>::
internal_compare_to(const NodeTransition *other) const {
  const VectorDataTransition<VecType, MatType> *ot;
  DCAST_INTO_R(ot, other, false);

  return _matrix.compare_to(ot->_matrix);
}
