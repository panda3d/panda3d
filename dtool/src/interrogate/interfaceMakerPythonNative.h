// Filename: InterfaceMakerPythonNative.h
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

#ifndef INTERFACEMAKERPYTHONNATIVE_H
#define INTERFACEMAKERPYTHONNATIVE_H
#include "map"
#include "set"
#include "dtoolbase.h"

#include "interfaceMakerPython.h"
#include "interrogate_interface.h"
#include "cppStructType.h"

class FunctionRemap;

////////////////////////////////////////////////////////////////////
//       Class : InterfaceMakerPythonNative
// Description : An InterfaceMaker for generating complex Python
//               function wrappers around C++ code.
////////////////////////////////////////////////////////////////////
class InterfaceMakerPythonNative : public InterfaceMakerPython {
public:
  InterfaceMakerPythonNative(InterrogateModuleDef *def);
  virtual ~InterfaceMakerPythonNative();


  virtual void write_prototypes(ostream &out, ostream *out_h);
  void write_prototypes_class(ostream &out, ostream *out_h, Object *obj) ;
  void write_prototypes_class_external(ostream &out, Object *obj);

  virtual void write_functions(ostream &out);

  virtual void write_module(ostream &out, ostream *out_h, InterrogateModuleDef *def);
  virtual void write_module_support(ostream &out, ostream *out_h, InterrogateModuleDef *def);

  void write_module_class(ostream &out, Object *cls);
  virtual void write_sub_module(ostream &out, Object *obj);

  virtual bool synthesize_this_parameter();
  virtual bool separate_overloading();

  virtual Object *record_object(TypeIndex type_index);

protected:
  virtual string get_wrapper_prefix();
  virtual string get_unique_prefix();
  virtual void record_function_wrapper(InterrogateFunction &ifunc,
                                       FunctionWrapperIndex wrapper_index);

  virtual void generate_wrappers();

private:
  // This enum defines the various prototypes that must be generated
  // for the specialty functions that Python requires, especially for
  // the slotted functions.
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
  };

  // This enum is passed to the wrapper generation functions to indicate
  // what sort of values the wrapper function is expected to return.
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

    // These indicate what should be returned on error.
    RF_err_notimplemented = 0x002,
    RF_err_null = 0x004,
    RF_err_false = 0x008,

    // Decref temporary args object before returning.
    RF_decref_args = 0x1000,
  };

  class SlottedFunctionDef {
  public:
    string _answer_location;
    WrapperType _wrapper_type;
    int _min_version;
    string _wrapper_name;
    set<FunctionRemap*> _remaps;
    bool _keep_method;
  };

  typedef std::map<string, SlottedFunctionDef> SlottedFunctions;

  static bool get_slotted_function_def(Object *obj, Function *func, FunctionRemap *remap, SlottedFunctionDef &def);
  static void write_function_slot(ostream &out, int indent_level,
                                  const SlottedFunctions &slots,
                                  const string &slot, const string &def = "0");

  void write_prototype_for_name(ostream &out, Function *func, const std::string &name);
  void write_prototype_for(ostream &out, Function *func);
  void write_function_for_top(ostream &out, Object *obj, Function *func);

  void write_function_for_name(ostream &out, Object *obj,
                               const Function::Remaps &remaps,
                               const std::string &name, string &expected_params,
                               bool coercion_allowed,
                               ArgsType args_type, int return_flags);
  void write_coerce_constructor(ostream &out, Object *obj, bool is_const);

  int collapse_default_remaps(std::map<int, std::set<FunctionRemap *> > &map_sets,
                              int max_required_args);

  void write_function_forset(ostream &out,
                             const std::set<FunctionRemap*> &remaps,
                             int min_num_args, int max_num_args,
                             string &expected_params, int indent_level,
                             bool coercion_allowed, bool report_errors,
                             ArgsType args_type, int return_flags,
                             bool check_exceptions = true,
                             bool verify_const = true,
                             const string &first_expr = string());

  void write_function_instance(ostream &out, FunctionRemap *remap,
                               int min_num_args, int max_num_args,
                               string &expected_params, int indent_level,
                               bool coercion_allowed, bool report_errors,
                               ArgsType args_type, int return_flags,
                               bool check_exceptions = true,
                               const string &first_pexpr = string());

  void error_return(ostream &out, int indent_level, int return_flags);
  void error_raise_return(ostream &out, int indent_level, int return_flags,
                          const string &exc_type, const string &message,
                          const string &format_args = "");
  void pack_return_value(ostream &out, int indent_level, FunctionRemap *remap,
                         std::string return_expr);

  void write_make_seq(ostream &out, Object *obj, const std::string &ClassName,
                      const std::string &cClassName, MakeSeq *make_seq);

  void write_class_prototypes(ostream &out) ;
  void write_class_declarations(ostream &out, ostream *out_h, Object *obj);
  void write_class_details(ostream &out, Object *obj);

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
  void write_python_instance(ostream &out, int indent_level, const std::string &return_expr, bool owns_memory, const InterrogateType &itype, bool is_const);
  bool HasAGetClassTypeFunction(CPPType *type);
  int NeedsAStrFunction(const InterrogateType &itype_class);
  int NeedsAReprFunction(const InterrogateType &itype_class);
  bool NeedsARichCompareFunction(const InterrogateType &itype_class);

  void output_quoted(ostream &out, int indent_level, const std::string &str,
                     bool first_line=true);

  // stash the forward declarations for this compile pass..
  std::set<CPPType *> _external_imports;
};

#endif
