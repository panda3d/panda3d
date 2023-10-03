/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogate_interface.cxx
 * @author drose
 * @date 2000-07-31
 */

#include "interrogate_interface.h"
#include "interrogateDatabase.h"
#include "interrogateType.h"
#include "interrogateFunction.h"
#include "config_interrogatedb.h"

using std::string;

// This function adds one more directory to the list of directories search for
// interrogate (*.in) files.  In the past, this list has been defined the
// environment variable ETC_PATH, but now it is passed in by the code
// generator.
void
interrogate_add_search_directory(const char *dirname) {
  // cerr << "interrogate_add_search_directory(" << dirname << ")\n";
  interrogatedb_path.append_directory(Filename::from_os_specific(dirname));
}

// This function works similar to the above, but adds a complete path string--
// a list of multiple directories, separated by the standard delimiter--to the
// search path.
void
interrogate_add_search_path(const char *pathstring) {
  // cerr << "interrogate_add_search_path(" << pathstring << ")\n";
  interrogatedb_path.append_path(pathstring);
}

bool interrogate_error_flag() {
  // cerr << "interrogate_error_flag\n";
  return InterrogateDatabase::get_ptr()->get_error_flag();
}

int
interrogate_number_of_manifests() {
  // cerr << "interrogate_number_of_manifests\n";
  return InterrogateDatabase::get_ptr()->get_num_global_manifests();
}

ManifestIndex
interrogate_get_manifest(int n) {
  // cerr << "interrogate_get_manifest(" << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_global_manifest(n);
}

ManifestIndex
interrogate_get_manifest_by_name(const char *manifest_name) {
  // cerr << "interrogate_get_manifest_by_name(" << manifest_name << ")\n";
  return InterrogateDatabase::get_ptr()->lookup_manifest_by_name(manifest_name);
}

const char *
interrogate_manifest_name(ManifestIndex manifest) {
  // cerr << "interrogate_manifest_name(" << manifest << ")\n";
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_name().c_str();
}

const char *
interrogate_manifest_definition(ManifestIndex manifest) {
  // cerr << "interrogate_manifest_definition(" << manifest << ")\n";
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_definition().c_str();
}

bool
interrogate_manifest_has_type(ManifestIndex manifest) {
  // cerr << "interrogate_manifest_has_type(" << manifest << ")\n";
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).has_type();
}

TypeIndex
interrogate_manifest_get_type(ManifestIndex manifest) {
  // cerr << "interrogate_manifest_get_type(" << manifest << ")\n";
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_type();
}

bool
interrogate_manifest_has_getter(ManifestIndex manifest) {
  // cerr << "interrogate_manifest_has_getter(" << manifest << ")\n";
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).has_getter();
}

FunctionIndex
interrogate_manifest_getter(ManifestIndex manifest) {
  // cerr << "interrogate_manifest_getter(" << manifest << ")\n";
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_getter();
}

bool
interrogate_manifest_has_int_value(ManifestIndex manifest) {
  // cerr << "interrogate_manifest_has_int_value(" << manifest << ")\n";
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).has_int_value();
}

int
interrogate_manifest_get_int_value(ManifestIndex manifest) {
  // cerr << "interrogate_manifest_get_int_value(" << manifest << ")\n";
  return InterrogateDatabase::get_ptr()->get_manifest(manifest).get_int_value();
}

const char *
interrogate_element_name(ElementIndex element) {
  // cerr << "interrogate_element_name(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_name().c_str();
}

const char *
interrogate_element_scoped_name(ElementIndex element) {
  // cerr << "interrogate_element_scoped_name(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_scoped_name().c_str();
}

bool
interrogate_element_has_comment(ElementIndex element) {
  // cerr << "interrogate_element_has_comment(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).has_comment();
}

const char *
interrogate_element_comment(ElementIndex element) {
  // cerr << "interrogate_element_comment(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_comment().c_str();
}

ElementIndex
interrogate_get_element_by_name(const char *element_name) {
  // cerr << "interrogate_get_element_by_name(" << element_name << ")\n";
  return InterrogateDatabase::get_ptr()->lookup_element_by_name(element_name);
}

ElementIndex
interrogate_get_element_by_scoped_name(const char *element_name) {
  // cerr << "interrogate_get_element_by_scoped_name(" << element_name <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->lookup_element_by_scoped_name(element_name);
}

TypeIndex
interrogate_element_type(ElementIndex element) {
  // cerr << "interrogate_element_type(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_type();
}

bool
interrogate_element_has_getter(ElementIndex element) {
  // cerr << "interrogate_element_has_getter(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).has_getter();
}

FunctionIndex
interrogate_element_getter(ElementIndex element) {
  // cerr << "interrogate_element_getter(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_getter();
}

bool
interrogate_element_has_setter(ElementIndex element) {
  // cerr << "interrogate_element_has_setter(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).has_setter();
}

FunctionIndex
interrogate_element_setter(ElementIndex element) {
  // cerr << "interrogate_element_setter(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_setter();
}

bool
interrogate_element_has_has_function(ElementIndex element) {
  // cerr << "interrogate_element_has_has_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).has_has_function();
}

FunctionIndex
interrogate_element_has_function(ElementIndex element) {
  // cerr << "interrogate_element_has_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_has_function();
}

bool
interrogate_element_has_clear_function(ElementIndex element) {
  // cerr << "interrogate_element_has_clear_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).has_clear_function();
}

FunctionIndex
interrogate_element_clear_function(ElementIndex element) {
  // cerr << "interrogate_element_clear_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_clear_function();
}

bool
interrogate_element_has_del_function(ElementIndex element) {
  // cerr << "interrogate_element_has_del_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).has_del_function();
}

FunctionIndex
interrogate_element_del_function(ElementIndex element) {
  // cerr << "interrogate_element_del_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_del_function();
}

bool
interrogate_element_has_insert_function(ElementIndex element) {
  // cerr << "interrogate_element_has_insert_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).has_insert_function();
}

FunctionIndex
interrogate_element_insert_function(ElementIndex element) {
  // cerr << "interrogate_element_insert_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_insert_function();
}

bool
interrogate_element_has_getkey_function(ElementIndex element) {
  // cerr << "interrogate_element_has_getkey_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).has_getkey_function();
}

FunctionIndex
interrogate_element_getkey_function(ElementIndex element) {
  // cerr << "interrogate_element_getkey_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_getkey_function();
}

FunctionIndex
interrogate_element_length_function(ElementIndex element) {
  // cerr << "interrogate_element_length_function(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).get_length_function();
}

bool
interrogate_element_is_sequence(ElementIndex element) {
  // cerr << "interrogate_element_is_sequence(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).is_sequence();
}

bool
interrogate_element_is_mapping(ElementIndex element) {
  // cerr << "interrogate_element_is_mapping(" << element << ")\n";
  return InterrogateDatabase::get_ptr()->get_element(element).is_mapping();
}

int
interrogate_number_of_globals() {
  // cerr << "interrogate_number_of_globals()\n";
  return InterrogateDatabase::get_ptr()->get_num_global_elements();
}

ElementIndex
interrogate_get_global(int n) {
  // cerr << "interrogate_get_global(" << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_global_element(n);
}

int
interrogate_number_of_global_functions() {
  // cerr << "interrogate_number_of_global_functions()\n";
  return InterrogateDatabase::get_ptr()->get_num_global_functions();
}

FunctionIndex
interrogate_get_global_function(int n) {
  // cerr << "interrogate_get_global_function(" << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_global_function(n);
}

int
interrogate_number_of_functions() {
  // cerr << "interrogate_number_of_functions()\n";
  return InterrogateDatabase::get_ptr()->get_num_all_functions();
}

FunctionIndex
interrogate_get_function(int n) {
  // cerr << "interrogate_get_function(" << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_all_function(n);
}

const char *
interrogate_function_name(FunctionIndex function) {
  // cerr << "interrogate_function_name(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).get_name().c_str();
}

const char *
interrogate_function_scoped_name(FunctionIndex function) {
  // cerr << "interrogate_function_scoped_name(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).get_scoped_name().c_str();
}

bool
interrogate_function_has_comment(FunctionIndex function) {
  // cerr << "interrogate_function_has_comment(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).has_comment();
}

const char *
interrogate_function_comment(FunctionIndex function) {
  // cerr << "interrogate_function_comment(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).get_comment().c_str();
}

const char *
interrogate_function_prototype(FunctionIndex function) {
  // cerr << "interrogate_function_prototype(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).get_prototype().c_str();
}

bool
interrogate_function_is_method(FunctionIndex function) {
  // cerr << "interrogate_function_is_method(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).is_method();
}

TypeIndex
interrogate_function_class(FunctionIndex function) {
  // cerr << "interrogate_function_class(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).get_class();
}

bool
interrogate_function_is_unary_op(FunctionIndex function) {
  // cerr << "interrogate_function_is_unary_op(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).is_unary_op();
}

bool
interrogate_function_is_operator_typecast(FunctionIndex function) {
  // cerr << "interrogate_function_is_operator_typecast(" << function <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).is_operator_typecast();
}

bool
interrogate_function_is_constructor(FunctionIndex function) {
  // cerr << "interrogate_function_is_constructor(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).is_constructor();
}

bool
interrogate_function_is_destructor(FunctionIndex function) {
  // cerr << "interrogate_function_is_destructor(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).is_destructor();
}

bool
interrogate_function_has_module_name(FunctionIndex function) {
  // cerr << "interrogate_function_has_module_name(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).has_module_name();
}

const char *
interrogate_function_module_name(FunctionIndex function) {
  // cerr << "interrogate_function_module_name(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).get_module_name();
}

bool
interrogate_function_has_library_name(FunctionIndex function) {
  // cerr << "interrogate_function_has_library_name(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).has_library_name();
}

const char *
interrogate_function_library_name(FunctionIndex function) {
  // cerr << "interrogate_function_library_name(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).get_library_name();
}



bool
interrogate_function_is_virtual(FunctionIndex function) {
  // cerr << "interrogate_function_is_virtual(" << function << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).is_virtual();
}

int
interrogate_function_number_of_c_wrappers(FunctionIndex function) {
  // cerr << "interrogate_function_number_of_c_wrappers(" << function <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).number_of_c_wrappers();
}

FunctionWrapperIndex
interrogate_function_c_wrapper(FunctionIndex function, int n) {
  // cerr << "interrogate_function_c_wrapper(" << function << ", " << n <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).get_c_wrapper(n);
}

int
interrogate_function_number_of_python_wrappers(FunctionIndex function) {
  // cerr << "interrogate_function_number_of_python_wrappers(" << function <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).number_of_python_wrappers();
}

FunctionWrapperIndex
interrogate_function_python_wrapper(FunctionIndex function, int n) {
  // cerr << "interrogate_function_python_wrapper(" << function << ", " << n
  // << ")\n";
  return InterrogateDatabase::get_ptr()->get_function(function).get_python_wrapper(n);
}

const char *
interrogate_wrapper_name(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_name(" << wrapper << ")\n";
  static string result;
  result = InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_name();
  return result.c_str();
}

FunctionIndex
interrogate_wrapper_function(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_function(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_function();
}

bool
interrogate_wrapper_is_callable_by_name(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_is_callable_by_name(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).is_callable_by_name();
}

bool
interrogate_wrapper_is_copy_constructor(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_is_copy_constructor(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).is_copy_constructor();
}

bool
interrogate_wrapper_is_coerce_constructor(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_is_coerce_constructor(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).is_coerce_constructor();
}

bool
interrogate_wrapper_is_extension(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_is_extension(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).is_extension();
}

bool
interrogate_wrapper_is_deprecated(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_is_deprecated(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).is_deprecated();
}

bool
interrogate_wrapper_has_comment(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_has_comment(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).has_comment();
}

const char *
interrogate_wrapper_comment(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_comment(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_comment().c_str();
}

bool
interrogate_wrapper_has_return_value(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_has_return_value(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).has_return_value();
}

TypeIndex
interrogate_wrapper_return_type(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_return_type(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_return_type();
}

bool
interrogate_wrapper_caller_manages_return_value(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_caller_manages_return_value(" << wrapper <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).caller_manages_return_value();
}

FunctionIndex
interrogate_wrapper_return_value_destructor(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_return_value_destructor(" << wrapper <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_return_value_destructor();
}

int
interrogate_wrapper_number_of_parameters(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_number_of_parameters(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).number_of_parameters();
}

TypeIndex
interrogate_wrapper_parameter_type(FunctionWrapperIndex wrapper, int n) {
  // cerr << "interrogate_wrapper_parameter_type(" << wrapper << ", " << n <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).parameter_get_type(n);
}

bool
interrogate_wrapper_parameter_has_name(FunctionWrapperIndex wrapper, int n) {
  // cerr << "interrogate_wrapper_parameter_has_name(" << wrapper << ", " << n
  // << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).parameter_has_name(n);
}

const char *
interrogate_wrapper_parameter_name(FunctionWrapperIndex wrapper, int n) {
  // cerr << "interrogate_wrapper_parameter_name(" << wrapper << ", " << n <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).parameter_get_name(n).c_str();
}

bool
interrogate_wrapper_parameter_is_this(FunctionWrapperIndex wrapper, int n) {
  // cerr << "interrogate_wrapper_is_this(" << wrapper << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).parameter_is_this(n);
}

bool
interrogate_wrapper_parameter_is_optional(FunctionWrapperIndex wrapper, int n) {
  // cerr << "interrogate_wrapper_is_optional(" << wrapper << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper(wrapper).parameter_is_optional(n);
}

bool
interrogate_wrapper_has_pointer(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_has_pointer(" << wrapper << ")\n";
  return (InterrogateDatabase::get_ptr()->get_fptr(wrapper) != nullptr);
}

void *
interrogate_wrapper_pointer(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_pointer(" << wrapper << ")\n";
  return InterrogateDatabase::get_ptr()->get_fptr(wrapper);
}

const char *
interrogate_wrapper_unique_name(FunctionWrapperIndex wrapper) {
  // cerr << "interrogate_wrapper_unique_name(" << wrapper << ")\n";
  static string result;
  result = InterrogateDatabase::get_ptr()->get_wrapper(wrapper).get_unique_name();
  return result.c_str();
}

FunctionWrapperIndex
interrogate_get_wrapper_by_unique_name(const char *unique_name) {
  // cerr << "interrogate_get_wrapper_by_unique_name(" << unique_name <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_wrapper_by_unique_name(unique_name);
}

const char *
interrogate_make_seq_seq_name(MakeSeqIndex make_seq) {
  // cerr << "interrogate_make_seq_seq_name(" << make_seq << ")\n";
  static string result;
  result = InterrogateDatabase::get_ptr()->get_make_seq(make_seq).get_name();
  return result.c_str();
}

const char *
interrogate_make_seq_scoped_name(MakeSeqIndex make_seq) {
  // cerr << "interrogate_make_seq_seq_name(" << make_seq << ")\n";
  static string result;
  result = InterrogateDatabase::get_ptr()->get_make_seq(make_seq).get_scoped_name();
  return result.c_str();
}

bool
interrogate_make_seq_has_comment(MakeSeqIndex make_seq) {
  // cerr << "interrogate_make_seq_has_comment(" << make_seq << ")\n";
  return InterrogateDatabase::get_ptr()->get_make_seq(make_seq).has_comment();
}

const char *
interrogate_make_seq_comment(MakeSeqIndex make_seq) {
  // cerr << "interrogate_make_seq_comment(" << make_seq << ")\n";
  return InterrogateDatabase::get_ptr()->get_make_seq(make_seq).get_comment().c_str();
}

const char *
interrogate_make_seq_num_name(MakeSeqIndex make_seq) {
  // cerr << "interrogate_make_seq_num_name(" << make_seq << ")\n";
  FunctionIndex function = InterrogateDatabase::get_ptr()->get_make_seq(make_seq).get_length_getter();
  return interrogate_function_name(function);
}

const char *
interrogate_make_seq_element_name(MakeSeqIndex make_seq) {
  // cerr << "interrogate_make_seq_element_name(" << make_seq << ")\n";
  FunctionIndex function = InterrogateDatabase::get_ptr()->get_make_seq(make_seq).get_element_getter();
  return interrogate_function_name(function);
}

FunctionIndex
interrogate_make_seq_num_getter(MakeSeqIndex make_seq) {
  // cerr << "interrogate_make_seq_num_getter(" << make_seq << ")\n";
  return InterrogateDatabase::get_ptr()->get_make_seq(make_seq).get_length_getter();
}

FunctionIndex
interrogate_make_seq_element_getter(MakeSeqIndex make_seq) {
  // cerr << "interrogate_make_seq_element_getter(" << make_seq << ")\n";
  return InterrogateDatabase::get_ptr()->get_make_seq(make_seq).get_element_getter();
}

int
interrogate_number_of_global_types() {
  // cerr << "interrogate_number_of_global_types()\n";
  return InterrogateDatabase::get_ptr()->get_num_global_types();
}

TypeIndex
interrogate_get_global_type(int n) {
  // cerr << "interrogate_get_global_type(" << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_global_type(n);
}

int
interrogate_number_of_types() {
  // cerr << "interrogate_number_of_types()\n";
  return InterrogateDatabase::get_ptr()->get_num_all_types();
}

TypeIndex
interrogate_get_type(int n) {
  // cerr << "interrogate_get_type(" << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_all_type(n);
}

TypeIndex
interrogate_get_type_by_name(const char *type_name) {
  // cerr << "interrogate_get_type_by_name(" << type_name << ")\n";
  return InterrogateDatabase::get_ptr()->lookup_type_by_name(type_name);
}

TypeIndex
interrogate_get_type_by_scoped_name(const char *type_name) {
  // cerr << "interrogate_get_type_by_scoped_name(" << type_name << ")\n";
  return InterrogateDatabase::get_ptr()->lookup_type_by_scoped_name(type_name);
}

TypeIndex
interrogate_get_type_by_true_name(const char *type_name) {
  // cerr << "interrogate_get_type_by_true_name(" << type_name << ")\n";
  return InterrogateDatabase::get_ptr()->lookup_type_by_true_name(type_name);
}

bool
interrogate_type_is_global(TypeIndex type) {
  // cerr << "interrogate_type_is_global(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_global();
}

bool
interrogate_type_is_deprecated(TypeIndex type) {
  // cerr << "interrogate_type_is_deprecated(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_deprecated();
}

const char *
interrogate_type_name(TypeIndex type) {
  // cerr << "interrogate_type_name(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_name().c_str();
}

const char *
interrogate_type_scoped_name(TypeIndex type) {
  // cerr << "interrogate_type_scoped_name(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_scoped_name().c_str();
}

const char *
interrogate_type_true_name(TypeIndex type) {
  // cerr << "interrogate_type_true_name(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_true_name().c_str();
}

bool
interrogate_type_is_nested(TypeIndex type) {
  // cerr << "interrogate_type_is_nested(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_nested();
}

TypeIndex
interrogate_type_outer_class(TypeIndex type) {
  // cerr << "interrogate_type_outer_class(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_outer_class();
}

bool
interrogate_type_has_comment(TypeIndex type) {
  // cerr << "interrogate_type_has_comment(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).has_comment();
}

const char *
interrogate_type_comment(TypeIndex type) {
  // cerr << "interrogate_type_comment(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_comment().c_str();
}

bool
interrogate_type_has_module_name(TypeIndex type) {
  // cerr << "interrogate_type_has_module_name(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).has_module_name();
}

const char *
interrogate_type_module_name(TypeIndex type) {
  // cerr << "interrogate_type_module_name(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_module_name();
}

bool
interrogate_type_has_library_name(TypeIndex type) {
  // cerr << "interrogate_type_has_library_name(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).has_library_name();
}

const char *
interrogate_type_library_name(TypeIndex type) {
  // cerr << "interrogate_type_library_name(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_library_name();
}


bool
interrogate_type_is_atomic(TypeIndex type) {
  // cerr << "interrogate_type_is_atomic(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_atomic();
}

AtomicToken
interrogate_type_atomic_token(TypeIndex type) {
  // cerr << "interrogate_type_atomic_token(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_atomic_token();
}

bool
interrogate_type_is_unsigned(TypeIndex type) {
  // cerr << "interrogate_type_is_unsigned(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_unsigned();
}

bool
interrogate_type_is_signed(TypeIndex type) {
  // cerr << "interrogate_type_is_signed(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_signed();
}

bool
interrogate_type_is_long(TypeIndex type) {
  // cerr << "interrogate_type_is_long(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_long();
}

bool
interrogate_type_is_longlong(TypeIndex type) {
  // cerr << "interrogate_type_is_longlong(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_longlong();
}

bool
interrogate_type_is_short(TypeIndex type) {
  // cerr << "interrogate_type_is_short(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_short();
}

bool
interrogate_type_is_wrapped(TypeIndex type) {
  // cerr << "interrogate_type_is_wrapped(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_wrapped();
}

bool
interrogate_type_is_pointer(TypeIndex type) {
  // cerr << "interrogate_type_is_pointer(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_pointer();
}

bool
interrogate_type_is_const(TypeIndex type) {
  // cerr << "interrogate_type_is_const(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_const();
}

bool
interrogate_type_is_typedef(TypeIndex type) {
  // cerr << "interrogate_type_is_typedef(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_typedef();
}

TypeIndex
interrogate_type_wrapped_type(TypeIndex type) {
  // cerr << "interrogate_type_wrapped_type(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_wrapped_type();
}

bool
interrogate_type_is_array(TypeIndex type) {
  // cerr << "interrogate_type_is_array(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_array();
}

int
interrogate_type_array_size(TypeIndex type) {
  // cerr << "interrogate_type_array_size(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_array_size();
}

bool
interrogate_type_is_enum(TypeIndex type) {
  // cerr << "interrogate_type_is_enum(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_enum();
}

bool
interrogate_type_is_scoped_enum(TypeIndex type) {
  // cerr << "interrogate_type_is_scoped_enum(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_scoped_enum();
}

int
interrogate_type_number_of_enum_values(TypeIndex type) {
  // cerr << "interrogate_type_number_of_enum_values(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_enum_values();
}

const char *
interrogate_type_enum_value_name(TypeIndex type, int n) {
  // cerr << "interrogate_type_enum_value_name(" << type << ", " << n <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_enum_value_name(n).c_str();
}

const char *
interrogate_type_enum_value_scoped_name(TypeIndex type, int n) {
  // cerr << "interrogate_type_enum_value_scoped_name(" << type << ", " << n
  // << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_enum_value_scoped_name(n).c_str();
}

const char *
interrogate_type_enum_value_comment(TypeIndex type, int n) {
  // cerr << "interrogate_type_enum_value_comment(" << type << ", " << n <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_enum_value_comment(n).c_str();
}

int
interrogate_type_enum_value(TypeIndex type, int n) {
  // cerr << "interrogate_type_enum_value(" << type << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_enum_value(n);
}

bool
interrogate_type_is_struct(TypeIndex type) {
  // cerr << "interrogate_type_is_struct(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_struct();
}

bool
interrogate_type_is_class(TypeIndex type) {
  // cerr << "interrogate_type_is_class(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_class();
}

bool
interrogate_type_is_union(TypeIndex type) {
  // cerr << "interrogate_type_is_union(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_union();
}

bool
interrogate_type_is_fully_defined(TypeIndex type) {
  // cerr << "interrogate_type_is_fully_defined(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_fully_defined();
}

bool
interrogate_type_is_unpublished(TypeIndex type) {
  // cerr << "interrogate_type_is_unpublished(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_unpublished();
}

int
interrogate_type_number_of_constructors(TypeIndex type) {
  // cerr << "interrogate_type_number_of_constructors(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_constructors();
}

FunctionIndex
interrogate_type_get_constructor(TypeIndex type, int n) {
  // cerr << "interrogate_type_get_constructor(" << type << ", " << n <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_constructor(n);
}

bool
interrogate_type_has_destructor(TypeIndex type) {
  // cerr << "interrogate_type_has_destructor(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).has_destructor();
}

bool
interrogate_type_destructor_is_inherited(TypeIndex type) {
  // cerr << "interrogate_type_destructor_is_inherited(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).destructor_is_inherited();
}

FunctionIndex
interrogate_type_get_destructor(TypeIndex type) {
  // cerr << "interrogate_type_get_destructor(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_destructor();
}

int
interrogate_type_number_of_elements(TypeIndex type) {
  // cerr << "interrogate_type_number_of_elements(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_elements();
}

ElementIndex
interrogate_type_get_element(TypeIndex type, int n) {
  // cerr << "interrogate_type_get_element(" << type << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_element(n);
}

int
interrogate_type_number_of_methods(TypeIndex type) {
  // cerr << "interrogate_type_number_of_methods(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_methods();
}

FunctionIndex
interrogate_type_get_method(TypeIndex type, int n) {
  // cerr << "interrogate_type_get_method(" << type << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_method(n);
}

int
interrogate_type_number_of_make_seqs(TypeIndex type) {
  // cerr << "interrogate_type_number_of_make_seqs(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_make_seqs();
}

MakeSeqIndex
interrogate_type_get_make_seq(TypeIndex type, int n) {
  // cerr << "interrogate_type_get_make_seq(" << type << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_make_seq(n);
}

int
interrogate_type_number_of_casts(TypeIndex type) {
  // cerr << "interrogate_type_number_of_casts(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_casts();
}

FunctionIndex
interrogate_type_get_cast(TypeIndex type, int n) {
  // cerr << "interrogate_type_get_cast(" << type << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_cast(n);
}

int
interrogate_type_number_of_derivations(TypeIndex type) {
  // cerr << "interrogate_type_number_of_derivations(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_derivations();
}

TypeIndex
interrogate_type_get_derivation(TypeIndex type, int n) {
  // cerr << "interrogate_type_get_derivation(" << type << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_derivation(n);
}

bool
interrogate_type_is_final(TypeIndex type) {
  // cerr << "interrogate_type_is_final(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).is_final();
}

bool
interrogate_type_derivation_has_upcast(TypeIndex type, int n) {
  // cerr << "interrogate_type_derivation_has_upcast(" << type << ", " << n <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_has_upcast(n);
}

FunctionIndex
interrogate_type_get_upcast(TypeIndex type, int n) {
  // cerr << "interrogate_type_get_upcast(" << type << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_get_upcast(n);
}

bool
interrogate_type_derivation_downcast_is_impossible(TypeIndex type, int n) {
  // cerr << "interrogate_type_derivation_downcast_is_impossible(" << type <<
  // ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_downcast_is_impossible(n);
}

bool
interrogate_type_derivation_has_downcast(TypeIndex type, int n) {
  // cerr << "interrogate_type_derivation_has_downcast(" << type << ", " << n
  // << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_has_downcast(n);
}

FunctionIndex
interrogate_type_get_downcast(TypeIndex type, int n) {
  // cerr << "interrogate_type_get_downcast(" << type << ", " << n << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).derivation_get_downcast(n);
}

int
interrogate_type_number_of_nested_types(TypeIndex type) {
  // cerr << "interrogate_type_number_of_nested_types(" << type << ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).number_of_nested_types();
}

TypeIndex
interrogate_type_get_nested_type(TypeIndex type, int n) {
  // cerr << "interrogate_type_get_nested_type(" << type << ", " << n <<
  // ")\n";
  return InterrogateDatabase::get_ptr()->get_type(type).get_nested_type(n);
}
