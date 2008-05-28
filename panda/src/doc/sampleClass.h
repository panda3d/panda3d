// Filename: sampleClass.h
// Created by:  drose (10Jun00)
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

#ifndef SAMPLECLASS_H
#define SAMPLECLASS_H

// This file shows some sample code that illustrates our general
// naming and style conventions for Panda coding.  Note that there is
// generally one .h file per class, with the .h file named after the
// class but the first letter lowercase.

#include "pandabase.h"

#include "localHeaderFile.h"
#include "anotherLocalHeaderFile.h"

#include "typedObject.h"
#include "anotherPandaHeaderFile.h"

#include <systemHeaderFile.h>

////////////////////////////////////////////////////////////////////
//       Class : SampleClass
// Description : A basic description of the function and purpose of
//               SampleClass.  Note that class names are generally
//               mixed case, no underscore, beginning with a capital
//               letter.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SampleClass : public TypedObject {
public:
  enum NestedEnum {
    NE_case_one,
    NE_case_two,
  };

  class EXPCL_PANDA NestedClass {
  public:
    int _data_member;
  };

  SampleClass();
  INLINE SampleClass(const SampleClass &copy);
  INLINE ~SampleClass();

  // Note that inline function bodies are generally not given here in
  // the .h file--they're defined in the associated .I file.

  // Method names are generally lower case, with underscores
  // separating words.  Accessors are generally of the form set_*()
  // and get_*().  Respect the const convention for methods which
  // should be const.

  INLINE void set_flag(int flag);
  INLINE int get_flag() const;

  int public_method();

protected:
  bool protected_method();

private:
  void private_method();


public:
  // Data members, whether private or public, are generally lower
  // case, with underscores separating words, and beginning with a
  // leading underscore.

  bool _public_data_member;

private:

  NestedEnumType _private_data_member;
  int _flag;


  // The TypeHandle stuff, below, need be present only for classes
  // that inherit from TypedObject.  Classes that do not inherit from
  // TypedObject may optionally define just the non-virtual methods
  // below: get_class_type(), init_type().
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "SampleClass",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sampleClass.I"

#endif
