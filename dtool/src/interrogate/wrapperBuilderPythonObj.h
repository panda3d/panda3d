// Filename: wrapperBuilderPythonObj.h
// Created by:  drose (11Sep01)
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

#ifndef WRAPPERBUILDERPYTHONOBJ_H
#define WRAPPERBUILDERPYTHONOBJ_H

#include "dtoolbase.h"
#include "wrapperBuilder.h"

////////////////////////////////////////////////////////////////////
//       Class : WrapperBuilderPythonObj
// Description : A specialization on WrapperBuilder that builds
//               sophisticated Python wrapper functions that get
//               assembled into Python methods, and make the C++
//               objects appeart directly as Python objects.
////////////////////////////////////////////////////////////////////
class WrapperBuilderPythonObj : public WrapperBuilder {
public:
  WrapperBuilderPythonObj();

  virtual void get_function_writers(FunctionWriters &writers);

  virtual void
  write_prototype(ostream &out, const string &wrapper_name) const;

  virtual void
  write_wrapper(ostream &out, const string &wrapper_name) const;

  virtual string
  get_wrapper_name(const string &library_hash_name) const;

  virtual bool supports_atomic_strings() const;
  virtual bool synthesize_this_parameter() const;
  virtual CallingConvention get_calling_convention() const;

protected:
  void test_assert(ostream &out, int indent_level) const;
  void pack_return_value(int def_index, ostream &out, string return_expr) const;
};

#endif
