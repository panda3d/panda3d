// Filename: alphaTransformProperty.cxx
// Created by:  jason (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "alphaTransformProperty.h"

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformProperty::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int AlphaTransformProperty::
compare_to(const AlphaTransformProperty &other) const {
  if (_offset < other._offset)
    return -1;
  else if (_offset > other._offset)
    return 1;
  if (_scale < other._scale)
    return -1;
  else if (_scale > other._scale)
    return 1;
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformProperty::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void AlphaTransformProperty::
output(ostream &out) const {
  out << "Offset " << _offset << " Scale " << _scale << endl;
}
