// Filename: clipPlaneAttribute.cxx
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "clipPlaneAttribute.h"
#include "clipPlaneTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle ClipPlaneAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle ClipPlaneAttribute::
get_handle() const {
  return ClipPlaneTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ClipPlaneAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *ClipPlaneAttribute::
make_copy() const {
  return new ClipPlaneAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ClipPlaneAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *ClipPlaneAttribute::
make_initial() const {
  return new ClipPlaneAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_clip_plane(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttribute::
output_property(ostream &out, const PT_Node &prop) const {
  const PlaneNode *node;
  DCAST_INTO_V(node, prop);
  out << node->get_name() << "=" << node->get_plane();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttribute::
write_property(ostream &out, const PT_Node &prop,
               int indent_level) const {
  const PlaneNode *node;
  DCAST_INTO_V(node, prop);
  indent(out, indent_level)
    << node->get_name() << "=" << node->get_plane() << "\n";
}
