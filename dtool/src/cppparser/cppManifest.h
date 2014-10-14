// Filename: cppManifest.h
// Created by:  drose (22Oct99)
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

#ifndef CPPMANIFEST_H
#define CPPMANIFEST_H

#include "dtoolbase.h"

#include "cppFile.h"
#include "cppVisibility.h"

#include "vector_string.h"

class CPPExpression;
class CPPType;

///////////////////////////////////////////////////////////////////
//       Class : CPPManifest
// Description :
////////////////////////////////////////////////////////////////////
class CPPManifest {
public:
  CPPManifest(const string &args, const CPPFile &file = CPPFile());
  ~CPPManifest();

  static string stringify(const string &source);
  string expand(const vector_string &args = vector_string()) const;

  CPPType *determine_type() const;

  void output(ostream &out) const;

  string _name;
  bool _has_parameters;
  int _num_parameters;
  int _variadic_param;
  CPPFile _file;
  CPPExpression *_expr;

  // Manifests don't have a visibility in the normal sense.  Normally
  // this will be V_public.  But a manifest that is defined between
  // __begin_publish and __end_publish will have a visibility of
  // V_published.
  CPPVisibility _vis;

private:
  void parse_parameters(const string &args, size_t &p,
                        vector_string &parameter_names);
  void save_expansion(const string &exp,
                      const vector_string &parameter_names);

  class ExpansionNode {
  public:
    ExpansionNode(int parm_number, bool stringify, bool paste);
    ExpansionNode(const string &str, bool paste = false);
    int _parm_number;
    bool _stringify;
    bool _paste;
    string _str;
  };
  typedef vector<ExpansionNode> Expansion;
  Expansion _expansion;
};

inline ostream &operator << (ostream &out, const CPPManifest &manifest) {
  manifest.output(out);
  return out;
}

#endif
