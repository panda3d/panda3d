/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeManager.h
 * @author drose
 * @date 2000-08-14
 */

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

/**
 * This is just a collection of static methods that perform useful operations
 * on CPPTypes for interrogate.  The class is really just a namespace that
 * groups these functions together.
 */
class TypeManager {
public:

  static CPPType *resolve_type(CPPType *type, CPPScope *scope = nullptr);

  static bool is_assignable(CPPType *type);

  static bool is_reference(CPPType *type);
  static bool is_rvalue_reference(CPPType *type);
  static bool is_ref_to_anything(CPPType *type);
  static bool is_const_ref_to_anything(CPPType *type);
  static bool is_const_pointer_to_anything(CPPType *type);
  static bool is_const_pointer_or_ref(CPPType *type);
  static bool is_non_const_pointer_or_ref(CPPType *type);
  static bool is_pointer(CPPType *type);
  static bool is_const(CPPType *type);
  static bool is_struct(CPPType *type);
  static bool is_scoped_enum(CPPType *type);
  static bool is_enum(CPPType *type);
  static bool is_const_enum(CPPType *type);
  static bool is_const_ref_to_enum(CPPType *type);
  static bool is_nullptr(CPPType *type);
  static bool is_simple(CPPType *type);
  static bool is_const_simple(CPPType *type);
  static bool is_const_ref_to_simple(CPPType *type);
  static bool is_ref_to_simple(CPPType *type);
  static bool is_simple_array(CPPType *type);
  static bool is_pointer_to_simple(CPPType *type);
  static bool is_pointable(CPPType *type);
  static bool is_char(CPPType *type);
  static bool is_unsigned_char(CPPType *type);
  static bool is_signed_char(CPPType *type);
  static bool is_char_pointer(CPPType *type);
  static bool is_const_char_pointer(CPPType *type);
  static bool is_unsigned_char_pointer(CPPType *type);
  static bool is_const_unsigned_char_pointer(CPPType *type);
  static bool is_basic_string_char(CPPType *type);
  static bool is_const_basic_string_char(CPPType *type);
  static bool is_const_ref_to_basic_string_char(CPPType *type);
  static bool is_const_ptr_to_basic_string_char(CPPType *type);
  static bool is_string(CPPType *type);
  static bool is_wchar(CPPType *type);
  static bool is_wchar_pointer(CPPType *type);
  static bool is_basic_string_wchar(CPPType *type);
  static bool is_const_basic_string_wchar(CPPType *type);
  static bool is_const_ref_to_basic_string_wchar(CPPType *type);
  static bool is_const_ptr_to_basic_string_wchar(CPPType *type);
  static bool is_wstring(CPPType *type);
  static bool is_vector_unsigned_char(CPPType *type);
  static bool is_const_vector_unsigned_char(CPPType *type);
  static bool is_pair(CPPType *type);
  static bool is_bool(CPPType *type);
  static bool is_integer(CPPType *type);
  static bool is_unsigned_integer(CPPType *type);
  static bool is_size(CPPType *type);
  static bool is_ssize(CPPType *type);
  static bool is_long(CPPType *type);
  static bool is_short(CPPType *type);
  static bool is_unsigned_short(CPPType *type);
  static bool is_longlong(CPPType *type);
  static bool is_unsigned_longlong(CPPType *type);
  static bool is_double(CPPType *type);
  static bool is_float(CPPType *type);
  static bool is_void(CPPType *type);
  static bool is_reference_count(CPPType *type);
  static bool is_reference_count_pointer(CPPType *type);
  static bool is_pointer_to_base(CPPType *type);
  static bool is_const_pointer_to_base(CPPType *type);
  static bool is_const_ref_to_pointer_to_base(CPPType *type);
  static bool is_pointer_to_PyObject(CPPType *type);
  static bool is_PyObject(CPPType *type);
  static bool is_pointer_to_PyTypeObject(CPPType *type);
  static bool is_PyTypeObject(CPPType *type);
  static bool is_pointer_to_PyStringObject(CPPType *type);
  static bool is_PyStringObject(CPPType *type);
  static bool is_pointer_to_PyUnicodeObject(CPPType *type);
  static bool is_PyUnicodeObject(CPPType *type);
  static bool is_pointer_to_Py_buffer(CPPType *type);
  static bool is_Py_buffer(CPPType *type);
  static bool is_handle(CPPType *type);
  static bool involves_unpublished(CPPType *type);
  static bool involves_protected(CPPType *type);
  static bool involves_rvalue_reference(CPPType *type);

  static bool is_ostream(CPPType *type);
  static bool is_pointer_to_ostream(CPPType *type);

  static CPPType *unwrap_pointer(CPPType *type);
  static CPPType *unwrap_reference(CPPType *type);
  static CPPType *unwrap_const(CPPType *type);
  static CPPType *unwrap_const_reference(CPPType *type);
  static CPPType *unwrap(CPPType *type);

  static CPPType *get_pointer_type(CPPStructType *pt_type);
  static CPPType *get_template_parameter_type(CPPType *type, int i = 0);

  static CPPType *wrap_pointer(CPPType *type);
  static CPPType *wrap_const_pointer(CPPType *type);
  static CPPType *wrap_const_reference(CPPType *type);

  static CPPType *get_basic_string_char_type();
  static CPPType *get_basic_string_wchar_type();
  static CPPType *get_reference_count_type();
  static CPPType *get_void_type();
  static CPPType *get_int_type();

  static std::string get_function_signature(CPPInstance *function,
                                       int num_default_parameters = 0);

  static std::string get_function_name(CPPInstance *function);

  static bool has_protected_destructor(CPPType *type);


  static bool is_exported(CPPType *type);
  static bool is_local(CPPType *type);
};

#endif
