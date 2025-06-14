/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file functionWriterPtrToPython.h
 * @author drose
 * @date 2001-09-14
 */

#ifndef FUNCTIONWRITERPTRTOPYTHON_H
#define FUNCTIONWRITERPTRTOPYTHON_H

#include "functionWriter.h"

class CPPType;

/**
 * This specialization of FunctionWriter generates a function that generates a
 * PyObject class wrapper object around the corresponding C++ pointer.
 */
class FunctionWriterPtrToPython : public FunctionWriter {
public:
  FunctionWriterPtrToPython(CPPType *type);
  virtual ~FunctionWriterPtrToPython();

  virtual void write_prototype(std::ostream &out);
  virtual void write_code(std::ostream &out);
  CPPType *get_pointer_type() const;

private:
  CPPType *_type;
  CPPType *_pointer_type;
};

#endif
