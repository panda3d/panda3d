// Filename: matrixTransition.T
// Created by:  drose (24Mar00)
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


template<class Matrix>
TypeHandle MatrixTransition<Matrix>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH MatrixTransition<Matrix>::
MatrixTransition() {
  _matrix = Matrix::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH MatrixTransition<Matrix>::
MatrixTransition(const Matrix &matrix) {
  _matrix = matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH MatrixTransition<Matrix>::
MatrixTransition(const MatrixTransition &copy) :
  NodeTransition(copy),
  _matrix(copy._matrix)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH void MatrixTransition<Matrix>::
operator = (const MatrixTransition &copy) {
  NodeTransition::operator = (copy);
  _matrix = copy._matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::set_matrix
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH void MatrixTransition<Matrix>::
set_matrix(const Matrix &mat) {
  _matrix = mat;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::get_matrix
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
INLINE_GRAPH const Matrix &MatrixTransition<Matrix>::
get_matrix() const {
  return _matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::compose
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
template<class Matrix>
NodeTransition *MatrixTransition<Matrix>::
compose(const NodeTransition *other) const {
  const MatrixTransition<Matrix> *ot;
  DCAST_INTO_R(ot, other, NULL);

  return make_with_matrix(ot->_matrix * _matrix);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::invert
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
NodeTransition *MatrixTransition<Matrix>::
invert() const {
  if (_matrix.almost_equal(Matrix::ident_mat())) {
    return (MatrixTransition<Matrix> *)this;
  }
  Matrix inverted;
  inverted.invert_from(_matrix);
  return make_with_matrix(inverted);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
void MatrixTransition<Matrix>::
output(ostream &out) const {
  out << _matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
void MatrixTransition<Matrix>::
write(ostream &out, int indent_level) const {
  _matrix.write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::internal_compare_to
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class Matrix>
int MatrixTransition<Matrix>::
internal_compare_to(const NodeTransition *other) const {
  const MatrixTransition<Matrix> *ot;
  DCAST_INTO_R(ot, other, false);

  // Should we bother comparing matrices componentwise, or should we
  // just assume that any two different Matrix pointers are probably
  // different matrices?

  // For now, we compare componentwise.  It makes paranoid_wrt more
  // sensible, and it doesn't seem to make a big different to
  // performance.
  return _matrix.compare_to(ot->_matrix, 0.00001f);

  // Uncomment this line instead to compare matrices pointerwise.
  //  return this - other;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::internal_generate_hash
//       Access: Protected, Virtual
//  Description: Should be overridden by particular NodeTransitions to
//               generate a hash value uniquifying this particular
//               transition.
////////////////////////////////////////////////////////////////////
template<class Matrix>
void MatrixTransition<Matrix>::
internal_generate_hash(GraphHashGenerator &hashgen) const {
  _matrix.generate_hash(hashgen, 0.00001f);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::make_with_matrix
//       Access: Protected, Virtual
//  Description: This function creates a new MatrixTransition of the
//               appropriate type, given the indicated matrix.
//
//               This is a pure virtual function and normally wouldn't
//               require a definition, but for some reason VC++
//               objects to instantiating the template if it's missing
//               any function definitions--even those for pure-virtual
//               functions.
////////////////////////////////////////////////////////////////////
template<class Matrix>
MatrixTransition<Matrix> *MatrixTransition<Matrix>::
make_with_matrix(const Matrix &) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
template<class Matrix>
void MatrixTransition<Matrix>::
write_datagram(BamWriter *manager, Datagram &me)
{
  NodeTransition::write_datagram(manager, me);
  _matrix.write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
template<class Matrix>
void MatrixTransition<Matrix>::
fillin(DatagramIterator& scan, BamReader* manager)
{
  NodeTransition::fillin(scan, manager);
  _matrix.read_datagram(scan);
}
