// Filename: interrogate_interface.cxx
// Created by:  drose (31Jul00)
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

#include "interrogate_interface.h"
#include "interrogateDatabase.h"
#include "interrogateType.h"
#include "interrogateFunction.h"

// This function adds one more directory to the list of directories
// search for interrogate (*.in) files.  In the past, this list has
// been defined the environment variable ETC_PATH, but now it is
// passed in by the code generator.
void 
interrogate_add_search_directory(const char *dirname) {
  get_interrogatedb_path().append_directory(Filename::from_os_specific(dirname));
}

bool interrogate_error_flag() {
  return InterrogateDatabase::get_ptr()->get_error_flag();
}

int
interrogate_number_of_manifests() {
  return InterrogateDatabase::get_ptr()->get_num_global_manifests();
}

ManifestIndex
interrogate_get_manifest(int n) {
  return InterrogateDatabase::get_ptr()->get_global_manifest(n);
}

ManifestIndex
interrogate_get_manifest_by_name(const char *manifest_name) {
  return InterrogateDatabase::get_ptr()->lookup_manifest_by_name(manifest_name);
}

const char *
interrogate_manifest_name(ManifestIndex manifest) {
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_name().c_str();
}

const char *
interrogate_manifest_definition(ManifestIndex manifest) {
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_definition().c_str();
}

bool
interrogate_manifest_has_type(ManifestIndex manifest) {
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).has_type();
}

TypeIndex
interrogate_manifest_get_type(ManifestIndex manifest) {
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_type();
}

bool
interrogate_manifest_has_getter(ManifestIndex manifest) {
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).has_getter();
}

FunctionIndex
interrogate_manifest_getter(ManifestIndex manifest) {
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_getter();
}

bool
interrogate_manifest_has_int_value(ManifestIndex manifest) {
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).has_int_value();
}

int
interrogate_manifest_get_int_value(ManifestIndex manifest) {
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_int_value();
}

const char *
interrogate_element_name(ElementIndex element) {
  return InterrogateDatabase::get_ptr()->get_element(element).get_name().c_str();
}

const char *
interrogate_element_scoped_name(ElementIndex element) {
  return InterrogateDatabase::get_ptr()->get_element(element).get_scoped_name().c_str();
}

ElementIndex
interrogate_get_element_by_name(const char *element_name) {
  return InterrogateDatabase::get_ptr()->lookup_element_by_name(element_name);
}

ElementIndex
interrogate_get_element_by_scoped_name(const char *element_name) {
  return InterrogateDatabase::get_ptr()->lookup_element_by_scoped_name(element_name);
}

TypeIndex
interrogate_element_type(ElementIndex element) {
  return InterrogateDatabase::get_ptr()->get_element(element).get_type();
}

bool
interrogate_element_has_getter(ElementIndex element) {
  return InterrogateDatabase::get_ptr()->get_element(element).has_getter();
}

FunctionIndex
interrogate_element_getter(ElementIndex element) {
  return InterrogateDatabase::get_ptr()->get_element(element).get_getter();
}

bool
interrogate_element_has_setter(ElementIndex element) {
  return InterrogateDatabase::get_ptr()->get_element(element).has_setter();
}

FunctionIndex
interrogate_element_setter(ElementIndex element) {
  return InterrogateDatabase::get_ptr()->get_element(element).get_setter();
}

int
interrogate_number_of_globals() {
  return InterrogateDatabase::get_ptr()->get_num_global_elements();
}

ElementIndex
interrogate_get_global(int n) {
  return InterrogateDatabase::get_ptr()->get_global_element(n);
}

int
interrogate_number_of_global_functions() {
  return InterrogateDatabase::get_ptr()->get_num_global_functions();
}

FunctionIndex
interrogate_get_global_function(int n) {
  return InterrogateDatabase::get_ptr()->get_global_function(n);
}

int
interrogate_number_of_functions() {
  return InterrogateDatabase::get_ptr()->get_num_all_functions();
}

FunctionIndex
interrogate_get_function(int n) {
  return InterrogateDatabase::get_ptr()->get_all_function(n);
}

const char *
interrogate_function_name(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).get_name().c_str();
}

const char *
interrogate_function_scoped_name(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).get_scoped_name().c_str();
}

bool
interrogate_function_has_comment(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).has_comment();
}

const char *
interrogate_function_comment(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).get_comment().c_str();
}

const char *
interrogate_function_prototype(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).get_prototype().c_str();
}

bool
interrogate_function_is_method(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).is_method();
}

TypeIndex
interrogate_function_class(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).get_class();
}

bool
interrogate_function_has_module_name(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).has_module_name();
}

const char *
interrogate_function_module_name(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).get_module_name();
}

bool
interrogate_function_is_virtual(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).is_virtual();
}

int
interrogate_function_number_of_c_wrappers(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).number_of_c_wrappers();
}

FunctionWrapperIndex
interrogate_function_c_wrapper(FunctionIndex function, int n) {
  return InterrogateDatabase::get_ptr()->get_function(function).get_c_wrapper(n);
}

int
interrogate_function_number_of_python_wrappers(FunctionIndex function) {
  return InterrogateDatabase::get_ptr()->get_function(function).number_of_python_wrappers();
}

FunctionWrapperIndex
interrogate_function_python_wrapper(FunctionIndex function, int n) {
  return InterrogateDatabase::get_ptr()->get_function(function).get_python_wrapper(n);
}

const char *
interrogate_wrapper_name(FunctionWrapperIndex wrapper) {
  static string result;
  result = InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_name();
  return result.c_str();
}

bool
interrogate_wrapper_is_callable_by_name(FunctionWrapperIndex wrapper) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).is_callable_by_name();
}

bool
interrogate_wrapper_has_return_value(FunctionWrapperIndex wrapper) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).has_return_value();
}

TypeIndex
interrogate_wrapper_return_type(FunctionWrapperIndex wrapper) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_return_type();
}

bool
interrogate_wrapper_caller_manages_return_value(FunctionWrapperIndex wrapper) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).caller_manages_return_value();
}

FunctionIndex
interrogate_wrapper_return_value_destructor(FunctionWrapperIndex wrapper) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_return_value_destructor();
}

int
interrogate_wrapper_number_of_parameters(FunctionWrapperIndex wrapper) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).number_of_parameters();
}

TypeIndex
interrogate_wrapper_parameter_type(FunctionWrapperIndex wrapper, int n) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).parameter_get_type(n);
}

bool
interrogate_wrapper_parameter_has_name(FunctionWrapperIndex wrapper, int n) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).parameter_has_name(n);
}

const char *
interrogate_wrapper_parameter_name(FunctionWrapperIndex wrapper, int n) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).parameter_get_name(n).c_str();
}

bool
interrogate_wrapper_parameter_is_this(FunctionWrapperIndex wrapper, int n) {
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).parameter_is_this(n);
}

bool
interrogate_wrapper_has_pointer(FunctionWrapperIndex wrapper) {
  return (InterrogateDatabase::get_ptr()->get_fptr(wrapper) != (void *)NULL);
}

void *
interrogate_wrapper_pointer(FunctionWrapperIndex wrapper) {
  return InterrogateDatabase::get_ptr()->get_fptr(wrapper);
}

const char *
interrogate_wrapper_unique_name(FunctionWrapperIndex wrapper) {
  static string result;
  result = InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_unique_name();
  return result.c_str();
}

FunctionWrapperIndex
interrogate_get_wrapper_by_unique_name(const char *unique_name) {
  return InterrogateDatabase::get_ptr()->get_wrapper_by_unique_name(unique_name);
}

int
interrogate_number_of_global_types() {
  return InterrogateDatabase::get_ptr()->get_num_global_types();
}

TypeIndex
interrogate_get_global_type(int n) {
  return InterrogateDatabase::get_ptr()->get_global_type(n);
}

int
interrogate_number_of_types() {
  return InterrogateDatabase::get_ptr()->get_num_all_types();
}

TypeIndex
interrogate_get_type(int n) {
  return InterrogateDatabase::get_ptr()->get_all_type(n);
}

TypeIndex
interrogate_get_type_by_name(const char *type_name) {
  return InterrogateDatabase::get_ptr()->lookup_type_by_name(type_name);
}

TypeIndex
interrogate_get_type_by_scoped_name(const char *type_name) {
  return InterrogateDatabase::get_ptr()->lookup_type_by_scoped_name(type_name);
}

TypeIndex
interrogate_get_type_by_true_name(const char *type_name) {
  return InterrogateDatabase::get_ptr()->lookup_type_by_true_name(type_name);
}

const char *
interrogate_type_name(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_name().c_str();
}

const char *
interrogate_type_scoped_name(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_scoped_name().c_str();
}

const char *
interrogate_type_true_name(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_true_name().c_str();
}

bool
interrogate_type_is_nested(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_nested();
}

TypeIndex
interrogate_type_outer_class(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_outer_class();
}

bool
interrogate_type_has_comment(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).has_comment();
}

const char *
interrogate_type_comment(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_comment().c_str();
}

bool
interrogate_type_has_module_name(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).has_module_name();
}

const char *
interrogate_type_module_name(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_module_name();
}

bool
interrogate_type_is_atomic(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_atomic();
}

AtomicToken
interrogate_type_atomic_token(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_atomic_token();
}

bool
interrogate_type_is_unsigned(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_unsigned();
}

bool
interrogate_type_is_signed(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_signed();
}

bool
interrogate_type_is_long(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_long();
}

bool
interrogate_type_is_longlong(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_longlong();
}

bool
interrogate_type_is_short(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_short();
}

bool
interrogate_type_is_wrapped(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_wrapped();
}

bool
interrogate_type_is_pointer(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_pointer();
}

bool
interrogate_type_is_const(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_const();
}

TypeIndex
interrogate_type_wrapped_type(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_wrapped_type();
}

bool
interrogate_type_is_enum(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_enum();
}

int
interrogate_type_number_of_enum_values(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_enum_values();
}

const char *
interrogate_type_enum_value_name(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_enum_value_name(n).c_str();
}

const char *
interrogate_type_enum_value_scoped_name(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_enum_value_scoped_name(n).c_str();
}

int
interrogate_type_enum_value(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_enum_value(n);
}

bool
interrogate_type_is_struct(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_struct();
}

bool
interrogate_type_is_class(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_class();
}

bool
interrogate_type_is_union(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_union();
}

bool
interrogate_type_is_fully_defined(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_fully_defined();
}

bool
interrogate_type_is_unpublished(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).is_unpublished();
}

int
interrogate_type_number_of_constructors(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_constructors();
}

FunctionIndex
interrogate_type_get_constructor(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_constructor(n);
}

bool
interrogate_type_has_destructor(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).has_destructor();
}

bool
interrogate_type_destructor_is_inherited(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).destructor_is_inherited();
}

FunctionIndex
interrogate_type_get_destructor(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_destructor();
}

int
interrogate_type_number_of_elements(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_elements();
}

ElementIndex
interrogate_type_get_element(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_element(n);
}

int
interrogate_type_number_of_methods(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_methods();
}

FunctionIndex
interrogate_type_get_method(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_method(n);
}

int
interrogate_type_number_of_casts(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_casts();
}

FunctionIndex
interrogate_type_get_cast(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_cast(n);
}

int
interrogate_type_number_of_derivations(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_derivations();
}

TypeIndex
interrogate_type_get_derivation(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_derivation(n);
}

bool
interrogate_type_derivation_has_upcast(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_has_upcast(n);
}

FunctionIndex
interrogate_type_get_upcast(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_get_upcast(n);
}

bool
interrogate_type_derivation_downcast_is_impossible(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_downcast_is_impossible(n);
}

bool
interrogate_type_derivation_has_downcast(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_has_downcast(n);
}

FunctionIndex
interrogate_type_get_downcast(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_get_downcast(n);
}

int
interrogate_type_number_of_nested_types(TypeIndex type) {
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_nested_types();
}

TypeIndex
interrogate_type_get_nested_type(TypeIndex type, int n) {
  return InterrogateDatabase::get_ptr()->get_type(type).get_nested_type(n);
}
