// Filename: functionWriterPtrToPython.h
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

#ifndef FUNCTIONWRITERPTRTOPYTHON_H
#define FUNCTIONWRITERPTRTOPYTHON_H

#include "functionWriter.h"

class CPPType;

////////////////////////////////////////////////////////////////////
//       Class : FunctionWriterPtrToPython
// Description : This specialization of FunctionWriter generates a
//               function that generates a PyObject class wrapper
//               object around the corresponding C++ pointer.
////////////////////////////////////////////////////////////////////
class FunctionWriterPtrToPython : public FunctionWriter {
public:
  FunctionWriterPtrToPython(CPPType *type);
  virtual ~FunctionWriterPtrToPython();

  virtual void write_prototype(ostream &out);
  virtual void write_code(ostream &out);
  CPPType *get_pointer_type() const;

private:
  CPPType *_type;
  CPPType *_pointer_type;
};

#endif
