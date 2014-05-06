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
class InterfaceMakerPythonNative : public InterfaceMakerPython 
{
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
    WT_numeric_operator,
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
  };

  class SlottedFunctionDef {
  public:
    string _answer_location;
    WrapperType _wrapper_type;
    int _min_version;
  };

  static bool get_slotted_function_def(Object *obj, Function *func, SlottedFunctionDef &def);
  
  void write_prototype_for_name(ostream &out, Function *func, const std::string &name);
  void write_prototype_for(ostream &out, Function *func);
  void write_function_for_name(ostream &out, Object *obj, Function *func, const std::string &name, const std::string &PreProcess, const std::string &ClassName,
                               bool coercion_allowed, bool &coercion_attempted);
  void write_function_for_top(ostream &out, Object *obj, Function *func, const std::string &PreProcess);
  void write_function_instance(ostream &out, Object *obj, Function *func,
                               FunctionRemap *remap, string &expected_params, 
                               int indent_level, bool errors_fatal, 
                               ostream &forwarddecl, const std::string &functionnamestr,
                               bool is_inplace, bool coercion_allowed,
                               bool &coercion_attempted,
                               const string &args_cleanup);
  
  void write_function_forset(ostream &out, Object *obj, Function *func,
                             std::set<FunctionRemap*> &remaps, string &expected_params,
                             int indent_level, ostream &forwarddecl, bool inplace,
                             bool coercion_allowed, bool &coercion_attempted, 
                             const string &args_cleanup);

  void pack_return_value(ostream &out, int indent_level, FunctionRemap *remap,
                         const std::string &return_expr, bool in_place);
  void pack_python_value(ostream &out, int indent_level, FunctionRemap *remap,
                         ParameterRemap *return_type, const std::string &return_expr,
                         const std::string &assign_expr, bool in_place);

  void write_make_seq(ostream &out, Object *obj, const std::string &ClassName,
                      MakeSeq *make_seq);

  void write_class_prototypes(ostream &out) ;
  void write_class_declarations(ostream &out, ostream *out_h, Object *obj);
  void write_class_details(ostream &out, Object *obj);
  
  void do_assert_init(ostream &out, int &indent_level, bool constructor, const string &args_cleanup) const;
public:
  bool is_remap_legal(FunctionRemap &remap);
  bool is_function_legal( Function *func);
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
  void write_python_instance(ostream &out, int indent_level, const std::string &return_expr, const std::string &assign_expr, std::string &owns_memory_flag, const std::string &class_name, CPPType *ctype, bool inplace, const std::string &const_flag);
  string HasAGetKeyFunction(const InterrogateType &itype_class);
  bool HasAGetClassTypeFunction(const InterrogateType &itype_class);
  int NeedsAStrFunction(const InterrogateType &itype_class);
  int NeedsAReprFunction(const InterrogateType &itype_class);
  bool NeedsARichCompareFunction(const InterrogateType &itype_class);

  void output_quoted(ostream &out, int indent_level, const std::string &str);
  
  // stash the forward declarations for this compile pass..
  std::set<std::string>     _external_imports;    
};

#endif
