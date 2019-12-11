/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file register_type.I
 * @author drose
 * @date 2001-08-06
 */

/**
 * This inline function is just a convenient way to call
 * TypeRegistry::register_type(), along with zero to four
 * record_derivation()s.  If for some reason you have a class that has more
 * than four base classes (you're insane!), then you will need to call
 * Register() and record_derivation() yourself.
 */
INLINE void
register_type(TypeHandle &type_handle, const std::string &name) {
  TypeRegistry::ptr()->register_type(type_handle, name);
}
INLINE void
register_type(TypeHandle &type_handle, const std::string &name,
              TypeHandle parent1) {
  TypeRegistry *registry = TypeRegistry::ptr();
  if (registry->register_type(type_handle, name)) {
    registry->record_derivation(type_handle, parent1);
  }
}
INLINE void
register_type(TypeHandle &type_handle, const std::string &name,
              TypeHandle parent1, TypeHandle parent2) {
  TypeRegistry *registry = TypeRegistry::ptr();
  if (registry->register_type(type_handle, name)) {
    registry->record_derivation(type_handle, parent1);
    registry->record_derivation(type_handle, parent2);
  }
}
INLINE void
register_type(TypeHandle &type_handle, const std::string &name,
              TypeHandle parent1, TypeHandle parent2,
              TypeHandle parent3) {
  TypeRegistry *registry = TypeRegistry::ptr();
  if (registry->register_type(type_handle, name)) {
    registry->record_derivation(type_handle, parent1);
    registry->record_derivation(type_handle, parent2);
    registry->record_derivation(type_handle, parent3);
  }
}
INLINE void
register_type(TypeHandle &type_handle, const std::string &name,
              TypeHandle parent1, TypeHandle parent2,
              TypeHandle parent3, TypeHandle parent4) {
  TypeRegistry *registry = TypeRegistry::ptr();
  if (registry->register_type(type_handle, name)) {
    registry->record_derivation(type_handle, parent1);
    registry->record_derivation(type_handle, parent2);
    registry->record_derivation(type_handle, parent3);
    registry->record_derivation(type_handle, parent4);
  }
}

/**
 * This is essentially similar to register_type(), except that it doesn't
 * store a reference to any TypeHandle passed in and it therefore doesn't
 * complain if the type is registered more than once to different TypeHandle
 * reference.
 */
INLINE TypeHandle
register_dynamic_type(const std::string &name) {
  return TypeRegistry::ptr()->register_dynamic_type(name);
}
INLINE TypeHandle
register_dynamic_type(const std::string &name, TypeHandle parent1) {
  TypeRegistry *registry = TypeRegistry::ptr();
  TypeHandle type_handle = registry->register_dynamic_type(name);
  registry->record_derivation(type_handle, parent1);
  return type_handle;
}
INLINE TypeHandle
register_dynamic_type(const std::string &name,
                      TypeHandle parent1, TypeHandle parent2) {
  TypeRegistry *registry = TypeRegistry::ptr();
  TypeHandle type_handle = registry->register_dynamic_type(name);
  registry->record_derivation(type_handle, parent1);
  registry->record_derivation(type_handle, parent2);
  return type_handle;
}
INLINE TypeHandle
register_dynamic_type(const std::string &name,
                      TypeHandle parent1, TypeHandle parent2,
                      TypeHandle parent3) {
  TypeRegistry *registry = TypeRegistry::ptr();
  TypeHandle type_handle = registry->register_dynamic_type(name);
  registry->record_derivation(type_handle, parent1);
  registry->record_derivation(type_handle, parent2);
  registry->record_derivation(type_handle, parent3);
  return type_handle;
}
INLINE TypeHandle
register_dynamic_type(const std::string &name,
                      TypeHandle parent1, TypeHandle parent2,
                      TypeHandle parent3, TypeHandle parent4) {
  TypeRegistry *registry = TypeRegistry::ptr();
  TypeHandle type_handle = registry->register_dynamic_type(name);
  registry->record_derivation(type_handle, parent1);
  registry->record_derivation(type_handle, parent2);
  registry->record_derivation(type_handle, parent3);
  registry->record_derivation(type_handle, parent4);
  return type_handle;
}

/**
 * This is a helper that returns the type name for any given type.
 */
template<class T>
INLINE std::basic_string<char>
_get_type_name() {
#if defined(__clang__)
  static const size_t entire_size = sizeof(__PRETTY_FUNCTION__) - 1;
  static const size_t prefix_size = sizeof("std::basic_string<char> _get_type_name() [T = ") - 1;
  static const size_t suffix_size = sizeof("]") - 1;
  return std::string(__PRETTY_FUNCTION__ + prefix_size, entire_size - prefix_size - suffix_size);
#elif defined(__GNUC__)
  // NB. Since GCC will otherwise include the definition of std::string, we are
  // declaring this function as returning std::basic_string<char> instead.
  static const size_t entire_size = sizeof(__PRETTY_FUNCTION__) - 1;
#ifdef _GLIBCXX_USE_CXX11_ABI
  static const size_t prefix_size = sizeof("std::__cxx11::basic_string<char> _get_type_name() [with T = ") - 1;
#else
  static const size_t prefix_size = sizeof("std::basic_string<char> _get_type_name() [with T = ") - 1;
#endif
  static const size_t suffix_size = sizeof("]") - 1;
  return std::string(__PRETTY_FUNCTION__ + prefix_size, entire_size - prefix_size - suffix_size);
#elif defined(_MSC_VER)
  // MSVC allows us to use RTTI to get the type name without having it enabled.
  // However, we do have to strip the "class" or "struct" prefix.
  std::string type_name = typeid(T).name();
  size_t space = type_name.find(' ');
  if (space != std::string::npos) {
    return type_name.substr(space + 1);
  } else {
    return type_name;
  }
#elif defined(HAVE_RTTI) && !defined(__EDG__)
  // If we have RTTI, we can use it to determine the name of the base type.
  // MSVC allows us to use this with RTTI disabled as well.
  return typeid(T).name();
#else
  return "unknown";
#endif
}
