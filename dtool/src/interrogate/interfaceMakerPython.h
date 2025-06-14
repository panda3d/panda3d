/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interfaceMakerPython.h
 * @author drose
 * @date 2001-09-21
 */

#ifndef INTERFACEMAKERPYTHON_H
#define INTERFACEMAKERPYTHON_H

#include "dtoolbase.h"

#include "interfaceMaker.h"

class FunctionRemap;

/**
 * The base class for InteraceMakerPythonSimple and InterfaceMakerPythonObj,
 * this includes a few functions that both have in common for formatting
 * Python objects.
 */
class InterfaceMakerPython : public InterfaceMaker {
protected:
  InterfaceMakerPython(InterrogateModuleDef *def);

public:
  virtual void write_includes(std::ostream &out);

protected:
  virtual void test_assert(std::ostream &out, int indent_level) const;
};

#endif
