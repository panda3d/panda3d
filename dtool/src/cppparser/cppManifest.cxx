// Filename: cppManifest.cxx
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


#include "cppManifest.h"
#include "cppExpression.h"

#include <ctype.h>

////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::ExpansionNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPManifest::ExpansionNode::
ExpansionNode(int parm_number, bool stringify, bool paste) :
  _parm_number(parm_number), _stringify(stringify), _paste(paste)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::ExpansionNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPManifest::ExpansionNode::
ExpansionNode(const string &str, bool paste) :
  _parm_number(-1), _stringify(false), _paste(paste), _str(str)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPManifest::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPManifest::
CPPManifest(const string &args, const CPPFile &file) :
  _variadic_param(-1),
  _file(file),
  _expr((CPPExpression *)NULL)
{
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
//     Function: CPPManifest::stringify
//       Access: Public, Static
//  Description: This implements the stringification operator, #.
////////////////////////////////////////////////////////////////////
string CPPManifest::
stringify(const string &source) {
  string result("\"");

  enum {
    S_escaped = 0x01,
    S_single_quoted = 0x02,
    S_double_quoted = 0x04,
    S_quoted = 0x06,
  };
  int state = 0;

  string::const_iterator it;
  for (it = source.begin(); it != source.end(); ++it) {
    char c = *it;

    if ((state & S_escaped) == 0) {
      switch (c) {
      case '\\':
        if (state & S_quoted) {
          state |= S_escaped;
          result += '\\';
        }
        break;

      case '\'':
        state ^= S_single_quoted;
        break;

      case '"':
        state ^= S_double_quoted;
        result += '\\';
        break;
      }
    } else {
      if (c == '\\' || c == '"') {
        result += '\\';
      }
      state &= ~S_escaped;
    }

    result += c;
  }

  result += '"';
  return result;
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

      string subst;
      if (i < (int)args.size()) {
        subst = args[i];

        if (i == _variadic_param) {
          for (++i; i < (int)args.size(); ++i) {
            subst += ", " + args[i];
          }
        }
        if ((*ei)._stringify) {
          subst = stringify(subst);
        }
      } else if (i == _variadic_param && (*ei)._paste) {
        // Special case GCC behavior: if __VA_ARGS__ is pasted
        // to a comma and no arguments are passed, the comma
        // is removed.  MSVC does this automatically.  Not sure
        // if we should allow MSVC behavior as well.
        if (!result.empty() && *result.rbegin() == ',') {
          result.resize(result.size() - 1);
        }
      }

      if (!subst.empty()) {
        if (result.empty() || (*ei)._paste) {
          result += subst;
        } else {
          result += ' ';
          result += subst;
        }
      }
    }
    if (!(*ei)._str.empty()) {
      if (result.empty() || (*ei)._paste) {
        result += (*ei)._str;
      } else {
        result += ' ';
        result += (*ei)._str;
      }
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
      if (_variadic_param == 0) {
        out << "...";
      } else {
        out << "$1";
      }

      for (int i = 1; i < _num_parameters; ++i) {
        if (_variadic_param == i) {
          out << ", ...";
        } else {
          out << ", $" << i + 1;
        }
      }
    }
    out << ")";
  }

  Expansion::const_iterator ei;
  for (ei = _expansion.begin(); ei != _expansion.end(); ++ei) {
    if ((*ei)._paste) {
      out << " ## ";
    } else {
      out << " ";
    }

    if ((*ei)._parm_number >= 0) {
      if ((*ei)._stringify) {
        out << "#";
      }
      if ((*ei)._parm_number == _variadic_param) {
        out << "__VA_ARGS__";
      } else {
        out << "$" << (*ei)._parm_number + 1;
      }
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

    // Check if it's a variadic parameter by checking if it ends
    // with "...".  This picks up both C99-style variadic macros
    // and GCC-style variadic macros.
    if (p - q >= 3 && args.compare(p - 3, 3, "...") == 0) {
      _variadic_param = parameter_names.size();
      parameter_names.push_back(args.substr(q, p - q - 3));
    } else {
      parameter_names.push_back(args.substr(q, p - q));
    }

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
  // Walk through the expansion string.  For each substring that is an
  // identifier, check it against parameter_names.
  size_t p = 0;
  size_t last = 0;
  bool stringify = false;
  bool paste = false;
  while (p < exp.size()) {
    if (isalpha(exp[p]) || exp[p] == '_') {
      // Here's the start of an identifier.  Find the end of it.
      size_t q = p;
      p++;
      while (p < exp.size() && (isalnum(exp[p]) || exp[p] == '_')) {
        p++;
      }

      string ident = exp.substr(q, p - q);

      // Is this identifier one of our parameters?
      int pnum = -1;
      bool va_args = false;

      if (ident == "__VA_ARGS__") {
        va_args = true;
        // C99-style variadics, ie. #define macro(...) __VA_ARGS__
        pnum = _variadic_param;

      } else {
        for (int i = 0; pnum == -1 && i < (int)parameter_names.size(); ++i) {
          const string &pname = parameter_names[i];
          if (pname == ident) {
            pnum = i;
          }
        }
      }

      if (pnum != -1) {
        // Yep!
        if (last != q) {
          _expansion.push_back(ExpansionNode(exp.substr(last, q - last), paste));
          paste = false;
        }
        _expansion.push_back(ExpansionNode(pnum, stringify, paste));
        stringify = false;
        paste = false;
        last = p;
      }
    } else if (exp[p] == '#') {
      // This may be a stringification operator.
      if (last != p) {
        _expansion.push_back(ExpansionNode(exp.substr(last, p - last), paste));
        paste = false;
      }

      ++p;

      if (p < exp.size() && exp[p] == '#') {
        // Woah, this is a token-pasting operator.
        paste = true;
        ++p;
      } else {
        // Mark that the next argument should be stringified.
        stringify = true;
      }
      last = p;

    } else if (isspace(exp[p])) {
      if (last != p) {
        _expansion.push_back(ExpansionNode(exp.substr(last, p - last), paste));
        paste = false;
      }

      ++p;
      last = p;

    } else {
      ++p;
    }
  }

  if (last != p) {
    _expansion.push_back(ExpansionNode(exp.substr(last, p - last), paste));
  }
}
