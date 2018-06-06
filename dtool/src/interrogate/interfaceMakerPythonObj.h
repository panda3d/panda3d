/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interfaceMakerPythonObj.h
 * @author drose
 * @date 2001-09-19
 */

#ifndef INTERFACEMAKERPYTHONOBJ_H
#define INTERFACEMAKERPYTHONOBJ_H

#include "dtoolbase.h"

#include "interfaceMakerPython.h"
#include "interrogate_interface.h"

#include <map>

class InterrogateType;
class InterrogateFunction;
class FunctionRemap;
class CPPInstance;
class FunctionWriterPtrFromPython;
class FunctionWriterPtrToPython;

/**
 * An InterfaceMaker suitable for generating object-oriented Python code, that
 * can be imported and used directly by Python.
 */
class InterfaceMakerPythonObj : public InterfaceMakerPython {
public:
  InterfaceMakerPythonObj(InterrogateModuleDef *def);
  virtual ~InterfaceMakerPythonObj();

  virtual void write_prototypes(std::ostream &out,std::ostream *out_h);
  virtual void write_functions(std::ostream &out);

  virtual void write_module(std::ostream &out,std::ostream *out_h, InterrogateModuleDef *def);

  virtual bool synthesize_this_parameter();

  static std::string get_builder_name(CPPType *struct_type);

protected:
  virtual std::string get_wrapper_prefix();

private:
  void write_class_wrapper(std::ostream &out, Object *object);
  void write_prototype_for(std::ostream &out, Function *func);
  void write_function_for(std::ostream &out, Function *func);
  void write_function_instance(std::ostream &out, int indent_level, Function *func,
                               FunctionRemap *remap, std::string &expected_params);

  void pack_return_value(std::ostream &out, int indent_level,
                         FunctionRemap *remap, std::string return_expr);

  FunctionWriterPtrFromPython *get_ptr_from_python(CPPType *type);
  FunctionWriterPtrToPython *get_ptr_to_python(CPPType *type);

  typedef std::map<CPPType *, FunctionWriter *> PtrConverter;
  PtrConverter _from_python;
  PtrConverter _to_python;
};

#endif
