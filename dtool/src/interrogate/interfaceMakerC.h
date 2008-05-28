// Filename: interfaceMakerC.h
// Created by:  drose (25Sep01)
//
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

#ifndef INTERFACEMAKERC_H
#define INTERFACEMAKERC_H

#include "dtoolbase.h"

#include "interfaceMaker.h"
#include "interrogate_interface.h"

class FunctionRemap;

////////////////////////////////////////////////////////////////////
//       Class : InterfaceMakerC
// Description : An InteraceMaker suitable for generating
//               a series of C-calling-convention functions for
//               Panda class objects.
////////////////////////////////////////////////////////////////////
class InterfaceMakerC : public InterfaceMaker {
public:
  InterfaceMakerC(InterrogateModuleDef *def);
  virtual ~InterfaceMakerC();

  virtual void write_prototypes(ostream &out,ostream *out_h);
  virtual void write_functions(ostream &out);

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
  void write_function_header(ostream &out, Function *func,
                             FunctionRemap *remap, bool newline);
};

#endif
