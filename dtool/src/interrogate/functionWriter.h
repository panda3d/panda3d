// Filename: functionWriter.h
// Created by:  drose (14Sep01)
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
