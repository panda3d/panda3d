// Filename: classBuilder.h
// Created by:  drose (17Sep01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CLASSBUILDER_H
#define CLASSBUILDER_H

#include "dtoolbase.h"
#include "interrogate_interface.h"

class CPPScope;
class CPPStructType;
class InterrogateType;

////////////////////////////////////////////////////////////////////
//       Class : ClassBuilder
// Description : Similar to WrapperBuilder, this contains all the
//               information needed to generate whatever source code
//               might be associated with a complete class definition
//               (as opposed to the source code associated with each
//               function).
//
//               This is an abstract class, and is implemented
//               separately for each kind of generated output.  Most
//               kinds do not need to write anything special for the
//               class definition, so do not implement this class.  At
//               the present, only ClassBuilderPythonObj is necessary.
////////////////////////////////////////////////////////////////////
class ClassBuilder {
public:
  ClassBuilder();
  virtual ~ClassBuilder();

  virtual bool set_class(TypeIndex type_index, CPPStructType *struct_type);

  virtual void write_prototype(ostream &out) const=0;
  virtual void write_code(ostream &out) const=0;

protected:
  CPPStructType *_struct_type;
  TypeIndex _type_index;
};

#endif
