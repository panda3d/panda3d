// Filename: cppManifest.cxx
// Created by:  drose (22Oct99)
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


#include "cppManifest.h"
#include "cppExpression.h"

#include <ctype.h>

////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::ExpansionNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPManifest::ExpansionNode::
ExpansionNode(int parm_number) :
  _parm_number(parm_number)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::ExpansionNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPManifest::ExpansionNode::
ExpansionNode(const string &str) :
  _parm_number(-1), _str(str)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPManifest::
CPPManifest(const string &args, const CPPFile &file) : _file(file) {
  assert(!args.empty());
  assert(!isspace(args[0]));

  _expr = (CPPExpression *)NULL;
  _vis = V_public;

  // First, identify the manifest name.

  size_t p = 0;
  while (p < args.size() && !isspace(args[p]) && args[p] != '(') {
    p++;
  }

  _name = args.substr(0, p);

  vector_string parameter_names;

  if (args[p] == '(') {
    // Hmm, parameters.
    _has_parameters = true;
    parse_parameters(args, p, parameter_names);
    _num_parameters = parameter_names.size();
    p++;
  } else {
    _has_parameters = false;
    _num_parameters = 0;
  }

  // Now identify the expansion.  Skip whitespace.
  while (p < args.size() && isspace(args[p])) {
    p++;
  }

  save_expansion(args.substr(p), parameter_names);
}


////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPManifest::
~CPPManifest() {
  if (_expr != (CPPExpression *)NULL) {
    delete _expr;
  }
}



////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::expand
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string CPPManifest::
expand(const vector_string &args) const {
  string result;

  Expansion::const_iterator ei;
  for (ei = _expansion.begin(); ei != _expansion.end(); ++ei) {
    if ((*ei)._parm_number >= 0) {
      int i = (*ei)._parm_number;
      if (i < (int)args.size()) {
        result += " " + args[i] + " ";
      } else {
        result += " ";
      }
    }
    if (!(*ei)._str.empty()) {
      result += (*ei)._str;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::determine_type
//       Access: Public
//  Description: Returns the type of the manifest, if it is known,
//               or NULL if the type cannot be determined.
////////////////////////////////////////////////////////////////////
CPPType *CPPManifest::
determine_type() const {
  if (_expr != (CPPExpression *)NULL) {
    return _expr->determine_type();
  }
  return (CPPType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPManifest::
output(ostream &out) const {
  out << _name;

  if (_has_parameters) {
    out << "(";
    if (_num_parameters > 0) {
      out << "$1";
      for (int i = 1; i < _num_parameters; ++i) {
        out << ", $" << i + 1;
      }
    }
    out << ")";
  }

  out << " ";

  Expansion::const_iterator ei;
  for (ei = _expansion.begin(); ei != _expansion.end(); ++ei) {
    if ((*ei)._parm_number >= 0) {
      out << " $" << (*ei)._parm_number + 1 << " ";
    }
    if (!(*ei)._str.empty()) {
      out << (*ei)._str;
    }
  }
}



////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::parse_parameters
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPManifest::
parse_parameters(const string &args, size_t &p,
                 vector_string &parameter_names) {
  assert(p < args.size());
  assert(args[p] == '(');

  p++;
  while (p < args.size() && isspace(args[p])) {
    p++;
  }

  while (p < args.size() && args[p] != ')') {
    // Here's the beginning of a parm.
    size_t q = p;
    while (p < args.size() && !isspace(args[p]) &&
           args[p] != ')' && args[p] != ',') {
      p++;
    }
    parameter_names.push_back(args.substr(q, p - q));

    // Skip whitespace after the parameter name.
    while (p < args.size() && isspace(args[p])) {
      p++;
    }

    if (p < args.size() && args[p] == ',') {
      p++;
      // Skip whitespace after a comma.
      while (p < args.size() && isspace(args[p])) {
        p++;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::save_expansion
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPManifest::
save_expansion(const string &exp, const vector_string &parameter_names) {
  if (parameter_names.empty()) {
    // No parameters; this is an easy case.
    _expansion.push_back(ExpansionNode(exp));
    return;
  }

  // Walk through the expansion string.  For each substring that is an
  // identifier, check it against parameter_names.
  size_t p = 0;
  size_t last = 0;
  while (p < exp.size()) {
    if (isalpha(exp[p]) || exp[p] == '_') {
      // Here's the start of an identifier.  Find the end of it.
      size_t q = p;
      p++;
      while (p < exp.size() && isalnum(exp[p]) || exp[p] == '_') {
        p++;
      }

      string ident = exp.substr(q, p - q);

      // Is this identifier one of our parameters?
      int pnum = -1;
      for (int i = 0; pnum == -1 && i < (int)parameter_names.size(); ++i) {
        if (parameter_names[i] == ident) {
          pnum = i;
        }
      }

      if (pnum != -1) {
        // Yep!
        if (last != q) {
          _expansion.push_back(ExpansionNode(exp.substr(last, q - last)));
        }
        _expansion.push_back(pnum);
        last = p;
      }
    } else {
      p++;
    }
  }

  if (last != p) {
    _expansion.push_back(exp.substr(last, p - last));
  }
}
