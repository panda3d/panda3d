// Filename: lsimpleMatrix.I
// Created by:  drose (15Dec11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: LSimpleMatrix::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class FloatType, int NumRows, int NumCols>
INLINE LSimpleMatrix<FloatType, NumRows, NumCols>::
LSimpleMatrix() {
  // No default initialization.
}

////////////////////////////////////////////////////////////////////
//     Function: LSimpleMatrix::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class FloatType, int NumRows, int NumCols>
INLINE LSimpleMatrix<FloatType, NumRows, NumCols>::
LSimpleMatrix(const LSimpleMatrix<FloatType, NumRows, NumCols> &copy) {
  memcpy(_array, copy._array, sizeof(_array));
}

////////////////////////////////////////////////////////////////////
//     Function: LSimpleMatrix::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class FloatType, int NumRows, int NumCols>
INLINE void LSimpleMatrix<FloatType, NumRows, NumCols>::
operator = (const LSimpleMatrix<FloatType, NumRows, NumCols> &copy) {
  memcpy(_array, copy._array, sizeof(_array));
}

////////////////////////////////////////////////////////////////////
//     Function: LSimpleMatrix::operator ()
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class FloatType, int NumRows, int NumCols>
INLINE const FloatType &LSimpleMatrix<FloatType, NumRows, NumCols>::
operator () (int row, int col) const {
  return _array[row][col];
}

////////////////////////////////////////////////////////////////////
//     Function: LSimpleMatrix::operator ()
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class FloatType, int NumRows, int NumCols>
INLINE FloatType &LSimpleMatrix<FloatType, NumRows, NumCols>::
operator () (int row, int col) {
  return _array[row][col];
}

////////////////////////////////////////////////////////////////////
//     Function: LSimpleMatrix::operator ()
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class FloatType, int NumRows, int NumCols>
INLINE const FloatType &LSimpleMatrix<FloatType, NumRows, NumCols>::
operator () (int col) const {
  return _array[0][col];
}

////////////////////////////////////////////////////////////////////
//     Function: LSimpleMatrix::operator ()
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class FloatType, int NumRows, int NumCols>
INLINE FloatType &LSimpleMatrix<FloatType, NumRows, NumCols>::
operator () (int col) {
  return _array[0][col];
}
