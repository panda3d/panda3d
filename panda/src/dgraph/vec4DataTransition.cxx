// Filename: vec4DataTransition.cxx
// Created by:  jason (03Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "vec4DataTransition.h"
#include "vec4DataAttribute.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle Vec4DataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Vec4DataTransition::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransition *Vec4DataTransition::
make_copy() const {
  return new Vec4DataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Vec4DataTransition::make_attrib
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *Vec4DataTransition::
make_attrib() const {
  return new Vec4DataAttribute;
}
