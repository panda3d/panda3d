// Filename: interfaceMakerPythonSimple.h
// Created by:  drose (01Oct01)
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

#ifndef INTERFACEMAKERPYTHONSIMPLE_H
#define INTERFACEMAKERPYTHONSIMPLE_H

#include "dtoolbase.h"

#include "interfaceMakerPython.h"
#include "interrogate_interface.h"

class FunctionRemap;

////////////////////////////////////////////////////////////////////
//       Class : InterfaceMakerPythonSimple
// Description : An InterfaceMaker for generating simple Python
//               function wrappers around C++ code.  This allows the
//               C++ code to be called by Python, but not necessarily
//               in a user-friendly or object-oriented way.
//
//               You probably want to use InterfaceMakerPythonObj for
//               a full object-oriented solution.  This InterfaceMaker
//               is primarily useful as a stopgap for our old
//               Python-based FFI system.
////////////////////////////////////////////////////////////////////
class InterfaceMakerPythonSimple : public InterfaceMakerPython {
public:
  InterfaceMakerPythonSimple(InterrogateModuleDef *def);
  virtual ~InterfaceMakerPythonSimple();

  virtual void write_prototypes(ostream &out);
  virtual void write_functions(ostream &out);

  virtual void write_module(ostream &out, InterrogateModuleDef *def);

  virtual bool synthesize_this_parameter();

protected:
  virtual string get_wrapper_prefix();
  virtual string get_unique_prefix();

  virtual void
  record_function_wrapper(InterrogateFunction &ifunc, 
                          FunctionWrapperIndex wrapper_index);

private:
  void write_prototype_for(ostream &out, Function *func);
  void write_function_for(ostream &out, Function *func);
  void write_function_instance(ostream &out, Function *func,
                               FunctionRemap *remap);

  void pack_return_value(ostream &out, int indent_level,
                         FunctionRemap *remap, string return_expr);
};

#endif
