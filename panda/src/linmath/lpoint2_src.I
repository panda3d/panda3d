// Filename: lpoint2.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LPoint2)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LPoint2)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase2)::init_type();
    string name =
      "LPoint2<" + get_type_handle(FLOATTYPE1).get_name() + ">";
    register_type(_type_handle, name, 
		  FLOATNAME(LVecBase2)::get_class_type());
  }
}


