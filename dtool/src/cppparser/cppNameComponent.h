// Filename: cppNameComponent.h
// Created by:  drose (12Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
