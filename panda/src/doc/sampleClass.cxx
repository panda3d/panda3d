// Filename: sampleClass.cxx
// Created by:  drose (10Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "sampleClass.h"

TypeHandle SampleClass::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SampleClass::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SampleClass::
SampleClass() {
}

////////////////////////////////////////////////////////////////////
//     Function: SampleClass::public_method
//       Access: Public
//  Description: A few sentences describing what public_method is
//               supposed to do and why you'd want to call it.
////////////////////////////////////////////////////////////////////
int SampleClass::
public_method() {
  switch (_private_data_member) {
  case NE_case_one:
    return 0;

  case NE_case_two:
    return _flag;

  default:
    return -1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SampleClass::protected_method
//       Access: Protected
//  Description: A few sentences describing what protected_method is
//               supposed to do.
////////////////////////////////////////////////////////////////////
bool SampleClass::
protected_method() {
  if (_flag > 0) {
    _flag--;
    return false;
  } else {
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SampleClass::private_method
//       Access: Private
//  Description: A few sentences describing what private_method is
//               supposed to do.
////////////////////////////////////////////////////////////////////
void SampleClass::
private_method() {
}
