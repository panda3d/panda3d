// Filename: lvector4_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LVector4)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LVector2::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LVector4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase4)::init_type();
    string name = "LVector4";
    name += FLOATTOKEN; 
    register_type(_type_handle, name, 
		  FLOATNAME(LVecBase4)::get_class_type());
  }
}


