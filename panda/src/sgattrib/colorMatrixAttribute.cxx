// Filename: colorMatrixAttribute.cxx
// Created by:  jason (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "colorMatrixAttribute.h"
#include "colorMatrixTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle ColorMatrixAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle ColorMatrixAttribute::
get_handle() const {
  return ColorMatrixTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMatrixAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorMatrixAttribute::
make_copy() const {
  return new ColorMatrixAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMatrixAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorMatrixAttribute::
make_initial() const {
  return new ColorMatrixAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void ColorMatrixAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_color_transform(this);
}
