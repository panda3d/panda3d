/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interfaceMaker.h
 * @author drose
 * @date 2001-09-19
 */

#ifndef INTERFACEMAKER_H
#define INTERFACEMAKER_H

#include "dtoolbase.h"

#include "cppMakeSeq.h"

#include "interrogate_interface.h"
#include "interrogate_request.h"
#include "functionWriters.h"

#include <vector>
#include <map>

class FunctionRemap;
class ParameterRemap;
class CPPType;
class CPPInstance;
class InterrogateBuilder;
class InterrogateElement;
class InterrogateFunction;
class InterrogateMakeSeq;
class InterrogateType;

/**
 * This is an abstract base class that defines how to generate code that can
 * be called from an external language (like Python or Squeak) and that can
 * call into Panda.
 *
 * The specializations of this class like InterfaceMakerPython and
 * InterfaceMakerC will generate the actual wrappers for the various language
 * calling conventions.
 */
class InterfaceMaker {
public:
  InterfaceMaker(InterrogateModuleDef *def);
  virtual ~InterfaceMaker();

  virtual void generate_wrappers();

  virtual void write_includes(std::ostream &out);
  virtual void write_prototypes(std::ostream &out, std::ostream *out_h);
  virtual void write_functions(std::ostream &out);
  virtual void write_module_support(std::ostream &out, std::ostream *out_h, InterrogateModuleDef *def) {};

  virtual void write_module(std::ostream &out, std::ostream *out_h, InterrogateModuleDef *def);

  virtual ParameterRemap *remap_parameter(CPPType *struct_type, CPPType *param_type);

  virtual bool synthesize_this_parameter();
  virtual bool separate_overloading();
  virtual bool wrap_global_functions();

  void get_function_remaps(std::vector<FunctionRemap *> &remaps);

  static std::ostream &indent(std::ostream &out, int indent_level);

public:
  // This contains information about the number of arguments that the wrapping
  // function should take.
  enum ArgsType {
    // This is deliberately engineered such that these values can be OR'ed
    // together to produce another valid enum value.
    AT_unknown      = 0x00,

    // The method or function takes no arguments.
    AT_no_args      = 0x01,

    // There is only a single argument.
    AT_single_arg   = 0x02,

    // The method takes a variable number of arguments.
    AT_varargs      = 0x03,

    // The method may take keyword arguments, if appropriate in the scripting
    // language.  Implies AT_varargs.
    AT_keyword_args = 0x07,
  };

  class Function {
  public:
    Function(const std::string &name,
             const InterrogateType &itype,
             const InterrogateFunction &ifunc);
    ~Function();

    std::string _name;
    const InterrogateType &_itype;
    const InterrogateFunction &_ifunc;
    typedef std::vector<FunctionRemap *> Remaps;
    Remaps _remaps;
    bool _has_this;
    int _flags;
    ArgsType _args_type;
  };
  typedef std::map<FunctionIndex, Function *> FunctionsByIndex;
  typedef std::vector<Function *> Functions;
  FunctionsByIndex _functions;

  class MakeSeq {
  public:
    MakeSeq(const std::string &name, const InterrogateMakeSeq &imake_seq);

    const InterrogateMakeSeq &_imake_seq;
    std::string _name;
    Function *_length_getter;
    Function *_element_getter;
  };
  typedef std::vector<MakeSeq *> MakeSeqs;

  class Property {
  public:
    Property(const InterrogateElement &ielement);

    const InterrogateElement &_ielement;
    std::vector<FunctionRemap *> _getter_remaps;
    std::vector<FunctionRemap *> _setter_remaps;
    Function *_length_function;
    Function *_has_function;
    Function *_clear_function;
    Function *_deleter;
    Function *_inserter;
    Function *_getkey_function;
    bool _has_this;
  };
  typedef std::vector<Property *> Properties;

  class Object {
  public:
    Object(const InterrogateType &itype);
    ~Object();

    void check_protocols();
    bool is_static_method(const std::string &name);

    const InterrogateType &_itype;
    Functions _constructors;
    Functions _methods;
    MakeSeqs _make_seqs;
    Properties _properties;

    enum ProtocolTypes {
      PT_sequence         = 0x0001,
      PT_mapping          = 0x0002,
      PT_make_copy        = 0x0004,
      PT_copy_constructor = 0x0008,
      PT_iter             = 0x0010,
      PT_python_gc        = 0x0020,
    };
    int _protocol_types;
  };
  typedef std::map<TypeIndex, Object *> Objects;
  Objects _objects;

  typedef std::map<std::string, FunctionRemap *> WrappersByHash;
  WrappersByHash _wrappers_by_hash;

  virtual FunctionRemap *
  make_function_remap(const InterrogateType &itype,
                      const InterrogateFunction &ifunc,
                      CPPInstance *cppfunc, int num_default_parameters);

  virtual std::string
  get_wrapper_name(const InterrogateType &itype,
                   const InterrogateFunction &ifunc,
                   FunctionIndex func_index);
  virtual std::string get_wrapper_prefix();
  virtual std::string get_unique_prefix();

  Function *
  record_function(const InterrogateType &itype, FunctionIndex func_index);

  virtual void
  record_function_wrapper(InterrogateFunction &ifunc,
                          FunctionWrapperIndex wrapper_index);

  virtual Object *record_object(TypeIndex type_index);

  void hash_function_signature(FunctionRemap *remap);


  std::string
  manage_return_value(std::ostream &out, int indent_level,
                      FunctionRemap *remap, const std::string &return_expr) const;

  void
  delete_return_value(std::ostream &out, int indent_level,
                      FunctionRemap *remap, const std::string &return_expr) const;

  void output_ref(std::ostream &out, int indent_level, FunctionRemap *remap,
                  const std::string &varname) const;
  void output_unref(std::ostream &out, int indent_level, FunctionRemap *remap,
                    const std::string &varname) const;
  void write_spam_message(std::ostream &out, FunctionRemap *remap) const;

protected:
  InterrogateModuleDef *_def;

  FunctionWriters _function_writers;
};

#endif
