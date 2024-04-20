/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppManifest.cxx
 * @author drose
 * @date 1999-10-22
 */

#include "cppManifest.h"
#include "cppExpression.h"
#include "cppPreprocessor.h"

#include <ctype.h>

using std::string;

/**
 *
 */
CPPManifest::ExpansionNode::
ExpansionNode(int parm_number, bool stringify, bool paste) :
  _parm_number(parm_number),
  _expand(!stringify && !paste),
  _stringify(stringify),
  _paste(paste),
  _optional(false)
{
}

/**
 *
 */
CPPManifest::ExpansionNode::
ExpansionNode(const string &str, bool paste) :
  _parm_number(-1),
  _expand(!paste),
  _stringify(false),
  _paste(paste),
  _optional(false),
  _str(str)
{
}

/**
 *
 */
CPPManifest::ExpansionNode::
ExpansionNode(Expansion nested, bool stringify, bool paste, bool optional) :
  _parm_number(-1),
  _expand(!stringify && !paste),
  _stringify(stringify),
  _paste(paste),
  _optional(optional),
  _nested(std::move(nested))
{
}

/**
 *
 */
bool CPPManifest::ExpansionNode::
operator ==(const ExpansionNode &other) const {
  return _parm_number == other._parm_number
      && _expand == other._expand
      && _stringify == other._stringify
      && _paste == other._paste
      && _optional == other._optional
      && _str == other._str
      && _nested == other._nested;
}

/**
 * Creates a manifest from a preprocessor definition.
 */
CPPManifest::
CPPManifest(const CPPPreprocessor &parser, const string &args, const cppyyltype &loc) :
  _parser(parser),
  _variadic_param(-1),
  _loc(loc),
  _expr(nullptr),
  _vis(V_public)
{
  assert(!args.empty());
  assert(!isspace(args[0]));

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

  save_expansion(_expansion, args.substr(p), parameter_names);
}

/**
 * Creates a custom manifest definition, for example as specified from a
 * command-line -D option.
 */
CPPManifest::
CPPManifest(const CPPPreprocessor &parser, const string &macro, const string &definition) :
  _parser(parser),
  _variadic_param(-1),
  _expr(nullptr),
  _vis(V_public)
{
  _loc.first_line = 0;
  _loc.first_column = 0;
  _loc.last_line = 0;
  _loc.last_column = 0;

  assert(!macro.empty());
  assert(!isspace(macro[0]));

  // First, identify the manifest name.
  size_t p = 0;
  while (p < macro.size() && !isspace(macro[p]) && macro[p] != '(') {
    p++;
  }

  _name = macro.substr(0, p);

  vector_string parameter_names;

  if (macro[p] == '(') {
    // Hmm, parameters.
    _has_parameters = true;
    parse_parameters(macro, p, parameter_names);
    _num_parameters = parameter_names.size();

    p++;
  } else {
    _has_parameters = false;
    _num_parameters = 0;
  }

  save_expansion(_expansion, definition, parameter_names);
}

/**
 *
 */
CPPManifest::
~CPPManifest() {
  delete _expr;
}

/**
 * This implements the stringification operator, #.
 */
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

/**
 *
 */
void CPPManifest::
extract_args(vector_string &args, const string &expr, size_t &p) const {
  // Skip whitespace till paren.
  while (p < expr.size() && isspace(expr[p])) {
    p++;
  }
  if (p >= expr.size() || expr[p] != '(') {
    // No paren, so we have only one arg.
    size_t q = p;
    while (p < expr.size() && (isalnum(expr[p]) || expr[p] == '_')) {
      p++;
    }
    args.push_back(expr.substr(q, p - q));
  }
  else if (expr[p] == '"' || expr[p] == '\'') {
    // Quoted string or character.
    int quote_mark = expr[p];
    p++;
    while (p < expr.size() && expr[p] != quote_mark && expr[p] != '\n') {
      if (expr[p] == '\\') {
        p++;
      }
      if (p < expr.size()) {
        p++;
      }
    }
    p++;
  }
  else {
    // Skip paren.
    p++;
    int paren_level = 1;
    size_t q = p;
    while (p < expr.size()) {
      if (expr[p] == ',' && paren_level == 1) {
        // Back up to strip any trailing whitespace.
        size_t r = p;
        while (r > q && isspace(expr[r - 1])) {
          --r;
        }
        args.push_back(expr.substr(q, r - q));
        q = p+1;
      }
      else if (expr[p] == '"' || expr[p] == '\'') {
        // Quoted string or character.
        int quote_mark = expr[p];
        p++;
        while (p < expr.size() && expr[p] != quote_mark && expr[p] != '\n') {
          if (expr[p] == '\\') {
            p++;
          }
          if (p < expr.size()) {
            p++;
          }
        }
      }
      else if (expr[p] == '(') {
        ++paren_level;
      }
      else if (expr[p] == ')') {
        --paren_level;
        if (paren_level == 0) {
          break;
        }
      }
      else if (isspace(expr[p])) {
        // Skip whitespace at the beginning.
        if (q == p) {
          q++;
        }
      }
      p++;
    }
    {
      // Back up to strip any trailing whitespace.
      size_t r = p;
      while (r > q && isspace(expr[r - 1])) {
        --r;
      }
      if (!args.empty() || r > q) {
        args.push_back(expr.substr(q, r - q));
      }
    }

    if (p < expr.size() && expr[p] == ')') {
      p++;
    }
  }

  if ((int)args.size() < _num_parameters) {
    _parser.warning("Not enough arguments for manifest " + _name);
  }
  else if (_variadic_param < 0 && (int)args.size() > _num_parameters) {
    _parser.warning("Too many arguments for manifest " + _name);
  }
}

/**
 *
 */
string CPPManifest::
expand(const vector_string &args, bool expand_undefined, const Ignores &ignores) const {
  return r_expand(_expansion, args, expand_undefined, ignores);
}

/**
 * Returns the type of the manifest, if it is known, or NULL if the type
 * cannot be determined.
 */
CPPType *CPPManifest::
determine_type() const {
  if (_expr != nullptr) {
    return _expr->determine_type();
  }
  return nullptr;
}

/**
 * Returns true if the macro definitions are equal.
 */
bool CPPManifest::
is_equal(const CPPManifest *other) const {
  if (this == other) {
    return true;
  }
  if (_name != other->_name) {
    return false;
  }
  if (_has_parameters != other->_has_parameters) {
    return false;
  }
  if (_num_parameters != other->_num_parameters) {
    return false;
  }
  if (_variadic_param != other->_variadic_param) {
    return false;
  }
  if (_expansion != other->_expansion) {
    return false;
  }
  return true;
}

/**
 *
 */
void CPPManifest::
output(std::ostream &out) const {
  out << _name;

  if (_has_parameters) {
    out << "(";
    if (_num_parameters > 0) {
      if (_variadic_param == 0) {
        out << "...";
      } else {
        out << "$1";
      }

      for (size_t i = 1; i < _num_parameters; ++i) {
        if (_variadic_param == (int)i) {
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

    if ((*ei)._stringify) {
      out << "#";
    }
    if ((*ei)._parm_number >= 0) {
      if ((*ei)._parm_number == _variadic_param) {
        out << "__VA_ARGS__";
      } else {
        out << "$" << (*ei)._parm_number + 1;
      }
    }
    if ((*ei)._optional) {
      out << "__VA_OPT__(";
    }
    if (!(*ei)._str.empty()) {
      out << (*ei)._str;
    }
    if ((*ei)._optional) {
      out << ")";
    }
  }
}

/**
 *
 */
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

    // Check if it's a variadic parameter by checking if it ends with "...".
    // This picks up both C99-style variadic macros and GCC-style variadic
    // macros.
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

/**
 *
 */
void CPPManifest::
save_expansion(Expansion &expansion, const string &exp, const vector_string &parameter_names) {
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

      if (ident == "__VA_ARGS__") {
        // C99-style variadics, ie.  #define macro(...) __VA_ARGS__
        pnum = _variadic_param;

      } else if (ident == "__VA_OPT__") {
        // Optional expansion, only expands if __VA_ARGS__ is non-empty
        while (p < exp.size() && isspace(exp[p])) {
          ++p;
        }
        if (p < exp.size() && exp[p] == '(') {
          int start = ++p;
          int nesting = 1;
          while (p < exp.size() && nesting > 0) {
            if (exp[p] == '(') {
              ++nesting;
            }
            else if (exp[p] == ')') {
              --nesting;
            }
            ++p;
          }

          if (last != q) {
            expansion.push_back(ExpansionNode(exp.substr(last, q - last), paste));
            paste = false;
          }

          // Store this as a nested expansion, because the whole thing may be
          // stringified as a whole.
          Expansion nested;
          save_expansion(nested, exp.substr(start, p - 1 - start), parameter_names);
          expansion.push_back(ExpansionNode(std::move(nested), stringify, paste, true));
          stringify = false;
          paste = false;
          last = p;
          continue;
        }

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
          expansion.push_back(ExpansionNode(exp.substr(last, q - last), paste));
          paste = false;
        }
        expansion.push_back(ExpansionNode(pnum, stringify, paste));
        stringify = false;
        paste = false;
        last = p;
      }
    } else if (exp[p] == '#') {
      // This may be a stringification operator.
      if (last != p) {
        expansion.push_back(ExpansionNode(exp.substr(last, p - last), paste));
        paste = false;
      }

      ++p;

      if (p < exp.size() && exp[p] == '#') {
        // Woah, this is a token-pasting operator.
        paste = true;
        if (!expansion.empty()) {
          // The previous expansion shouldn't be expanded.
          expansion.back()._expand = false;
        }
        ++p;
      } else {
        // Mark that the next argument should be stringified.
        stringify = true;
      }
      last = p;

    } else if (isspace(exp[p])) {
      if (last != p) {
        expansion.push_back(ExpansionNode(exp.substr(last, p - last), paste));
        paste = false;
      }

      ++p;
      last = p;

    } else {
      ++p;
    }
  }

  if (last != p) {
    expansion.push_back(ExpansionNode(exp.substr(last, p - last), paste));
  }
}

/**
 *
 */
string CPPManifest::
r_expand(const Expansion &expansion, const vector_string &args,
         bool expand_undefined, const Ignores &ignores) const {
  std::string result;

  for (const ExpansionNode &node : expansion) {
    if (node._parm_number >= 0) {
      int i = node._parm_number;

      string subst;
      if (i < (int)args.size()) {
        subst = args[i];

        if (i == _variadic_param) {
          for (++i; i < (int)args.size(); ++i) {
            subst += ", " + args[i];
          }
        }
        if (node._stringify) {
          subst = stringify(subst);
        }
      }
      else if (i == _variadic_param && node._paste) {
        // Special case GCC behavior: if __VA_ARGS__ is pasted to a comma and
        // no arguments are passed, the comma is removed.  MSVC does this
        // automatically.  Not sure if we should allow MSVC behavior as well.
        if (!result.empty() && *result.rbegin() == ',') {
          result.resize(result.size() - 1);
        }
      }

      if (node._expand) {
        _parser.expand_manifests(subst, expand_undefined, ignores);
      }

      if (!subst.empty()) {
        if (result.empty() || node._paste || result.back() == '(') {
          result += subst;
        } else {
          result += ' ';
          result += subst;
        }
      }
    }
    if (!node._str.empty()) {
      if (result.empty() || node._paste || node._str[0] == ',' || node._str[0] == ')') {
        result += node._str;
      } else {
        result += ' ';
        result += node._str;
      }
    }
    if (!node._nested.empty()) {
      string nested_result;
      if (node._optional && args.size() >= _num_parameters) {
        nested_result = r_expand(node._nested, args, expand_undefined, ignores);
      }
      if (node._stringify) {
        nested_result = stringify(nested_result);
      }
      if (result.empty() || node._paste) {
        result += nested_result;
      } else {
        result += ' ';
        result += nested_result;
      }
    }
  }

  return result;
}
