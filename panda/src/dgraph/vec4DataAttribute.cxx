// Filename: vec4DataAttribute.cxx
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "vec4DataAttribute.h"
#include "vec4DataTransition.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle Vec4DataAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Vec4DataAttribute::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *Vec4DataAttribute::
make_copy() const {
  return new Vec4DataAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Vec4DataAttribute::make_initial
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *Vec4DataAttribute::
make_initial() const {
  return new Vec4DataAttribute;
}
