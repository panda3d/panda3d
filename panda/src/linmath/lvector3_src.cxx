// Filename: lvector3_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LVector3)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LVector3::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LVector3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase3)::init_type();
    string name = "LVector3";
    name += FLOATTOKEN; 
    register_type(_type_handle, name, 
                  FLOATNAME(LVecBase3)::get_class_type());
  }
}


