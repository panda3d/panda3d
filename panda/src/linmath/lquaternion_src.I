
TypeHandle FLOATNAME(LQuaternionBase)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Destructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

FLOATNAME(LQuaternionBase)::
~FLOATNAME(LQuaternionBase)() {
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::pure_imaginary_quat
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

FLOATNAME(LQuaternionBase) FLOATNAME(LQuaternionBase)::
pure_imaginary(const FLOATNAME(LVector3) &v) {
  return FLOATNAME(LQuaternionBase)(0, v[0], v[1], v[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::ident_quat
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

const FLOATNAME(LQuaternionBase) &FLOATNAME(LQuaternionBase)::
ident_quat(void) {
  static FLOATNAME(LQuaternionBase) q(1, 0, 0, 0);
  return q;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::init_type
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LQuaternionBase)::
init_type(void) {
  if (_type_handle == TypeHandle::none()) {
    do_init_type(FLOATTYPE1);
    string name = "FLOATNAME(LQuaternionBase)<" + get_type_handle(FLOATTYPE1).get_name()
                  + ">";
    register_type(_type_handle, name);
  }
}
