// Filename: texMatrixAttribute.cxx
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "texMatrixAttribute.h"
#include "texMatrixTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle TexMatrixAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle TexMatrixAttribute::
get_handle() const {
  return TexMatrixTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexMatrixAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *TexMatrixAttribute::
make_copy() const {
  return new TexMatrixAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexMatrixAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *TexMatrixAttribute::
make_initial() const {
  return new TexMatrixAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TexMatrixAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_tex_matrix(this);
}
