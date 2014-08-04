// Filename: register_type.cxx
// Created by:  drose (06Aug01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "register_type.h"


TypeHandle long_type_handle;
TypeHandle int_type_handle;
TypeHandle uint_type_handle;
TypeHandle short_type_handle;
TypeHandle ushort_type_handle;
TypeHandle char_type_handle;
TypeHandle uchar_type_handle;
TypeHandle bool_type_handle;
TypeHandle double_type_handle;
TypeHandle float_type_handle;
TypeHandle string_type_handle;
TypeHandle wstring_type_handle;

TypeHandle long_p_type_handle;
TypeHandle int_p_type_handle;
TypeHandle short_p_type_handle;
TypeHandle char_p_type_handle;
TypeHandle bool_p_type_handle;
TypeHandle double_p_type_handle;
TypeHandle float_p_type_handle;
TypeHandle void_p_type_handle;

TypeHandle pvector_type_handle;
TypeHandle ov_set_type_handle;
TypeHandle pdeque_type_handle;
TypeHandle plist_type_handle;
TypeHandle pmap_type_handle;
TypeHandle pset_type_handle;

void init_system_type_handles() {
  static bool done = false;
  if (!done) {
    done = true;
    register_type(long_type_handle, "long");
    register_type(int_type_handle, "int");
    register_type(uint_type_handle, "uint");
    register_type(short_type_handle, "short");
    register_type(ushort_type_handle, "ushort");
    register_type(char_type_handle, "char");
    register_type(uchar_type_handle, "uchar");
    register_type(bool_type_handle, "bool");
    register_type(double_type_handle, "double");
    register_type(float_type_handle, "float");
    register_type(string_type_handle, "string");
    register_type(wstring_type_handle, "wstring");

    register_type(int_p_type_handle, "int*");
    register_type(short_p_type_handle, "short*");
    register_type(char_p_type_handle, "char*");
    register_type(bool_p_type_handle, "bool*");
    register_type(double_p_type_handle, "double*");
    register_type(float_p_type_handle, "float*");
    register_type(void_p_type_handle, "void*");

    register_type(pvector_type_handle, "pvector");
    register_type(ov_set_type_handle, "ov_set");
    register_type(pdeque_type_handle, "pdeque");
    register_type(plist_type_handle, "plist");
    register_type(pmap_type_handle, "pmap");
    register_type(pset_type_handle, "pset");
  }
}
