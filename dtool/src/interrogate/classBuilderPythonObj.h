// Filename: classBuilderPythonObj.h
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

#ifndef CLASSBUILDERPYTHONOBJ_H
#define CLASSBUILDERPYTHONOBJ_H

#include "dtoolbase.h"
#include "classBuilder.h"

class CPPType;

////////////////////////////////////////////////////////////////////
//       Class : ClassBuilderPythonObj
// Description : A specialization on ClassBuilder that builds
//               actual Python class objects.
////////////////////////////////////////////////////////////////////
class ClassBuilderPythonObj : public ClassBuilder {
public:
  virtual bool set_class(TypeIndex type_index, CPPStructType *struct_type);

  virtual void write_prototype(ostream &out) const;
  virtual void write_code(ostream &out) const;

  static string get_builder_name(CPPType *struct_type);

private:
  string _name;
};

#endif
