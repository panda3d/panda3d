// Filename: typeManager.h
// Created by:  drose (14Aug00)
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

#ifndef TYPEMANAGER_H
#define TYPEMANAGER_H

#include "dtoolbase.h"

class CPPFunctionGroup;
class CPPInstance;
class CPPType;
class CPPSimpleType;
class CPPPointerType;
class CPPConstType;
class CPPExtensionType;
class CPPStructType;
class CPPEnumType;
class CPPFunctionType;
class CPPScope;
class CPPIdentifier;
class CPPNameComponent;
class CPPManifest;

////////////////////////////////////////////////////////////////////
//       Class : TypeManager
// Description : This is just a collection of static methods that
//               perform useful operations on CPPTypes for
//               interrogate.  The class is really just a namespace
//               that groups these functions together.
////////////////////////////////////////////////////////////////////
class TypeManager {
public:

  static CPPType *resolve_type(CPPType *type, CPPScope *scope = (CPPScope *)NULL);

  static bool is_assignable(CPPType *type);

  static bool is_reference(CPPType *type);
  static bool is_ref_to_anything(CPPType *type);
  static bool is_const_ref_to_anything(CPPType *type);
  static bool is_pointer(CPPType *type);
  static bool is_const(CPPType *type);
  static bool is_struct(CPPType *type);
  static bool is_enum(CPPType *type);
  static bool is_const_enum(CPPType *type);
  static bool is_const_ref_to_enum(CPPType *type);
  static bool is_simple(CPPType *type);
  static bool is_const_simple(CPPType *type);
  static bool is_const_ref_to_simple(CPPType *type);
  static bool is_pointable(CPPType *type);
  static bool is_char(CPPType *type);
  static bool is_char_pointer(CPPType *type);
  static bool is_basic_string_char(CPPType *type);
  static bool is_const_basic_string_char(CPPType *type);
  static bool is_const_ref_to_basic_string_char(CPPType *type);
  static bool is_bool(CPPType *type);
  static bool is_integer(CPPType *type);
  static bool is_unsigned_longlong(CPPType *type);
  static bool is_longlong(CPPType *type);
  static bool is_float(CPPType *type);
  static bool is_void(CPPType *type);
  static bool is_reference_count(CPPType *type);
  static bool is_reference_count_pointer(CPPType *type);
  static bool is_pointer_to_base(CPPType *type);
  static bool is_const_pointer_to_base(CPPType *type);
  static bool is_const_ref_to_pointer_to_base(CPPType *type);
  static bool is_pointer_to_PyObject(CPPType *type);
  static bool is_PyObject(CPPType *type);
  static bool involves_unpublished(CPPType *type);
  static bool involves_protected(CPPType *type);

  static bool is_ostream(CPPType *type);
  static bool is_pointer_to_ostream(CPPType *type);

  static CPPType *unwrap_pointer(CPPType *type);
  static CPPType *unwrap_reference(CPPType *type);
  static CPPType *unwrap_const(CPPType *type);
  static CPPType *unwrap_const_reference(CPPType *type);
  static CPPType *unwrap(CPPType *type);

  static CPPType *get_pointer_type(CPPStructType *pt_type);

  static CPPType *wrap_pointer(CPPType *type);
  static CPPType *wrap_const_pointer(CPPType *type);
  static CPPType *wrap_const_reference(CPPType *type);

  static CPPType *get_basic_string_char_type();
  static CPPType *get_reference_count_type();
  static CPPType *get_void_type();
  static CPPType *get_int_type();

  static string get_function_signature(CPPInstance *function,
                                       int num_default_parameters = 0);

  static string get_function_name(CPPInstance *function);

  static bool has_protected_destructor(CPPType *type);


  static bool IsExported(CPPType *type);
  static bool IsLocal(CPPType *type);

};

#endif

