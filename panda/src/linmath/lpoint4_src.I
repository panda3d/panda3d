// Filename: lpoint4.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LPoint4)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LPoint4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase4)::init_type();
    string name =
      "LPoint4<" + get_type_handle(FLOATTYPE1).get_name() + ">";
    register_type(_type_handle, name, 
		  FLOATNAME(LVecBase4)::get_class_type());
  }
}


