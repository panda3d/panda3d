// Filename: texMatrixTransition.cxx
// Created by:  mike (19Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "texMatrixTransition.h"
#include "texMatrixAttribute.h"

#include <indent.h>

TypeHandle TexMatrixTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexMatrixTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *TexMatrixTransition::
make_copy() const {
  return new TexMatrixTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexMatrixAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *TexMatrixTransition::
make_attrib() const {
  return new TexMatrixAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixTransition::make_with_matrix
//       Access: Protected, Virtual
//  Description: Returns a new transition with the indicated matrix.
////////////////////////////////////////////////////////////////////
MatrixTransition<LMatrix4f> *TexMatrixTransition::
make_with_matrix(const LMatrix4f &matrix) const {
  return new TexMatrixTransition(matrix);
}
