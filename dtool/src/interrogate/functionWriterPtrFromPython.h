// Filename: functionWriterPtrFromPython.h
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

#ifndef FUNCTIONWRITERPTRFROMPYTHON_H
#define FUNCTIONWRITERPTRFROMPYTHON_H

#include "functionWriter.h"

class CPPType;

////////////////////////////////////////////////////////////////////
//       Class : FunctionWriterPtrFromPython
// Description : This specialization of FunctionWriter generates a
//               function that converts a PyObject pointer
//               representing a class wrapper object to the
//               corresponding C++ pointer.
////////////////////////////////////////////////////////////////////
class FunctionWriterPtrFromPython : public FunctionWriter {
public:
  FunctionWriterPtrFromPython(CPPType *type);
  virtual ~FunctionWriterPtrFromPython();

  virtual void write_prototype(ostream &out);
  virtual void write_code(ostream &out);
  CPPType *get_type() const;
  CPPType *get_pointer_type() const;

private:
  CPPType *_type;
  CPPType *_pointer_type;
};

#endif
