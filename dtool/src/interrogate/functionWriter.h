// Filename: functionWriter.h
// Created by:  drose (14Sep01)
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

#ifndef FUNCTIONWRITER_H
#define FUNCTIONWRITER_H

#include "dtoolbase.h"

////////////////////////////////////////////////////////////////////
//       Class : FunctionWriter
// Description : This is an abstract class that can be used by the
//               various InterfaceMakers to indicate a generic helper
//               function or variable that needs to be written to the
//               generated source file.
////////////////////////////////////////////////////////////////////
class FunctionWriter {
public:
  FunctionWriter();
  virtual ~FunctionWriter();

  const string &get_name() const;
  virtual int compare_to(const FunctionWriter &other) const;

  virtual void write_prototype(ostream &out);
  virtual void write_code(ostream &out);

protected:
  string _name;
};

#endif
