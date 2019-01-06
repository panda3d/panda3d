/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lsimpleMatrix.I
 * @author drose
 * @date 2011-12-15
 */

/**
 *
 */
template <class FloatType, int NumRows, int NumCols>
INLINE const FloatType &LSimpleMatrix<FloatType, NumRows, NumCols>::
operator () (int row, int col) const {
  return _array[row][col];
}

/**
 *
 */
template <class FloatType, int NumRows, int NumCols>
INLINE FloatType &LSimpleMatrix<FloatType, NumRows, NumCols>::
operator () (int row, int col) {
  return _array[row][col];
}

/**
 *
 */
template <class FloatType, int NumRows, int NumCols>
INLINE const FloatType &LSimpleMatrix<FloatType, NumRows, NumCols>::
operator () (int col) const {
  return _array[0][col];
}

/**
 *
 */
template <class FloatType, int NumRows, int NumCols>
INLINE FloatType &LSimpleMatrix<FloatType, NumRows, NumCols>::
operator () (int col) {
  return _array[0][col];
}
