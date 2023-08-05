/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeManager.cxx
 * @author drose
 * @date 2000-08-14
 */

#include "typeManager.h"
#include "interrogate.h"

#include "cppArrayType.h"
#include "cppConstType.h"
#include "cppEnumType.h"
#include "cppFunctionGroup.h"
#include "cppFunctionType.h"
#include "cppIdentifier.h"
#include "cppParameterList.h"
#include "cppPointerType.h"
#include "cppReferenceType.h"
#include "cppSimpleType.h"
#include "cppStructType.h"
#include "cppTemplateScope.h"
#include "cppTypeDeclaration.h"
#include "cppTypedefType.h"
#include "pnotify.h"

using std::string;

/**
 * A horrible hack around a CPPParser bug.  We don't trust the CPPType pointer
 * we were given; instead, we ask CPPParser to parse a new type of the same
 * name.  This has a better chance of fully resolving templates.
 */
CPPType *TypeManager::
resolve_type(CPPType *type, CPPScope *scope) {
  if (scope == nullptr) {
    scope = &parser;
  }

  //CPPType *orig_type = type;
  type = type->resolve_type(scope, &parser);
  string name = type->get_local_name(&parser);
  if (name.empty()) {
    // Don't try to resolve unnamed types.
    return type;
  }

  // I think I fixed the bug; no need for the below hack any more.
  return type;

/*
  CPPType *new_type = parser.parse_type(name);
  if (new_type == (CPPType *)NULL) {
    nout << "Type \"" << name << "\" (from " << *orig_type << ") is unknown to parser.\n";
  } else {
    type = new_type->resolve_type(&parser, &parser);
  }

  return type;
*/
}

/**
 * Returns true if the indicated type is something we can legitimately assign
 * a value to, or false otherwise.
 */
bool TypeManager::
is_assignable(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
  case CPPDeclaration::ST_reference:
  case CPPDeclaration::ST_extension:
    return false;

  case CPPDeclaration::ST_struct:
    // In many cases, this is assignable, but there are some bizarre cases
    // where it is not.  Particularly in the event that the programmer has
    // defined a private copy assignment operator for the class or struct.

    // We could try to figure out whether this has happened, but screw it.
    // Concrete structure objects are not assignable, and so they don't get
    // setters synthesized for them.  If you want a setter, write it yourself.

    // We'll make an exception for the string types, however, since these are
    // nearly an atomic type.
    if (is_basic_string_char(type) || is_basic_string_wchar(type)) {
      return true;
    }

    return false;

  case CPPDeclaration::ST_typedef:
    return is_assignable(type->as_typedef_type()->_type);

  default:
    return true;
  }
}

/**
 * Returns true if the indicated type is some kind of a reference or const
 * reference type to something useful, false otherwise.
 */
bool TypeManager::
is_reference(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_reference(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return is_pointable(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_reference(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is some kind of an rvalue reference.
 */
bool TypeManager::
is_rvalue_reference(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_rvalue_reference(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return type->as_reference_type()->_value_category == CPPReferenceType::VC_rvalue;

  case CPPDeclaration::ST_typedef:
    return is_rvalue_reference(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is some kind of a reference or const
 * reference type at all, false otherwise.
 */
bool TypeManager::
is_ref_to_anything(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_ref_to_anything(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return true;

  case CPPDeclaration::ST_typedef:
    return is_ref_to_anything(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const reference to something, false
 * otherwise.
 */
bool TypeManager::
is_const_ref_to_anything(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_const_ref_to_anything(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return is_const(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_ref_to_anything(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const pointer to something, false
 * otherwise.
 */
bool TypeManager::
is_const_pointer_to_anything(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_const_pointer_to_anything(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_const(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_pointer_to_anything(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a non-const pointer or reference to
 * something, false otherwise.
 */
bool TypeManager::
is_const_pointer_or_ref(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_const_pointer_or_ref(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_const(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_reference:
    return is_const(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_pointer_or_ref(type->as_typedef_type()->_type);

  case CPPDeclaration::ST_struct:
    if (type->get_simple_name() == "PointerTo") {
      return false;
    } else if (type->get_simple_name() == "ConstPointerTo") {
      return true;
    }

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a non-const pointer or reference to
 * something, false otherwise.
 */
bool TypeManager::
is_non_const_pointer_or_ref(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_non_const_pointer_or_ref(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return !is_const(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_reference:
    return !is_const(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_non_const_pointer_or_ref(type->as_typedef_type()->_type);

  case CPPDeclaration::ST_struct:
    if (type->get_simple_name() == "PointerTo") {
      return true;
    } else if (type->get_simple_name() == "ConstPointerTo") {
      return false;
    }

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is some kind of a pointer or const
 * pointer type, false otherwise.
 */
bool TypeManager::
is_pointer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_pointable(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_pointer(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is some kind of a const type, false
 * otherwise.
 */
bool TypeManager::
is_const(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return true;

  case CPPDeclaration::ST_typedef:
    return is_const(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a concrete struct, class, or union
 * type, or false otherwise.
 */
bool TypeManager::
is_struct(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_struct(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_struct:
  case CPPDeclaration::ST_extension:
    return true;

  case CPPDeclaration::ST_typedef:
    return is_struct(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is an enum class, const or otherwise.
 */
bool TypeManager::
is_scoped_enum(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_enum:
    return ((CPPEnumType *)type)->is_scoped();

  case CPPDeclaration::ST_const:
    return is_scoped_enum(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_scoped_enum(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is some kind of enumerated type, const
 * or otherwise.
 */
bool TypeManager::
is_enum(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_enum:
    return true;

  case CPPDeclaration::ST_const:
    return is_enum(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_enum(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const enumerated type.
 */
bool TypeManager::
is_const_enum(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_enum(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_enum(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const reference to an enumerated
 * type.
 */
bool TypeManager::
is_const_ref_to_enum(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_reference:
    return is_const_enum(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_ref_to_enum(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is nullptr_t, possibly const or a
 * typedef to it.
 */
bool TypeManager::
is_nullptr(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_simple:
    return type->as_simple_type()->_type == CPPSimpleType::T_nullptr;

  case CPPDeclaration::ST_const:
    return is_nullptr(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_nullptr(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is something that a scripting language
 * can handle directly as a concrete, like an int or float, either const or
 * non-const.
 */
bool TypeManager::
is_simple(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_simple:
  case CPPDeclaration::ST_enum:
    return !is_void(type);

  case CPPDeclaration::ST_const:
    return is_simple(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_simple(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const wrapper around some simple
 * type like int.
 */
bool TypeManager::
is_const_simple(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_simple(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_const_simple(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const reference to something that a
 * scripting language can handle directly as a concrete.
 */
bool TypeManager::
is_const_ref_to_simple(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_reference:
    return is_const_simple(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_ref_to_simple(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a non-const reference to something
 * that a scripting language can handle directly as a concrete.
 */
bool TypeManager::
is_ref_to_simple(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_reference:
    return is_simple(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_ref_to_simple(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is an array of a simple type.
 */
bool TypeManager::
is_simple_array(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_simple_array(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_array:
    return is_simple(type->as_array_type()->_element_type);

  case CPPDeclaration::ST_typedef:
    return is_simple_array(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const or a non-constant pointer to
 * a simple type.  This could also be a reference to an array of the simple
 * type.
 */
bool TypeManager::
is_pointer_to_simple(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer_to_simple(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_simple(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_array:
    return is_simple(type->as_array_type()->_element_type);

  case CPPDeclaration::ST_reference:
    return is_pointer_to_simple(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_pointer_to_simple(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is something ordinary that a scripting
 * language can handle a pointer to, e.g.  a class or a structure, but not an
 * int or a function.
 */
bool TypeManager::
is_pointable(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointable(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_extension:
    return (type->as_extension_type()->_type != CPPExtensionType::T_enum);

  case CPPDeclaration::ST_struct:
    return true;

  // case CPPDeclaration::ST_simple: return is_char(type);

  case CPPDeclaration::ST_typedef:
    return is_pointable(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is char or const char, but not signed or
 * unsigned char.
 */
bool TypeManager::
is_char(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_char(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return
          simple_type->_type == CPPSimpleType::T_char &&
          simple_type->_flags == 0;
      }
    }

  case CPPDeclaration::ST_typedef:
    return is_char(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is unsigned char, but not signed or
 * 'plain' char.
 */
bool TypeManager::
is_unsigned_char(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_unsigned_char(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();

      if (simple_type != nullptr) {
        return
          (simple_type->_type == CPPSimpleType::T_char) &&
          (simple_type->_flags & CPPSimpleType::F_unsigned) != 0;
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_unsigned_char(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is signed char, but not unsigned or
 * 'plain' char.
 */
bool TypeManager::
is_signed_char(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_signed_char(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();

      if (simple_type != nullptr) {
        return
          (simple_type->_type == CPPSimpleType::T_char) &&
          (simple_type->_flags & CPPSimpleType::F_signed) != 0;
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_signed_char(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is char * or const char * or some such.
 */
bool TypeManager::
is_char_pointer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_char_pointer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_char(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_char_pointer(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is const char*.
 */
bool TypeManager::
is_const_char_pointer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_const_char_pointer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return (is_const(type->as_pointer_type()->_pointing_at) &&
            is_char(type->as_pointer_type()->_pointing_at));

  case CPPDeclaration::ST_typedef:
    return is_const_char_pointer(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is unsigned char* or const unsigned
 * char*.
 */
bool TypeManager::
is_unsigned_char_pointer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_unsigned_char_pointer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_unsigned_char(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_unsigned_char_pointer(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is const unsigned char*.
 */
bool TypeManager::
is_const_unsigned_char_pointer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_unsigned_char_pointer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_const_unsigned_char_pointer(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the type is basic_string<char>.  This is the standard C++
 * string class.
 */
bool TypeManager::
is_basic_string_char(CPPType *type) {
  CPPType *string_type = get_basic_string_char_type();
  if (string_type != nullptr &&
      string_type->get_local_name(&parser) == type->get_local_name(&parser)) {
    return true;
  }

  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_basic_string_char(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_basic_string_char(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is a const wrapper around
 * basic_string<char>.
 */
bool TypeManager::
is_const_basic_string_char(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_basic_string_char(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_const_basic_string_char(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const reference to
 * basic_string<char>.
 */
bool TypeManager::
is_const_ref_to_basic_string_char(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_reference:
    return is_const_basic_string_char(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_ref_to_basic_string_char(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const pointer to
 * basic_string<char>.
 */
bool TypeManager::
is_const_ptr_to_basic_string_char(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_pointer:
    return is_const_basic_string_char(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_ptr_to_basic_string_char(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the type is basic_string<char>, or a const reference to it.
 */
bool TypeManager::
is_string(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_reference:
    return is_const_basic_string_char(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_string(type->as_typedef_type()->_type);

  default:
    break;
  }

  return is_basic_string_char(type);
}

/**
 * Returns true if the indicated type is wchar_t or const wchar_t.  We don't
 * mind signed or unsigned wchar_t.
 */
bool TypeManager::
is_wchar(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_wchar(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return simple_type->_type == CPPSimpleType::T_wchar_t;
      }
    }

  case CPPDeclaration::ST_typedef:
    return is_wchar(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is wchar_t * or const wchar_t * or some
 * such.
 */
bool TypeManager::
is_wchar_pointer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_wchar_pointer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_wchar(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_wchar_pointer(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the type is basic_string<wchar_t>.  This is the standard
 * C++ wide string class.
 */
bool TypeManager::
is_basic_string_wchar(CPPType *type) {
  CPPType *string_type = get_basic_string_wchar_type();
  if (string_type != nullptr &&
      string_type->get_local_name(&parser) == type->get_local_name(&parser)) {
    return true;
  }

  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_basic_string_wchar(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_basic_string_wchar(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is a const wrapper around
 * basic_string<wchar_t>.
 */
bool TypeManager::
is_const_basic_string_wchar(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_basic_string_wchar(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_const_basic_string_wchar(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const reference to
 * basic_string<wchar_t>.
 */
bool TypeManager::
is_const_ref_to_basic_string_wchar(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_reference:
    return is_const_basic_string_wchar(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_ref_to_basic_string_wchar(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const pointer to
 * basic_string<wchar_t>.
 */
bool TypeManager::
is_const_ptr_to_basic_string_wchar(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_pointer:
    return is_const_basic_string_wchar(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_ptr_to_basic_string_wchar(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the type is basic_string<wchar_t>, or a const reference to
 * it.
 */
bool TypeManager::
is_wstring(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_reference:
    return is_const_basic_string_wchar(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_wstring(type->as_typedef_type()->_type);

  default:
    break;
  }

  return is_basic_string_wchar(type);
}

/**
 * Returns true if the type is vector<unsigned char>, or a const reference to
 * it.
 */
bool TypeManager::
is_vector_unsigned_char(CPPType *type) {
  if (type->get_local_name(&parser) == "vector< unsigned char >" ||
      type->get_local_name(&parser) == "std::vector< unsigned char >" ||
      type->get_local_name(&parser) == "pvector< unsigned char >") {
    return true;
  }

  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_vector_unsigned_char(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return is_const_vector_unsigned_char(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_struct:
    {
      CPPStructType *stype = type->as_struct_type();
      CPPStructType::Derivation::const_iterator di;
      for (di = stype->_derivation.begin();
           di != stype->_derivation.end();
           ++di) {
        if (is_vector_unsigned_char((*di)._base)) {
          return true;
        }
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_vector_unsigned_char(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is a const wrapper around
 * vector<unsigned char>.
 */
bool TypeManager::
is_const_vector_unsigned_char(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_vector_unsigned_char(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_const_vector_unsigned_char(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is bool, or some trivial variant.
 */
bool TypeManager::
is_bool(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_bool(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return
          simple_type->_type == CPPSimpleType::T_bool;
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_bool(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is one of the basic integer types: bool,
 * char, short, int, or long, signed or unsigned, as well as enumerated types.
 */
bool TypeManager::
is_integer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_integer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_enum:
    return true;

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return
          (simple_type->_type == CPPSimpleType::T_bool ||
           simple_type->_type == CPPSimpleType::T_char ||
           simple_type->_type == CPPSimpleType::T_wchar_t ||
           simple_type->_type == CPPSimpleType::T_char8_t ||
           simple_type->_type == CPPSimpleType::T_char16_t ||
           simple_type->_type == CPPSimpleType::T_char32_t ||
           simple_type->_type == CPPSimpleType::T_int);
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_integer(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is one of the basic integer types, but
 * only the unsigned varieties.
 */
bool TypeManager::
is_unsigned_integer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_unsigned_integer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return
          ((simple_type->_type == CPPSimpleType::T_bool ||
            simple_type->_type == CPPSimpleType::T_char ||
            simple_type->_type == CPPSimpleType::T_wchar_t ||
            simple_type->_type == CPPSimpleType::T_int) &&
           (simple_type->_flags & CPPSimpleType::F_unsigned) != 0) ||
           (simple_type->_type == CPPSimpleType::T_char8_t ||
            simple_type->_type == CPPSimpleType::T_char16_t ||
            simple_type->_type == CPPSimpleType::T_char32_t);
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_unsigned_integer(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is the "size_t" type, or a const size_t,
 * or a typedef to either.
 */
bool TypeManager::
is_size(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_size(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    if (type->get_simple_name() == "size_t") {
      return is_integer(type->as_typedef_type()->_type);
    } else {
      return is_size(type->as_typedef_type()->_type);
    }

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is the "ssize_t" type, or a const
 * ssize_t, or a typedef to either.  ptrdiff_t and streamsize are also
 * accepted, since they are usually also defined as the signed counterpart to
 * size_t.
 */
bool TypeManager::
is_ssize(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_ssize(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    if (type->get_simple_name() == "Py_ssize_t" ||
        type->get_simple_name() == "ssize_t" ||
        type->get_simple_name() == "ptrdiff_t" ||
        type->get_simple_name() == "streamsize") {
      return is_integer(type->as_typedef_type()->_type);
    } else {
      return is_ssize(type->as_typedef_type()->_type);
    }

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is the "long" type, whether signed or
 * unsigned.
 */
bool TypeManager::
is_long(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_long(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return (simple_type->_type == CPPSimpleType::T_int &&
                (simple_type->_flags & CPPSimpleType::F_long) != 0);
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_long(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is the "short" type, whether signed or
 * unsigned.
 */
bool TypeManager::
is_short(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_short(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return (simple_type->_type == CPPSimpleType::T_int &&
                (simple_type->_flags & CPPSimpleType::F_short) != 0);
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_short(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is an unsigned "short" type.
 */
bool TypeManager::
is_unsigned_short(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_unsigned_short(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return (simple_type->_type == CPPSimpleType::T_int &&
                (simple_type->_flags & (CPPSimpleType::F_short | CPPSimpleType::F_unsigned)) == (CPPSimpleType::F_short | CPPSimpleType::F_unsigned));
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_unsigned_short(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is the "long long" type or larger, or at
 * least a 64-bit integer, whether signed or unsigned.
 */
bool TypeManager::
is_longlong(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_longlong(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return (simple_type->_type == CPPSimpleType::T_int &&
                (simple_type->_flags & CPPSimpleType::F_longlong) != 0);
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_longlong(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is an unsigned "long long" type or
 * larger, or at least a 64-bit unsigned integer.
 */
bool TypeManager::
is_unsigned_longlong(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_unsigned_longlong(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return (simple_type->_type == CPPSimpleType::T_int &&
                (simple_type->_flags & (CPPSimpleType::F_longlong | CPPSimpleType::F_unsigned)) == (CPPSimpleType::F_longlong | CPPSimpleType::F_unsigned));
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_unsigned_longlong(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is the "double" type.
 */
bool TypeManager::
is_double(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_double(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return (simple_type->_type == CPPSimpleType::T_double);
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_double(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is one of the basic floating-point
 * types: float, double, or some similar variant.
 */
bool TypeManager::
is_float(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_float(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_simple:
    {
      CPPSimpleType *simple_type = type->as_simple_type();
      if (simple_type != nullptr) {
        return
          (simple_type->_type == CPPSimpleType::T_float ||
           simple_type->_type == CPPSimpleType::T_double);
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_float(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is void.  (Not void *, just void.)
 */
bool TypeManager::
is_void(CPPType *type) {
  CPPSimpleType *simple_type = type->as_simple_type();
  if (simple_type != nullptr) {
    return
      simple_type->_type == CPPSimpleType::T_void &&
      simple_type->_flags == 0;
  }

  return false;
}

/**
 * Returns true if the indicated type is some class that derives from
 * ReferenceCount, or defines ref and unref(), or false otherwise.
 */
bool TypeManager::
is_reference_count(CPPType *type) {
  CPPType *refcount_type = get_reference_count_type();
  if (refcount_type != nullptr &&
      refcount_type->get_local_name(&parser) == type->get_local_name(&parser)) {
    return true;
  }

  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_reference_count(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_struct:
    {
      CPPStructType *stype = type->as_struct_type();

      // If we have methods named ref() and unref(), this is good enough.
      if (stype->_scope->_functions.count("ref") &&
          stype->_scope->_functions.count("unref") &&
          stype->_scope->_functions.count("get_ref_count")) {
        return true;
      }

      CPPStructType::Derivation::const_iterator di;
      for (di = stype->_derivation.begin();
           di != stype->_derivation.end();
           ++di) {
        if (is_reference_count((*di)._base)) {
          return true;
        }
      }
    }
    break;

  case CPPDeclaration::ST_typedef:
    return is_reference_count(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is a pointer to a class that derives
 * from ReferenceCount.
 */
bool TypeManager::
is_reference_count_pointer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_reference_count_pointer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_reference_count(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_reference_count_pointer(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is some class that derives from
 * PointerToBase, or false otherwise.
 */
bool TypeManager::
is_pointer_to_base(CPPType *type) {
  // We only check the simple name of the type against PointerToBase, since we
  // need to allow for the various template instantiations of this thing.

  // We also check explicitly for "PointerTo" and "ConstPointerTo", instead of
  // actually checking for PointerToBase, because we don't want to consider
  // PointerToArray in this category.
  if (type->get_simple_name() == "PointerTo" ||
      type->get_simple_name() == "ConstPointerTo") {
    return true;
  }

  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer_to_base(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_struct:
    {
      CPPStructType *stype = type->as_struct_type();
      CPPStructType::Derivation::const_iterator di;
      for (di = stype->_derivation.begin();
           di != stype->_derivation.end();
           ++di) {
        if (is_pointer_to_base((*di)._base)) {
          return true;
        }
      }
    }
    return false;

  case CPPDeclaration::ST_typedef:
    return is_pointer_to_base(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const PointerToBase or some
 * derivative.
 */
bool TypeManager::
is_const_pointer_to_base(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer_to_base(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return is_const_pointer_to_base(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is a const reference to a class that
 * derives from PointerToBase.
 */
bool TypeManager::
is_const_ref_to_pointer_to_base(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_reference(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return is_const_pointer_to_base(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_const_ref_to_pointer_to_base(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the type is pair<>, or a reference to it.
 */
bool TypeManager::
is_pair(CPPType *type) {
  // We only check the simple name of the type against pair, since we need to
  // allow for the various template instantiations of this thing.
  if (type->get_simple_name() == "pair") {
    return true;
  }

  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pair(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return is_pair(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_pair(type->as_typedef_type()->_type);

  default:
    break;
  }

  return false;
}

/**
 * Returns true if the indicated type is PyObject *.
 */
bool TypeManager::
is_pointer_to_PyObject(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer_to_PyObject(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_PyObject(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_pointer_to_PyObject(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is PyObject.
 */
bool TypeManager::
is_PyObject(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_PyObject(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_extension:
  case CPPDeclaration::ST_struct:
    return (type->get_local_name(&parser) == "_object" ||
            type->get_local_name(&parser) == "_typeobject");

  case CPPDeclaration::ST_typedef:
    return (is_struct(type->as_typedef_type()->_type) &&
            (type->get_local_name(&parser) == "PyObject" ||
             type->get_local_name(&parser) == "PyTypeObject" ||
             type->get_local_name(&parser) == "PyStringObject" ||
             type->get_local_name(&parser) == "PyUnicodeObject")) ||
           is_PyObject(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is PyTypeObject *.
 */
bool TypeManager::
is_pointer_to_PyTypeObject(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer_to_PyTypeObject(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_PyTypeObject(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_pointer_to_PyTypeObject(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is PyTypeObject.
 */
bool TypeManager::
is_PyTypeObject(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_PyTypeObject(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_extension:
  case CPPDeclaration::ST_struct:
    return (type->get_local_name(&parser) == "_typeobject");

  case CPPDeclaration::ST_typedef:
    return (type->get_local_name(&parser) == "PyTypeObject" &&
            is_struct(type->as_typedef_type()->_type)) ||
           is_PyTypeObject(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is PyStringObject *.
 */
bool TypeManager::
is_pointer_to_PyStringObject(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer_to_PyStringObject(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_PyStringObject(type->as_pointer_type()->_pointing_at);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is PyStringObject.
 */
bool TypeManager::
is_PyStringObject(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_PyStringObject(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return (type->get_local_name(&parser) == "PyStringObject" &&
            is_struct(type->as_typedef_type()->_type)) ||
           is_PyStringObject(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is PyStringObject *.
 */
bool TypeManager::
is_pointer_to_PyUnicodeObject(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer_to_PyUnicodeObject(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_PyUnicodeObject(type->as_pointer_type()->_pointing_at);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is PyUnicodeObject.
 */
bool TypeManager::
is_PyUnicodeObject(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_PyUnicodeObject(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_typedef:
    return (type->get_local_name(&parser) == "PyUnicodeObject" &&
            is_struct(type->as_typedef_type()->_type)) ||
           is_PyUnicodeObject(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is Py_buffer *.
 */
bool TypeManager::
is_pointer_to_Py_buffer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer_to_Py_buffer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return is_Py_buffer(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_pointer_to_Py_buffer(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is Py_buffer.
 */
bool TypeManager::
is_Py_buffer(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_Py_buffer(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_extension:
  case CPPDeclaration::ST_struct:
    return (type->get_local_name(&parser) == "Py_buffer" ||
            type->get_local_name(&parser) == "bufferinfo");

  case CPPDeclaration::ST_typedef:
    return is_Py_buffer(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is TypeHandle or a class with identical
 * semantics like ButtonHandle.
 */
bool TypeManager::
is_handle(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_handle(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_extension:
  case CPPDeclaration::ST_struct:
    return (type->get_local_name(&parser) == "TypeHandle" ||
            type->get_local_name(&parser) == "ButtonHandle");

  case CPPDeclaration::ST_typedef:
    return is_handle(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is PyObject.
 */
bool TypeManager::is_ostream(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_ostream(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_struct:
    return (type->get_local_name(&parser) == "std::ostream" ||
            type->get_local_name(&parser) == "ostream" ||
            type->get_local_name(&parser) == "std::basic_ostream< char >");

  case CPPDeclaration::ST_typedef:
    return is_ostream(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the indicated type is PyObject *.
 */
bool TypeManager::
is_pointer_to_ostream(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_pointer_to_ostream(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return is_ostream(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_pointer:
    return is_ostream(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return is_pointer_to_ostream(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns true if the type is an unpublished type, e.g.  a protected or
 * private nested class, or simply a type not marked as 'published', or if the
 * type is a pointer or reference to such an unpublished type, or even if the
 * type is a function type that includes a parameter of such an unpublished
 * type.
 */
bool TypeManager::
involves_unpublished(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return involves_unpublished(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return involves_unpublished(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_pointer:
    return involves_unpublished(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_struct:
    // A struct type is unpublished only if all of its members are
    // unpublished.
    if (type->_declaration != nullptr) {
      if (type->_declaration->_vis <= min_vis) {
        return false;
      }
    }
    {
      CPPScope *scope = type->as_struct_type()->_scope;

      bool any_exported = false;
      CPPScope::Declarations::const_iterator di;
      for (di = scope->_declarations.begin();
           di != scope->_declarations.end() && !any_exported;
           ++di) {
        if ((*di)->_vis <= min_vis) {
          any_exported = true;
        }
      }

      return !any_exported;
    }

  case CPPDeclaration::ST_function:
    if (type->_declaration != nullptr) {
      if (type->_declaration->_vis <= min_vis) {
        return false;
      }
    }
    return true;
    /*
    {
      CPPFunctionType *ftype = type->as_function_type();
      if (involves_unpublished(ftype->_return_type)) {
        return true;
      }
      const CPPParameterList::Parameters &params =
        ftype->_parameters->_parameters;
      CPPParameterList::Parameters::const_iterator pi;
      for (pi = params.begin(); pi != params.end(); ++pi) {
        if (involves_unpublished((*pi)->_type)) {
          return true;
        }
      }
      return false;
    }
    */

  case CPPDeclaration::ST_typedef:
    return involves_unpublished(type->as_typedef_type()->_type);

  default:
    if (type->_declaration != nullptr) {
      return (type->_declaration->_vis > min_vis);
    }
    return false;
  }
}

/**
 * Returns true if the type is an protected type, e.g.  a protected or private
 * nested class, or if the type is a pointer or reference to such a protected
 * type, or even if the type is a function type that includes a parameter of
 * such a protected type.
 */
bool TypeManager::
involves_protected(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return involves_protected(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return involves_protected(type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_pointer:
    return involves_protected(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_function:
    {
      CPPFunctionType *ftype = type->as_function_type();
      if (involves_protected(ftype->_return_type)) {
        return true;
      }
      const CPPParameterList::Parameters &params =
        ftype->_parameters->_parameters;
      CPPParameterList::Parameters::const_iterator pi;
      for (pi = params.begin(); pi != params.end(); ++pi) {
        if (involves_protected((*pi)->_type)) {
          return true;
        }
      }
      return false;
    }

  case CPPDeclaration::ST_typedef:
    return involves_protected(type->as_typedef_type()->_type);

  default:
    if (type->_declaration != nullptr) {
      return (type->_declaration->_vis > V_public);
    }
    return false;
  }
}

/**
 * Returns true if the type involves an rvalue reference.
 */
bool TypeManager::
involves_rvalue_reference(CPPType *type) {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return involves_rvalue_reference(type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return type->as_reference_type()->_value_category == CPPReferenceType::VC_rvalue;

  case CPPDeclaration::ST_pointer:
    return involves_rvalue_reference(type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_function:
    {
      CPPFunctionType *ftype = type->as_function_type();
      if (involves_rvalue_reference(ftype->_return_type)) {
        return true;
      }
      const CPPParameterList::Parameters &params =
        ftype->_parameters->_parameters;
      CPPParameterList::Parameters::const_iterator pi;
      for (pi = params.begin(); pi != params.end(); ++pi) {
        if (involves_rvalue_reference((*pi)->_type)) {
          return true;
        }
      }
      return false;
    }

  case CPPDeclaration::ST_typedef:
    return involves_rvalue_reference(type->as_typedef_type()->_type);

  default:
    return false;
  }
}

/**
 * Returns the type this pointer type points to.
 */
CPPType *TypeManager::
unwrap_pointer(CPPType *source_type) {
  switch (source_type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return unwrap_pointer(source_type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_pointer:
    return source_type->as_pointer_type()->_pointing_at;

  default:
    return source_type;
  }
}

/**
 * Returns the type this reference type points to.
 */
CPPType *TypeManager::
unwrap_reference(CPPType *source_type) {
  switch (source_type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return unwrap_reference(source_type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return source_type->as_reference_type()->_pointing_at;

  default:
    return source_type;
  }
}

/**
 * Removes the const declaration from the outside of the type.
 */
CPPType *TypeManager::
unwrap_const(CPPType *source_type) {
  switch (source_type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return unwrap_const(source_type->as_const_type()->_wrapped_around);

  default:
    return source_type;
  }
}

/**
 * Removes a reference or a const reference from the type.
 */
CPPType *TypeManager::
unwrap_const_reference(CPPType *source_type) {
  switch (source_type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return unwrap_const_reference(source_type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return unwrap_const(source_type->as_reference_type()->_pointing_at);

  default:
    return source_type;
  }
}

/**
 * Removes all const, pointer, reference wrappers, and typedefs, to get to the
 * thing we're talking about.
 */
CPPType *TypeManager::
unwrap(CPPType *source_type) {
  switch (source_type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return unwrap(source_type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return unwrap(source_type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_pointer:
    return unwrap(source_type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_typedef:
    return unwrap(source_type->as_typedef_type()->_type);

  default:
    return source_type;
  }
}

/**
 * Returns the type of pointer the given PointerTo class emulates.
 * Essentially this just checks the return type of the method called 'p()'.
 * Returns NULL if the PointerTo class has no method p().
 */
CPPType *TypeManager::
get_pointer_type(CPPStructType *pt_type) {
  CPPScope *scope = pt_type->_scope;

  CPPScope::Functions::const_iterator fi;
  fi = scope->_functions.find("p");
  if (fi != scope->_functions.end()) {
    CPPFunctionGroup *fgroup = (*fi).second;

    // These are all the functions named "p".  Now look for one that takes no
    // parameters.
    CPPFunctionGroup::Instances::iterator ii;
    for (ii = fgroup->_instances.begin();
         ii != fgroup->_instances.end();
         ++ii) {
      CPPInstance *function = (*ii);
      CPPFunctionType *ftype = function->_type->as_function_type();
      assert(ftype != nullptr);
      if (ftype->_parameters->_parameters.empty()) {
        // Here's the function p().  What's its return type?
        return resolve_type(ftype->_return_type);
      }
    }
  }

  return nullptr;
}

/**
 * Returns the ith template parameter type.  For instance, if the type is
 * pair<A, B>, then this function will return type A when passing 0 and type B
 * when passing 1, and NULL otherwise.
 */
CPPType *TypeManager::
get_template_parameter_type(CPPType *source_type, int i) {
  switch (source_type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return get_template_parameter_type(source_type->as_const_type()->_wrapped_around, i);

  case CPPDeclaration::ST_reference:
    return get_template_parameter_type(source_type->as_reference_type()->_pointing_at, i);

  case CPPDeclaration::ST_typedef:
    return get_template_parameter_type(source_type->as_typedef_type()->_type);

  default:
    break;
  }

  CPPStructType *type = source_type->as_struct_type();
  if (type == nullptr) {
    return nullptr;
  }

  // I'm not sure how reliable this is, but I don't know if there is a more
  // proper way to access this.
  CPPTemplateParameterList *templ = type->_ident->_names.back().get_templ();
  if (templ == nullptr || i >= (int)templ->_parameters.size()) {
    return nullptr;
  }

  CPPDeclaration *decl = templ->_parameters[i];
  return decl->as_type();
}

/**
 * Returns the type corresponding to a pointer to the given type.
 */
CPPType *TypeManager::
wrap_pointer(CPPType *source_type) {
  return CPPType::new_type(new CPPPointerType(source_type));
}

/**
 * Returns the type corresponding to a const pointer to the given type.
 */
CPPType *TypeManager::
wrap_const_pointer(CPPType *source_type) {
  if (source_type->as_const_type() != nullptr) {
    // It's already const.
    return
      CPPType::new_type(new CPPPointerType(source_type));
  } else {
    return
      CPPType::new_type(new CPPPointerType(new CPPConstType(source_type)));
  }
}

/**
 * Returns the type corresponding to a const reference to the given type.
 */
CPPType *TypeManager::
wrap_const_reference(CPPType *source_type) {
  if (source_type->as_const_type() != nullptr) {
    // It's already const.
    return
      CPPType::new_type(new CPPReferenceType(source_type));
  } else {
    return
      CPPType::new_type(new CPPReferenceType(new CPPConstType(source_type)));
  }
}

/**
 * Returns a CPPType that represents basic_string<char>, or NULL if the type
 * is unknown.
 */
CPPType *TypeManager::
get_basic_string_char_type() {
  static bool got_type = false;
  static CPPType *type = nullptr;
  if (!got_type) {
    type = parser.parse_type("std::basic_string<char>");
    got_type = true;
  }
  return type;
}

/**
 * Returns a CPPType that represents basic_string<wchar_t>, or NULL if the
 * type is unknown.
 */
CPPType *TypeManager::
get_basic_string_wchar_type() {
  static bool got_type = false;
  static CPPType *type = nullptr;
  if (!got_type) {
    type = parser.parse_type("std::basic_string<wchar_t>");
    got_type = true;
  }
  return type;
}

/**
 * Returns a CPPType that represents ReferenceCount, or NULL if the type is
 * unknown.
 */
CPPType *TypeManager::
get_reference_count_type() {
  static bool got_type = false;
  static CPPType *type = nullptr;
  if (!got_type) {
    type = parser.parse_type("ReferenceCount");
    got_type = true;
  }
  return type;
}

/**
 * Returns a CPPType that represents void.
 */
CPPType *TypeManager::
get_void_type() {
  static bool got_type = false;
  static CPPType *type = nullptr;
  if (!got_type) {
    type = CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_void));
    got_type = true;
  }
  return type;
}

/**
 * Returns a CPPType that represents int.
 */
CPPType *TypeManager::
get_int_type() {
  static bool got_type = false;
  static CPPType *type = nullptr;
  if (!got_type) {
    type = CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_int));
    got_type = true;
  }
  return type;
}

/**
 * Returns a string corresponding to the given function signature.  This is a
 * unique string per each uniquely-callable C++ function or method.  Basically
 * it's the function prototype, sans the return type.
 *
 * If num_default_parameters is nonzero, it is the number of parameters to
 * omit from the end of the parameter list.  This in effect gets the function
 * signature for an equivalent function with n parameters assuming default
 * values.
 */
string TypeManager::
get_function_signature(CPPInstance *function,
                       int num_default_parameters) {
  CPPFunctionType *ftype = function->_type->as_function_type();
  assert(ftype != nullptr);

  std::ostringstream out;

  // It's tempting to mark static methods with a different function signature
  // than non-static, because a static method doesn't have an implicit 'this'
  // parameter.  However, this breaks the lookup when we come across a method
  // definition outside of the class body; since there's no clue at this point
  // whether the method is static or not, we can't successfully look it up.
  // Bummer.
  /*
    if ((function->_storage_class & CPPInstance::SC_static) != 0) {
    out << "static ";
    }
  */

  out << function->get_local_name(&parser) << "(";

  const CPPParameterList::Parameters &params =
    ftype->_parameters->_parameters;
  CPPParameterList::Parameters::const_iterator pi;

  int num_params = params.size() - num_default_parameters;
  pi = params.begin();
  for (int n = 0; n < num_params; n++) {
    assert(pi != params.end());
    CPPType *ptype = (*pi)->_type;

    // One exception: if the type is a const reference to something, we build
    // the signature with its corresponding concrete.  C++ can't differentiate
    // these two anyway.
    if (is_const_ref_to_anything(ptype)) {
      ptype = unwrap_const_reference(ptype);
    }

    out << ptype->get_local_name(&parser);

    if (n + 1 < num_params) {
      out << ", ";
    }

    ++pi;
  }
  out << ")";

  if (ftype->_flags & CPPFunctionType::F_const_method) {
    out << " const";
  }

  return out.str();
}

/**
 * Returns a string corresponding to the given function name.  This is not
 * necessarily unique to the particular overloaded function instance, but is
 * common among all overloaded functions of the same name.
 */
string TypeManager::
get_function_name(CPPInstance *function) {
  return function->get_local_name(&parser);
}

/**
 * Returns true if the destructor for the given class or struct is protected
 * or private, or false if the destructor is public or absent.
 */
bool TypeManager::
has_protected_destructor(CPPType *type) {
  CPPStructType *struct_type = type->as_struct_type();
  if (struct_type == nullptr) {
    // It's not even a struct type!
    return false;
  }

  CPPScope *scope = struct_type->get_scope();

  // Look for the destructor.
  CPPScope::Declarations::const_iterator di;
  for (di = scope->_declarations.begin();
       di != scope->_declarations.end();
       ++di) {
    if ((*di)->get_subtype() == CPPDeclaration::ST_instance) {
      CPPInstance *inst = (*di)->as_instance();
      if (inst->_type->get_subtype() == CPPDeclaration::ST_function) {
        // Here's a function declaration.
        CPPFunctionType *ftype = inst->_type->as_function_type();
        assert(ftype != nullptr);
        if ((ftype->_flags & CPPFunctionType::F_destructor) != 0) {
          // Here's the destructor!  Is it protected?
          return (inst->_vis > V_public);
        }
      }
    }
  }

  // No explicit destructor.
  return false;
}
/**
 *
 */
bool TypeManager::
is_exported(CPPType *in_type) {
  string name = in_type->get_local_name(&parser);
  if (name.empty()) {
    return false;
  }

  // this question is about the base type
  CPPType *base_type = resolve_type(unwrap(in_type));
  // CPPType *base_type = in_type; Ok export Rules.. Classes and Structs and
  // Unions are exported only if they have a function that is exported..
  // function is the easiest case.

  if (base_type->_vis <= min_vis) {
    return true;
  }

  if (in_type->_vis <= min_vis) {
    return true;
  }

  if (base_type->get_subtype() == CPPDeclaration::ST_struct) {
    CPPStructType *struct_type = base_type->resolve_type(&parser, &parser)->as_struct_type();
    CPPScope *scope = struct_type->_scope;

    CPPScope::Declarations::const_iterator di;
    for (di = scope->_declarations.begin();
         di != scope->_declarations.end(); ++di) {
      if ((*di)->_vis <= min_vis) {
        return true;
      }
    }

  } else if (base_type->get_subtype() == CPPDeclaration::ST_instance) {
    CPPInstance *inst = base_type->as_instance();
    if (inst->_type->get_subtype() == CPPDeclaration::ST_function) {
      CPPInstance *function = inst;
      CPPFunctionType *ftype = function->_type->resolve_type(&parser, &parser)->as_function_type();
      if (ftype->_vis <= min_vis) {
        return true;
      }
    } else {
      if (inst->_vis <= min_vis) {
        return true;
      }
    }

  } else if (base_type->get_subtype() == CPPDeclaration::ST_typedef) {
    CPPTypedefType *tdef = base_type->as_typedef_type();
    if (tdef->_type->get_subtype() == CPPDeclaration::ST_struct) {
      CPPStructType *struct_type =tdef->_type->resolve_type(&parser, &parser)->as_struct_type();
      return is_exported(struct_type);
    }

  } else if (base_type->get_subtype() == CPPDeclaration::ST_type_declaration) {
    CPPType *type = base_type->as_type_declaration()->_type;
    if (type->get_subtype() == CPPDeclaration::ST_struct) {
      CPPStructType *struct_type =type->as_type()->resolve_type(&parser, &parser)->as_struct_type();
      // CPPScope *scope = struct_type->_scope;
      return is_exported(struct_type);

    } else if (type->get_subtype() == CPPDeclaration::ST_enum) {
      // CPPEnumType *enum_type = type->as_type()->resolve_type(&parser,
      // &parser)->as_enum_type();
      if (type->_vis <= min_vis) {
        return true;
      }
    }
  }

/*
  printf("---------------------> Visibility Failed  %s %d Vis=%d, Minvis=%d\n",
         base_type->get_fully_scoped_name().c_str(),
         base_type->get_subtype(),
         base_type->_vis,
         min_vis);
*/
  return false;
}

/**
 * Returns true if the type is defined in a local file rather than one that is
 * included.
 */
bool TypeManager::
is_local(CPPType *source_type) {
  switch (source_type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return is_local(source_type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return is_local(source_type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_pointer:
    return is_local(source_type->as_pointer_type()->_pointing_at);

  case CPPDeclaration::ST_simple:
    return false;

  default:
    {
      CPPType *resolved_type = resolve_type(source_type);
      if (resolved_type->_file._source == CPPFile::S_local && !resolved_type->is_incomplete()) {
        return true;
      }
    }
  }

 return false;

    /*

    if (base_type->get_subtype() == CPPDeclaration::ST_struct)
    {
            CPPStructType *struct_type = base_type->as_struct_type();
            if (struct_type->_file._source == CPPFile::S_local)
                return  true;

    }
    else if (base_type->get_subtype() == CPPDeclaration::ST_instance)
    {
        CPPInstance *inst = base_type->as_instance();
        if (inst->_type->get_subtype() == CPPDeclaration::ST_function)
        {
            CPPInstance *function = inst;
            CPPFunctionType *ftype = function->_type->resolve_type(&parser, &parser)->as_function_type();
            if (ftype->_file._source == CPPFile::S_local)
                return true;
        }
        else
        {
            if (inst->_file._source == CPPFile::S_local)
                return true;
        }
    }
    else if (base_type->get_subtype() == CPPDeclaration::ST_typedef)
    {
        CPPTypedef *tdef = base_type->as_typedef();
        if (tdef->_type->get_subtype() == CPPDeclaration::ST_struct)
        {
            CPPStructType *struct_type =tdef->_type->resolve_type(&parser, &parser)->as_struct_type();
            return IsLocal(struct_type);


        }
    }
    else if (base_type->get_subtype() == CPPDeclaration::ST_type_declaration)
    {
        CPPType *type = base_type->as_type_declaration()->_type;
        if (type->get_subtype() == CPPDeclaration::ST_struct)
        {
            CPPStructType *struct_type =type->as_type()->resolve_type(&parser, &parser)->as_struct_type();
            if (struct_type->_file._source == CPPFile::S_local)
                return true;

        }
        else if (type->get_subtype() == CPPDeclaration::ST_enum)
        {
            CPPEnumType *enum_type = type->as_type()->resolve_type(&parser, &parser)->as_enum_type();
            if (enum_type->_file._source != CPPFile::S_local)
                return true;
        }
    }

  if (base_type->_file._source == CPPFile::S_local)
      return true;

 return false;
 */
}
