// Filename: decalAttribute.cxx
// Created by:  drose (17Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "decalAttribute.h"
#include "decalTransition.h"

#include <graphicsStateGuardianBase.h>

TypeHandle DecalAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DecalAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle DecalAttribute::
get_handle() const {
  return DecalTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DecalAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *DecalAttribute::
make_copy() const {
  return new DecalAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated DecalAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *DecalAttribute::
make_initial() const {
  return new DecalAttribute;
}
