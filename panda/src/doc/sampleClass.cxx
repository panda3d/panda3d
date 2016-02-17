/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sampleClass.cxx
 * @author drose
 * @date 2000-06-10
 */

#include "sampleClass.h"

TypeHandle SampleClass::_type_handle;

/**
 *
 */
SampleClass::
SampleClass() {
}

/**
 * A few sentences describing what public_method is supposed to do and why
 * you'd want to call it.
 */
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

/**
 * A few sentences describing what protected_method is supposed to do.
 */
bool SampleClass::
protected_method() {
  if (_flag > 0) {
    _flag--;
    return false;
  } else {
    return true;
  }
}

/**
 * A few sentences describing what private_method is supposed to do.
 */
void SampleClass::
private_method() {
}
