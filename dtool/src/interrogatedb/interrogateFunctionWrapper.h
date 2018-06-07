/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateFunctionWrapper.h
 * @author drose
 * @date 2000-08-06
 */

#ifndef INTERROGATEFUNCTIONWRAPPER_H
#define INTERROGATEFUNCTIONWRAPPER_H

#include "dtoolbase.h"

#include "interrogateComponent.h"

#include <vector>

class IndexRemapper;

/**
 * An internal representation of a callable function.
 */
class EXPCL_INTERROGATEDB InterrogateFunctionWrapper : public InterrogateComponent {
public:
  INLINE InterrogateFunctionWrapper(InterrogateModuleDef *def = nullptr);
  INLINE InterrogateFunctionWrapper(const InterrogateFunctionWrapper &copy);
  INLINE void operator = (const InterrogateFunctionWrapper &copy);

  INLINE FunctionIndex get_function() const;

  INLINE bool is_callable_by_name() const;

  INLINE bool has_return_value() const;
  INLINE TypeIndex get_return_type() const;
  INLINE bool caller_manages_return_value() const;
  INLINE FunctionIndex get_return_value_destructor() const;

  INLINE int number_of_parameters() const;
  INLINE TypeIndex parameter_get_type(int n) const;
  INLINE bool parameter_has_name(int n) const;
  INLINE const std::string &parameter_get_name(int n) const;
  INLINE bool parameter_is_this(int n) const;

  INLINE const std::string &get_unique_name() const;

  INLINE bool has_comment() const;
  INLINE const std::string &get_comment() const;

  void output(std::ostream &out) const;
  void input(std::istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  enum Flags {
    F_caller_manages   = 0x0001,
    F_has_return       = 0x0002,
    F_callable_by_name = 0x0004
  };

  enum ParameterFlags {
    PF_has_name       = 0x0001,
    PF_is_this        = 0x0002,
  };

  int _flags;
  FunctionIndex _function;
  TypeIndex _return_type;
  FunctionIndex _return_value_destructor;
  std::string _unique_name;
  std::string _comment;

public:
  // This nested class must be declared public just so we can declare the
  // external ostream and istream IO operator functions, on the SGI compiler.
  // Arguably a compiler bug, but what can you do.
  class Parameter {
  public:
    void output(std::ostream &out) const;
    void input(std::istream &in);

    int _parameter_flags;
    TypeIndex _type;
    std::string _name;
  };

private:
  typedef std::vector<Parameter> Parameters;
  Parameters _parameters;

  friend class InterrogateBuilder;
  friend class FunctionRemap;
};

INLINE std::ostream &operator << (std::ostream &out, const InterrogateFunctionWrapper &wrapper);
INLINE std::istream &operator >> (std::istream &in, InterrogateFunctionWrapper &wrapper);

INLINE std::ostream &operator << (std::ostream &out, const InterrogateFunctionWrapper::Parameter &p);
INLINE std::istream &operator >> (std::istream &in, InterrogateFunctionWrapper::Parameter &p);

#include "interrogateFunctionWrapper.I"

#endif
