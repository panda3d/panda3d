/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateComponent.h
 * @author drose
 * @date 2000-08-08
 */

#ifndef INTERROGATECOMPONENT_H
#define INTERROGATECOMPONENT_H

#include "dtoolbase.h"

#include "interrogate_interface.h"
#include "interrogate_request.h"

#include <vector>

class IndexRemapper;

/**
 * The base class for things that are part of the interrogate database.  This
 * includes types, functions, and function wrappers.
 */
class EXPCL_INTERROGATEDB InterrogateComponent {
public:
  INLINE InterrogateComponent(InterrogateModuleDef *def = nullptr);
  INLINE InterrogateComponent(const InterrogateComponent &copy);
  INLINE void operator = (const InterrogateComponent &copy);

  INLINE bool has_library_name() const;
  INLINE const char *get_library_name() const;

  INLINE bool has_module_name() const;
  INLINE const char *get_module_name() const;

  INLINE bool has_name() const;
  INLINE const std::string &get_name() const;

  INLINE int get_num_alt_names() const;
  INLINE const std::string &get_alt_name(int n) const;

  void output(std::ostream &out) const;
  void input(std::istream &in);

protected:
  static std::string _empty_string;

private:
  InterrogateModuleDef *_def;
  std::string _name;

  typedef std::vector<std::string> Strings;
  Strings _alt_names;

  friend class InterrogateBuilder;
  friend class FunctionRemap;
};

#include "interrogateComponent.I"

#endif
