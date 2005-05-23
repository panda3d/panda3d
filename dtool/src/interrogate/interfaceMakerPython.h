// Filename: interfaceMakerPython.h
// Created by:  drose (21Sep01)
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

#ifndef INTERFACEMAKERPYTHON_H
#define INTERFACEMAKERPYTHON_H

#include "dtoolbase.h"

#include "interfaceMaker.h"

class FunctionRemap;

////////////////////////////////////////////////////////////////////
//       Class : InterfaceMakerPython
// Description : The base class for InteraceMakerPythonSimple and
//               InterfaceMakerPythonObj, this includes a few
//               functions that both have in common for formatting
//               Python objects.
////////////////////////////////////////////////////////////////////
class InterfaceMakerPython : public InterfaceMaker {
protected:
  InterfaceMakerPython(InterrogateModuleDef *def);

public:
  virtual void write_includes(ostream &out);

protected:
  virtual void test_assert(ostream &out, int indent_level) const;
};

#endif
