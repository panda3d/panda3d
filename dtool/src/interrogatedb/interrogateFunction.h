// Filename: interrogateFunction.h
// Created by:  drose (01Aug00)
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

#ifndef INTERROGATEFUNCTION_H
#define INTERROGATEFUNCTION_H

#include "dtoolbase.h"

#include "interrogateComponent.h"

#include <vector>
#include <map>

class IndexRemapper;
class CPPInstance;

////////////////////////////////////////////////////////////////////
//       Class : InterrogateFunction
// Description : An internal representation of a function.
////////////////////////////////////////////////////////////////////
class EXPCL_INTERROGATEDB InterrogateFunction : public InterrogateComponent {
public:
  InterrogateFunction(InterrogateModuleDef *def = NULL);
  InterrogateFunction(const InterrogateFunction &copy);
  void operator = (const InterrogateFunction &copy);

  INLINE bool is_global() const;
  INLINE bool is_virtual() const;
  INLINE bool is_method() const;
  INLINE bool is_unary_op() const;
  INLINE bool is_operator_typecast() const;
  INLINE TypeIndex get_class() const;

  INLINE bool has_scoped_name() const;
  INLINE const string &get_scoped_name() const;

  INLINE bool has_comment() const;
  INLINE const string &get_comment() const;

  INLINE bool has_prototype() const;
  INLINE const string &get_prototype() const;

  INLINE int number_of_c_wrappers() const;
  INLINE FunctionWrapperIndex get_c_wrapper(int n) const;

  INLINE int number_of_python_wrappers() const;
  INLINE FunctionWrapperIndex get_python_wrapper(int n) const;

  void output(ostream &out) const;
  void input(istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  enum Flags {
    F_global          = 0x0001,
    F_virtual         = 0x0002,
    F_method          = 0x0004,
    F_typecast        = 0x0008,
    F_getter          = 0x0010,
    F_setter          = 0x0020,
    F_unary_op        = 0x0040,
    F_operator_typecast = 0x0080,
  };

  int _flags;
  string _scoped_name;
  string _comment;
  string _prototype;
  TypeIndex _class;

  typedef vector<FunctionWrapperIndex> Wrappers;
  Wrappers _c_wrappers;
  Wrappers _python_wrappers;

public:
  // The rest of the members in this class aren't part of the public
  // interface to interrogate, but are used internally as the
  // interrogate database is built.  They are valid only during the
  // session of interrogate that generates the database, and will not
  // be filled in when the database is reloaded from disk.

  // This must be a pointer, rather than a concrete map, so we don't
  // risk trying to create a map in one DLL and access it in another.
  // Silly Windows.
  typedef map<string, CPPInstance *> Instances;
  Instances *_instances;
  string _expression;

  friend class InterrogateBuilder;
  friend class InterfaceMakerC;
  friend class InterfaceMakerPythonSimple;
  friend class InterfaceMakerPythonNative;
  friend class FunctionRemap;
};

INLINE ostream &operator << (ostream &out, const InterrogateFunction &function);
INLINE istream &operator >> (istream &in, InterrogateFunction &function);

#include "interrogateFunction.I"

#endif
