/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interfaceMakerPythonNative.h
 */

#ifndef INTERFACEMAKERPYTHONNATIVE_H
#define INTERFACEMAKERPYTHONNATIVE_H
#include <map>
#include <set>
#include "dtoolbase.h"

#include "interfaceMakerPython.h"
#include "interrogate_interface.h"
#include "cppStructType.h"

class FunctionRemap;

/**
 * An InterfaceMaker for generating complex Python function wrappers around
 * C++ code.
 */
class InterfaceMakerPythonNative : public InterfaceMakerPython {
public:
  InterfaceMakerPythonNative(InterrogateModuleDef *def);
  virtual ~InterfaceMakerPythonNative();


  virtual void write_prototypes(std::ostream &out, std::ostream *out_h);
  void write_prototypes_class(std::ostream &out, std::ostream *out_h, Object *obj) ;
  void write_prototypes_class_external(std::ostream &out, Object *obj);

  virtual void write_functions(std::ostream &out);

  virtual void write_module(std::ostream &out, std::ostream *out_h, InterrogateModuleDef *def);
  virtual void write_module_support(std::ostream &out, std::ostream *out_h, InterrogateModuleDef *def);

  void write_module_class(std::ostream &out, Object *cls);
  virtual void write_sub_module(std::ostream &out, Object *obj);

  virtual bool synthesize_this_parameter();
  virtual bool separate_overloading();

  virtual Object *record_object(TypeIndex type_index);
  Property *record_property(const InterrogateType &itype, ElementIndex element_index);

protected:
  virtual std::string get_wrapper_prefix();
  virtual std::string get_unique_prefix();
  virtual void record_function_wrapper(InterrogateFunction &ifunc,
                                       FunctionWrapperIndex wrapper_index);

  virtual void generate_wrappers();

private:
  // This enum defines the various prototypes that must be generated for the
  // specialty functions that Python requires, especially for the slotted
  // functions.
  enum WrapperType {
    WT_none,
    WT_no_params,
    WT_one_param,
    WT_binary_operator,
    WT_setattr,
    WT_getattr,
    WT_sequence_getitem,
    WT_sequence_setitem,
    WT_sequence_size,
    WT_mapping_setitem,
    WT_inquiry,
    WT_getbuffer,
    WT_releasebuffer,
    WT_iter_next,
    WT_ternary_operator,
    WT_inplace_binary_operator,
    WT_inplace_ternary_operator,
    WT_traverse,
    WT_compare,
    WT_hash,
    WT_new,
  };

  // This enum is passed to the wrapper generation functions to indicate what
  // sort of values the wrapper function is expected to return.
  enum ReturnFlags {
    // -1 on failure, 0 on success.
    RF_int = 0x100,

    // Like RF_int, but special case that it returns -1, 0, or 1.
    RF_compare = RF_int | 0x200,

    // Returns the actual return value as PyObject*.
    RF_pyobject = 0x010,

    // Returns a reference to self.
    RF_self = 0x020,

    // Assign to the coerced argument, in the case of a coercion constructor.
    RF_coerced = 0x040,

    // Don't automatically map NULL to None
    RF_preserve_null = 0x080,

    // These indicate what should be returned on error.
    RF_err_notimplemented = 0x002,
    RF_err_null = 0x004,
    RF_err_false = 0x008,

    // Decref temporary args object before returning.
    RF_decref_args = 0x1000,

    // This raises a KeyError on falsey (or -1) return value.
    RF_raise_keyerror = 0x4000,

    // Invert boolean return value.
    RF_invert_bool = 0x8000,

    // Used inside a rich comparison function.
    RF_richcompare_zero = 0x10000,
  };

  class SlottedFunctionDef {
  public:
    std::string _answer_location;
    WrapperType _wrapper_type;
    int _min_version = 0;
    std::string _wrapper_name;
    std::set<FunctionRemap*> _remaps;
    bool _keep_method;
  };

  typedef std::map<std::string, SlottedFunctionDef> SlottedFunctions;

  static bool get_slotted_function_def(Object *obj, Function *func, FunctionRemap *remap, SlottedFunctionDef &def);
  static void write_function_slot(std::ostream &out, int indent_level,
                                  const SlottedFunctions &slots,
                                  const std::string &slot, const std::string &def = "nullptr");

  void write_prototype_for_name(std::ostream &out, Function *func, const std::string &name);
  void write_prototype_for(std::ostream &out, Function *func);
  void write_function_for_top(std::ostream &out, Object *obj, Function *func);

  void write_function_for_name(std::ostream &out, Object *obj,
                               const Function::Remaps &remaps,
                               const std::string &name, std::string &expected_params,
                               bool coercion_allowed,
                               ArgsType args_type, int return_flags);
  void write_coerce_constructor(std::ostream &out, Object *obj, bool is_const);

  int collapse_default_remaps(std::map<int, std::set<FunctionRemap *> > &map_sets,
                              int max_required_args);

  bool write_function_forset(std::ostream &out,
                             const std::set<FunctionRemap*> &remaps,
                             int min_num_args, int max_num_args,
                             std::string &expected_params, int indent_level,
                             bool coercion_allowed, bool report_errors,
                             ArgsType args_type, int return_flags,
                             bool check_exceptions = true,
                             bool verify_const = true,
                             const std::string &first_expr = std::string());

  bool write_function_instance(std::ostream &out, FunctionRemap *remap,
                               int min_num_args, int max_num_args,
                               std::string &expected_params, int indent_level,
                               bool coercion_allowed, bool report_errors,
                               ArgsType args_type, int return_flags,
                               bool check_exceptions = true,
                               const std::string &first_pexpr = std::string());

  void error_return(std::ostream &out, int indent_level, int return_flags);
  void error_bad_args_return(std::ostream &out, int indent_level, int return_flags,
                             const std::string &expected_params);
  void error_raise_return(std::ostream &out, int indent_level, int return_flags,
                          const std::string &exc_type, const std::string &message,
                          const std::string &format_args = "");
  void pack_return_value(std::ostream &out, int indent_level, FunctionRemap *remap,
                         std::string return_expr, int return_flags);

  void write_make_seq(std::ostream &out, Object *obj, const std::string &ClassName,
                      const std::string &cClassName, MakeSeq *make_seq);
  void write_getset(std::ostream &out, Object *obj, Property *property);

  void write_class_prototypes(std::ostream &out) ;
  void write_class_declarations(std::ostream &out, std::ostream *out_h, Object *obj);
  void write_class_details(std::ostream &out, Object *obj);

public:
  bool is_remap_legal(FunctionRemap *remap);
  int has_coerce_constructor(CPPStructType *type);
  bool is_remap_coercion_possible(FunctionRemap *remap);
  bool is_function_legal(Function *func);
  bool is_cpp_type_legal(CPPType *ctype);
  bool isExportThisRun(CPPType *ctype);
  bool isExportThisRun(Function *func);
  bool isFunctionWithThis( Function *func);
  bool IsRunTimeTyped(const InterrogateType &itype);
  bool is_python_subclassable(CPPStructType *type);
  bool has_self_member(CPPStructType *type);

  // comunicates the cast capabilites among methods..
  struct CastDetails {
    CPPStructType   *_structType;
    std::string     _to_class_name;
    std::string     _up_cast_string;
    bool            _can_downcast;
    bool            _is_legal_py_class;
  };

  void get_valid_child_classes(std::map<std::string, CastDetails> &answer, CPPStructType *inclass, const std::string &upcast_seed = "", bool can_downcast = true);
  bool DoesInheritFromIsClass(const CPPStructType * inclass, const std::string &name);
  bool IsPandaTypedObject(CPPStructType * inclass) { return DoesInheritFromIsClass(inclass,"TypedObject"); };
  void write_python_instance(std::ostream &out, int indent_level, const std::string &return_expr, bool owns_memory, const InterrogateType &itype, bool is_const);
  bool has_get_class_type_function(CPPType *type);
  bool has_init_type_function(CPPType *type);
  int NeedsAStrFunction(const InterrogateType &itype_class);
  int NeedsAReprFunction(const InterrogateType &itype_class);
  bool NeedsARichCompareFunction(const InterrogateType &itype_class);

  void output_quoted(std::ostream &out, int indent_level, const std::string &str,
                     bool first_line=true);

  // stash the forward declarations for this compile pass..
  std::set<CPPType *> _external_imports;
};

#endif
