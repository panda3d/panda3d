// Filename: lrotation_src.h
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LRotation)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LRotation::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void FLOATNAME(LRotation)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LQuaternion)::init_type();
    // Format a string to describe the type.
    string name = "LRotation";
    name += FLOATTOKEN; 
    register_type(_type_handle, name, 
		  FLOATNAME(LQuaternion)::get_class_type());
  }
}

