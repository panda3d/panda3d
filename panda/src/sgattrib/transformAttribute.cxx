// Filename: transformAttribute.cxx
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "transformAttribute.h"
#include "transformTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle TransformAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransformAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle TransformAttribute::
get_handle() const {
  return TransformTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TransformAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *TransformAttribute::
make_copy() const {
  return new TransformAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TransformAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *TransformAttribute::
make_initial() const {
  return new TransformAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TransformAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_transform(this);
}
