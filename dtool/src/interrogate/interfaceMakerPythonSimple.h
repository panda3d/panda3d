/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interfaceMakerPythonSimple.h
 * @author drose
 * @date 2001-10-01
 */

#ifndef INTERFACEMAKERPYTHONSIMPLE_H
#define INTERFACEMAKERPYTHONSIMPLE_H

#include "dtoolbase.h"

#include "interfaceMakerPython.h"
#include "interrogate_interface.h"

class FunctionRemap;

/**
 * An InterfaceMaker for generating simple Python function wrappers around C++
 * code.  This allows the C++ code to be called by Python, but not necessarily
 * in a user-friendly or object-oriented way.
 *
 * You probably want to use InterfaceMakerPythonObj for a full object-oriented
 * solution.  This InterfaceMaker is primarily useful as a stopgap for our old
 * Python-based FFI system.
 */
class InterfaceMakerPythonSimple : public InterfaceMakerPython {
public:
  InterfaceMakerPythonSimple(InterrogateModuleDef *def);
  virtual ~InterfaceMakerPythonSimple();

  virtual void write_prototypes(std::ostream &out,std::ostream *out_h);
  virtual void write_functions(std::ostream &out);

  virtual void write_module(std::ostream &out,std::ostream *out_h, InterrogateModuleDef *def);

  virtual bool synthesize_this_parameter();

protected:
  virtual std::string get_wrapper_prefix();
  virtual std::string get_unique_prefix();

  virtual void
  record_function_wrapper(InterrogateFunction &ifunc,
                          FunctionWrapperIndex wrapper_index);

private:
  void write_prototype_for(std::ostream &out, Function *func);
  void write_function_for(std::ostream &out, Function *func);
  void write_function_instance(std::ostream &out, Function *func,
                               FunctionRemap *remap);

  void pack_return_value(std::ostream &out, int indent_level,
                         FunctionRemap *remap, std::string return_expr);
};

#endif
