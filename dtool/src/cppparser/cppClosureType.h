/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppClosureType.h
 * @author rdb
 * @date 2017-01-14
 */

#ifndef CPPCLOSURETYPE_H
#define CPPCLOSURETYPE_H

#include "dtoolbase.h"

#include "cppFunctionType.h"

/**
 * The type of a lambda expression.  This is like a function, but with
 * additional captures defined.
 */
class CPPClosureType : public CPPFunctionType {
public:
  enum CaptureType {
    CT_none,
    CT_by_reference,
    CT_by_value,
  };

  CPPClosureType(CaptureType default_capture = CT_none);
  CPPClosureType(const CPPClosureType &copy);
  void operator = (const CPPClosureType &copy);

  struct Capture {
    std::string _name;
    CaptureType _type;
    CPPExpression *_initializer;
  };
  typedef std::vector<Capture> Captures;
  Captures _captures;

  CaptureType _default_capture;

  void add_capture(std::string name, CaptureType type, CPPExpression *initializer = nullptr);

  virtual bool is_fully_specified() const;

  virtual bool is_default_constructible() const;
  virtual bool is_copy_constructible() const;
  virtual bool is_destructible() const;

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;
  virtual CPPClosureType *as_closure_type();

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;
};

#endif
