// Filename: lorientation_src.h
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LOrientation)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void FLOATNAME(LOrientation)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LQuaternion)::init_type();
    string name = "LOrientation";
    name += FLOATTOKEN; 
    register_type(_type_handle, name, 
		  FLOATNAME(LQuaternion)::get_class_type());
  }
}
