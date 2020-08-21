/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateElement.h
 * @author drose
 * @date 2000-08-11
 */

#ifndef INTERROGATEELEMENT_H
#define INTERROGATEELEMENT_H

#include "dtoolbase.h"

#include "interrogateComponent.h"

class IndexRemapper;
class CPPMakeProperty;

/**
 * An internal representation of a data element, like a data member or a
 * global variable.
 */
class EXPCL_INTERROGATEDB InterrogateElement : public InterrogateComponent {
public:
  INLINE InterrogateElement(InterrogateModuleDef *def = nullptr);
  INLINE InterrogateElement(const InterrogateElement &copy);
  INLINE void operator = (const InterrogateElement &copy);

  INLINE bool is_global() const;

  INLINE bool has_scoped_name() const;
  INLINE const std::string &get_scoped_name() const;

  INLINE bool has_comment() const;
  INLINE const std::string &get_comment() const;

  INLINE TypeIndex get_type() const;
  INLINE bool has_getter() const;
  INLINE FunctionIndex get_getter() const;
  INLINE bool has_setter() const;
  INLINE FunctionIndex get_setter() const;
  INLINE bool has_has_function() const;
  INLINE FunctionIndex get_has_function() const;
  INLINE bool has_clear_function() const;
  INLINE FunctionIndex get_clear_function() const;
  INLINE bool has_del_function() const;
  INLINE FunctionIndex get_del_function() const;
  INLINE bool has_insert_function() const;
  INLINE FunctionIndex get_insert_function() const;
  INLINE bool has_getkey_function() const;
  INLINE FunctionIndex get_getkey_function() const;
  INLINE bool is_sequence() const;
  INLINE FunctionIndex get_length_function() const;
  INLINE bool is_mapping() const;

  void output(std::ostream &out) const;
  void input(std::istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  enum Flags {
    F_global          = 0x0001,
    F_has_getter      = 0x0002,
    F_has_setter      = 0x0004,
    F_has_has_function= 0x0008,
    F_has_clear_function= 0x0010,
    F_has_del_function= 0x0020,
    F_sequence        = 0x0040,
    F_mapping         = 0x0080,
    F_has_insert_function= 0x0100,
    F_has_getkey_function= 0x0200,
  };

  int _flags;
  std::string _scoped_name;
  std::string _comment;
  TypeIndex _type;
  FunctionIndex _length_function;
  FunctionIndex _getter;
  FunctionIndex _setter;
  FunctionIndex _has_function;
  FunctionIndex _clear_function;
  FunctionIndex _del_function;
  FunctionIndex _insert_function;
  FunctionIndex _getkey_function;

  CPPMakeProperty *_make_property;

  friend class InterrogateBuilder;
};

INLINE std::ostream &operator << (std::ostream &out, const InterrogateElement &element);
INLINE std::istream &operator >> (std::istream &in, InterrogateElement &element);

#include "interrogateElement.I"

#endif
