// Filename: matrixAttribute.I
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#include <indent.h>

template<class Matrix>
TypeHandle MatrixAttribute<Matrix>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MatrixAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH MatrixAttribute<Matrix>::
MatrixAttribute() {
  _matrix = Matrix::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixAttribute::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH MatrixAttribute<Matrix>::
MatrixAttribute(const MatrixAttribute &copy) :
  NodeAttribute(copy),
  _matrix(copy._matrix)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixAttribute::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH void MatrixAttribute<Matrix>::
operator = (const MatrixAttribute &copy) {
  NodeAttribute::operator = (copy);
  _matrix = copy._matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixAttribute::set_matrix
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH void MatrixAttribute<Matrix>::
set_matrix(const Matrix &mat) {
  _matrix = mat;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixAttribute::get_matrix
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH const Matrix &MatrixAttribute<Matrix>::
get_matrix() const {
  return _matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixAttribute::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Matrix>
void MatrixAttribute<Matrix>::
output(ostream &out) const {
  out << _matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixAttribute::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Matrix>
void MatrixAttribute<Matrix>::
write(ostream &out, int indent_level) const {
  _matrix.write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixAttribute::internal_compare_to
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Matrix>
int MatrixAttribute<Matrix>::
internal_compare_to(const NodeAttribute *other) const {
  const MatrixAttribute *ot;
  DCAST_INTO_R(ot, other, false);

  return _matrix.compare_to(ot->_matrix);
}
