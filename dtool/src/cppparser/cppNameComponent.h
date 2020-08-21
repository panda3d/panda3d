/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppNameComponent.h
 * @author drose
 * @date 1999-11-12
 */

#ifndef CPPNAMECOMPONENT_H
#define CPPNAMECOMPONENT_H

#include "dtoolbase.h"
#include "cppBisonDefs.h"

#include <string>

class CPPTemplateParameterList;
class CPPScope;

class CPPNameComponent {
public:
  CPPNameComponent(const std::string &name);
  bool operator == (const CPPNameComponent &other) const;
  bool operator != (const CPPNameComponent &other) const;
  bool operator < (const CPPNameComponent &other) const;

  std::string get_name() const;
  std::string get_name_with_templ(CPPScope *scope = nullptr) const;
  CPPTemplateParameterList *get_templ() const;
  bool empty() const;
  bool has_templ() const;

  bool is_tbd() const;

  void set_name(const std::string &name);
  void append_name(const std::string &name);
  void set_templ(CPPTemplateParameterList *templ);

  void output(std::ostream &out) const;

private:
  std::string _name;
  CPPTemplateParameterList *_templ;
};

inline std::ostream &operator << (std::ostream &out, const CPPNameComponent &name) {
  name.output(out);
  return out;
}

#endif
