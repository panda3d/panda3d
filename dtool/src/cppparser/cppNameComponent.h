// Filename: cppNameComponent.h
// Created by:  drose (12Nov99)
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

#ifndef CPPNAMECOMPONENT_H
#define CPPNAMECOMPONENT_H

#include "dtoolbase.h"

#include <string>

using namespace std;

class CPPTemplateParameterList;
class CPPScope;

class CPPNameComponent {
public:
  CPPNameComponent(const string &name);
  bool operator == (const CPPNameComponent &other) const;
  bool operator != (const CPPNameComponent &other) const;
  bool operator < (const CPPNameComponent &other) const;

  string get_name() const;
  string get_name_with_templ(CPPScope *scope = (CPPScope *)NULL) const;
  CPPTemplateParameterList *get_templ() const;
  bool empty() const;
  bool has_templ() const;

  bool is_tbd() const;

  void set_name(const string &name);
  void append_name(const string &name);
  void set_templ(CPPTemplateParameterList *templ);

  void output(ostream &out) const;

private:
  string _name;
  CPPTemplateParameterList *_templ;
};

inline ostream &operator << (ostream &out, const CPPNameComponent &name) {
  name.output(out);
  return out;
}

#endif
