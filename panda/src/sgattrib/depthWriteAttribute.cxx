// Filename: depthWriteAttribute.cxx
// Created by:  drose (31Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "depthWriteAttribute.h"
#include "depthWriteTransition.h"

#include <graphicsStateGuardianBase.h>

TypeHandle DepthWriteAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle DepthWriteAttribute::
get_handle() const {
  return DepthWriteTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthWriteAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *DepthWriteAttribute::
make_copy() const {
  return new DepthWriteAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthWriteAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *DepthWriteAttribute::
make_initial() const {
  return new DepthWriteAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void DepthWriteAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_depth_write(this);
}
