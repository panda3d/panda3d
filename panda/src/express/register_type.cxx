// Filename: register_type.cxx
// Created by:  drose (06Aug01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "register_type.h"


TypeHandle long_type_handle;
TypeHandle int_type_handle;
TypeHandle short_type_handle;
TypeHandle char_type_handle;
TypeHandle bool_type_handle;
TypeHandle double_type_handle;
TypeHandle float_type_handle;

TypeHandle long_p_type_handle;
TypeHandle int_p_type_handle;
TypeHandle short_p_type_handle;
TypeHandle char_p_type_handle;
TypeHandle bool_p_type_handle;
TypeHandle double_p_type_handle;
TypeHandle float_p_type_handle;
TypeHandle void_p_type_handle;

void init_system_type_handles() {
  static bool done = false;
  if (!done) {
    done = true;
    register_type(long_type_handle, "long");
    register_type(int_type_handle, "int");
    register_type(short_type_handle, "short");
    register_type(char_type_handle, "char");
    register_type(bool_type_handle, "bool");
    register_type(double_type_handle, "double");
    register_type(float_type_handle, "float");

    register_type(int_p_type_handle, "int*");
    register_type(short_p_type_handle, "short*");
    register_type(char_p_type_handle, "char*");
    register_type(bool_p_type_handle, "bool*");
    register_type(double_p_type_handle, "double*");
    register_type(float_p_type_handle, "float*");
    register_type(void_p_type_handle, "void*");
  }
}


