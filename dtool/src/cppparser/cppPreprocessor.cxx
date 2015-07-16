// Filename: cppPreprocessor.cxx
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


#include "cppPreprocessor.h"
#include "cppExpressionParser.h"
#include "cppExpression.h"
#include "cppScope.h"
#include "cppIdentifier.h"
#include "cppTemplateScope.h"
#include "cppTemplateParameterList.h"
#include "cppSimpleType.h"
#include "cppGlobals.h"
#include "cppCommentBlock.h"
#include "cppBison.h"
#include "indent.h"
#include "pstrtod.h"

#include <assert.h>
#include <ctype.h>

// We manage our own visibility counter, in addition to that managed
// by cppBison.y.  We do this just so we can define manifests with the
// correct visibility when they are declared.  (Asking the parser for
// the current visibility is prone to error, since the parser might be
// several tokens behind the preprocessor.)
static CPPVisibility preprocessor_vis = V_public;

static int
hex_val(int c) {
  switch (c) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return (c - '0');

  default:
    return (tolower(c) - 'a' + 10);
  }
}

static string
trim_blanks(const string &str) {
  size_t first, last;

  if(str.empty())
      return str;

  first = 0;
  while (first < str.length() && isspace(str[first])) {
    first++;
  }

  last = str.length() - 1;
  while (last > first && isspace(str[last])) {
    last--;
  }

  return str.substr(first, last - first + 1);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::InputFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPPreprocessor::InputFile::
InputFile() {
  _in = NULL;
  _ignore_manifest = NULL;
  _line_number = 1;
  _col_number = 1;
  _lock_position = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::InputFile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPPreprocessor::InputFile::
~InputFile() {
  if (_in != NULL) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting
    // the stream pointer does not call the appropriate global delete
    // function; instead apparently calling the system delete
    // function.  So we call the delete function by hand instead.
#if !defined(USE_MEMORY_NOWRAPPERS) && defined(REDEFINE_GLOBAL_OPERATOR_NEW)
    _in->~istream();
    (*global_operator_delete)(_in);
#else
    delete _in;
#endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::InputFile::open
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPPreprocessor::InputFile::
open(const CPPFile &file) {
  assert(_in == NULL);

  _file = file;
  pifstream *in = new pifstream;
  _in = in;

  return _file._filename.open_read(*in);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::InputFile::connect_input
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPPreprocessor::InputFile::
connect_input(const string &input) {
  assert(_in == NULL);

  _input = input;
  _in = new istringstream(_input);
  return !_in->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::InputFile::get
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::InputFile::
get() {
  assert(_in != NULL);
  int c = _in->get();

  // Quietly skip over embedded carriage-return characters.  We
  // shouldn't see any of these unless there was some DOS-to-Unix file
  // conversion problem.
  while (c == '\r') {
    c = _in->get();
  }

  switch (c) {
  case EOF:
    break;

  case '\n':
    if (!_lock_position) {
      _line_number++;
      _col_number = 1;
    }
    break;

  default:
    if (!_lock_position) {
      _col_number++;
    }
  }

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPPreprocessor::
CPPPreprocessor() {
  _noangles = false;
  _state = S_eof;
  _paren_nesting = 0;
  _angle_bracket_found = false;
  _unget = '\0';
  _last_c = '\0';
  _start_of_line = true;
  _last_cpp_comment = false;
  _save_comments = true;

  _resolve_identifiers = true;

  _warning_count = 0;
  _error_count = 0;
  _error_abort = false;
#ifdef CPP_VERBOSE_LEX
  _token_index = 0;
#endif
  _verbose = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::set_verbose
//       Access: Public
//  Description: Sets the verbosity level of the parser.  At 0, no
//               warnings will be reported; at 1 or higher, expect to
//               get spammed.
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
set_verbose(int verbose) {
  _verbose = verbose;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_verbose
//       Access: Public
//  Description: Returns the verbosity level of the parser.
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
get_verbose() const {
  return _verbose;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::copy_filepos
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
copy_filepos(const CPPPreprocessor &other) {
  assert(!_files.empty());
  _files.back()._file = other.get_file();
  _files.back()._line_number = other.get_line_number();
  _files.back()._col_number = other.get_col_number();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_file
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPFile CPPPreprocessor::
get_file() const {
  if (_files.empty()) {
    return CPPFile("");
  }
  return _files.back()._file;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_line_number
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
get_line_number() const {
  if (_files.empty()) {
    return 0;
  }
  return _files.back()._line_number;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_col_number
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
get_col_number() const {
  if (_files.empty()) {
    return 0;
  }
  return _files.back()._col_number;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_next_token
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
get_next_token() {

#ifdef CPP_VERBOSE_LEX
  CPPToken tok = get_next_token0();
  indent(cerr, _files.size() * 2)
    << _token_index++ << ". " << tok << "\n";
  return tok;
}

CPPToken CPPPreprocessor::
get_next_token0() {
#endif

  // We make a nested call to internal_get_next_token(), so we can
  // combine sequences of identifiers and scoping symbols into a
  // single identifier, for yacc's convenience.

  CPPToken token(0);
  if (!_saved_tokens.empty()) {
    token = _saved_tokens.back();
    _saved_tokens.pop_back();
  } else {
    token = internal_get_next_token();
  }

  int first_line = token._lloc.first_line;
  int first_col = token._lloc.first_column;
  CPPFile first_file = token._lloc.file;

  if (_resolve_identifiers &&
      (token._token == SIMPLE_IDENTIFIER || token._token == SCOPE)) {
    // We will be returning a scoped identifier, or a scoping.  Keep
    // pulling off tokens until we reach the end of the
    // scope/identifier sequence.

    string name;

    // If we started the ball with an identifier, use it and get the
    // next token.  Otherwise, we started with :: (global scope), and
    // we indicate this with an empty string at the beginning of the
    // scoping sequence.
    if (token._token == SIMPLE_IDENTIFIER) {
      name = token._lval.str;
      token = internal_get_next_token();
    }

    CPPIdentifier *ident = new CPPIdentifier(name, token._lloc.file);
    YYSTYPE result;
    result.u.identifier = ident;

    if (token._token == '<') {
      // If the next token is an angle bracket and the current
      // identifier wants template instantiation, assume the angle
      // bracket begins the instantiation and call yacc recursively to
      // parse the template parameters.
      CPPDeclaration *decl = ident->find_template(current_scope, global_scope);
      if (decl != NULL) {
        ident->_names.back().set_templ
          (nested_parse_template_instantiation(decl->get_template_scope()));
        token = internal_get_next_token();
      }
    }

    while (token._token == SCOPE) {
      name += "::";
      token = internal_get_next_token();
      string token_prefix;

      if (token._token == '~') {
        // A scoping operator followed by a tilde can only be the
        // start of a scoped destructor name.  Make the tilde be part
        // of the name.
        name += "~";
        token_prefix = "~";
        token = internal_get_next_token();
      }

      if (token._token != SIMPLE_IDENTIFIER) {
        // The last useful token was a SCOPE, thus this is a scoping
        // token.

        if (token._token == KW_OPERATOR) {
          // Unless the last token we came across was the "operator"
          // keyword.  We make a special case for this, because it's
          // occasionally scoped in normal use.
          token._lval = result;
          return token;
        }
        _saved_tokens.push_back(token);
        return CPPToken(SCOPING, first_line, first_col, first_file,
                        name, result);
      }

      name += token._lval.str;
      ident->_names.push_back(token_prefix + token._lval.str);

      token = internal_get_next_token();

      if (token._token == '<') {
        // If the next token is an angle bracket and the current
        // indentifier wants template instantiation, assume the angle
        // bracket begins the instantiation and call yacc recursively to
        // parse the template parameters.
        CPPDeclaration *decl =
          ident->find_template(current_scope, global_scope);
        if (decl != NULL) {
          ident->_names.back().set_templ
            (nested_parse_template_instantiation(decl->get_template_scope()));
          token = internal_get_next_token();
        }
      }
    }
    // The last useful token was a SIMPLE_IDENTIFIER, thus this is a
    // normal scoped identifier.
    _saved_tokens.push_back(token);

    int token_type = IDENTIFIER;
    CPPDeclaration *decl = ident->find_symbol(current_scope, global_scope);
    if (decl != NULL && decl->as_type() != NULL) {
      token_type = TYPENAME_IDENTIFIER;
    }

    return CPPToken(token_type, first_line, first_col, first_file,
                    name, result);
  }

  // This is the normal case: just pass through whatever token we got.
  return token;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::warning
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
warning(const string &message, int line, int col, CPPFile file) {
  if (_verbose >= 2) {
    if (line == 0) {
      line = get_line_number();
      col = get_col_number();
    }
    if (file.empty()) {
      file = get_file();
    }
    int indent_level = 0;
    if (_verbose >= 3) {
      indent_level = _files.size() * 2;
    }
    indent(cerr, indent_level)
      << "*** Warning in " << file
      << " near line " << line << ", column " << col << ":\n";
    indent(cerr, indent_level)
      << message << "\n";
  }
  _warning_count++;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::error
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
error(const string &message, int line, int col, CPPFile file) {
  if (_state == S_nested || _state == S_end_nested) {
    // Don't report or log errors in the nested state.  These will be
    // reported when the nesting level collapses.
    return;
  }

  if (_verbose >= 1) {
    if (line == 0) {
      line = get_line_number();
      col = get_col_number();
    }
    if (file.empty()) {
      file = get_file();
    }
    int indent_level = 0;
    if (_verbose >= 3) {
      indent_level = _files.size() * 2;
    }
    indent(cerr, indent_level)
      << "*** Error in " << file
      << " near line " << line << ", column " << col << ":\n";
    indent(cerr, indent_level)
      << message << "\n";

    if (_error_abort) {
      cerr << "Aborting.\n";
      abort();
    }
  }
  _error_count++;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_warning_count
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
get_warning_count() const {
  return _warning_count;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_error_count
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
get_error_count() const {
  return _error_count;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_comment_before
//       Access: Public
//  Description: Returns the CPPCommentBlock immediately preceding the
//               indicated line, if any.  If there is no such comment,
//               returns NULL.
////////////////////////////////////////////////////////////////////
CPPCommentBlock *CPPPreprocessor::
get_comment_before(int line, CPPFile file) {
  CPPComments::reverse_iterator ci;
  ci = _comments.rbegin();

  int wrong_file_count = 0;
  while (ci != _comments.rend()) {
    CPPCommentBlock *comment = (*ci);
    if (comment->_file == file) {
      wrong_file_count = 0;
      if (comment->_last_line == line || comment->_last_line == line - 1) {
        return comment;
      }

      if (comment->_last_line < line) {
        return (CPPCommentBlock *)NULL;
      }
    } else {
      wrong_file_count++;
      if (wrong_file_count > 10) {
        return (CPPCommentBlock *)NULL;
      }
    }

    ++ci;
  }

  return (CPPCommentBlock *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_comment_on
//       Access: Public
//  Description: Returns the CPPCommentBlock that starts on the
//               indicated line, if any.  If there is no such
//               comment, returns NULL.
////////////////////////////////////////////////////////////////////
CPPCommentBlock *CPPPreprocessor::
get_comment_on(int line, CPPFile file) {
  CPPComments::reverse_iterator ci;
  ci = _comments.rbegin();

  while (ci != _comments.rend()) {
    CPPCommentBlock *comment = (*ci);
    if (comment->_file == file) {
      if (comment->_line_number == line) {
        return comment;
      } else if (comment->_line_number < line) {
        return (CPPCommentBlock *)NULL;
      }
    }

    ++ci;
  }

  return (CPPCommentBlock *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::init_cpp
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPPreprocessor::
init_cpp(const CPPFile &file) {
  _state = S_normal;
  _saved_tokens.push_back(CPPToken(START_CPP));
  _last_c = '\0';

  return push_file(file);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::init_const_expr
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPPreprocessor::
init_const_expr(const string &expr) {
  _state = S_normal;
  _saved_tokens.push_back(CPPToken(START_CONST_EXPR));

  return push_string(expr, false);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::init_type
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPPreprocessor::
init_type(const string &type) {
  _state = S_normal;
  _saved_tokens.push_back(CPPToken(START_TYPE));

  return push_string(type, false);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::push_file
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPPreprocessor::
push_file(const CPPFile &file) {
  if (_verbose >= 3) {
    indent(cerr, _files.size() * 2)
      << "Reading " << file << "\n";
  }

  _files.push_back(InputFile());
  InputFile &infile = _files.back();

  if (infile.open(file)) {
    // Record the fact that we opened the file for the benefit of user
    // code.
    _parsed_files.insert(file);

    infile._prev_last_c = _last_c;
    _last_c = '\0';
    _start_of_line = true;
    return true;
  }

  _files.pop_back();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::push_string
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPPreprocessor::
push_string(const string &input, bool lock_position) {
#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "Pushing to string \"" << input
    << "\"\nlock_position = " << lock_position << "\n";
#endif
  CPPFile first_file = get_file();
  int first_line = get_line_number();
  int first_col = get_col_number();

  _files.push_back(InputFile());
  InputFile &infile = _files.back();

  if (infile.connect_input(input)) {
    if (lock_position) {
      infile._file = first_file;
      infile._line_number = first_line;
      infile._col_number = first_col;
      infile._lock_position = true;
    }

    infile._prev_last_c = _last_c;
    _last_c = '\0';
    return true;
  }

#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "Unable to read string\n";
#endif

  _files.pop_back();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::expand_manifests
//       Access: Protected
//  Description: Given a string, expand all manifests within the
//               string and return the new string.
////////////////////////////////////////////////////////////////////
string CPPPreprocessor::
expand_manifests(const string &input_expr, bool expand_undefined) {
  // Get a copy of the expression string we can modify.
  string expr = input_expr;

  // Repeatedly scan the expr for any manifest names or defined()
  // function.

  // We'll need to save the set of manifests we've already expanded,
  // to guard against recursive references.
  set<const CPPManifest *> already_expanded;

  bool manifest_found;
  do {
    manifest_found = false;
    size_t p = 0;
    while (p < expr.size()) {
      if (isalpha(expr[p]) || expr[p] == '_') {
        size_t q = p;
        while (p < expr.size() && (isalnum(expr[p]) || expr[p] == '_')) {
          p++;
        }
        string ident = expr.substr(q, p - q);

        // Here's an identifier.  Is it "defined"?
        if (ident == "defined") {
          expand_defined_function(expr, q, p);
        } else {
          // Is it a manifest?
          Manifests::const_iterator mi = _manifests.find(ident);
          if (mi != _manifests.end()) {
            const CPPManifest *manifest = (*mi).second;
            if (already_expanded.insert(manifest).second) {
              expand_manifest_inline(expr, q, p, (*mi).second);
              manifest_found = true;
            }
          } else if (expand_undefined && ident != "true" && ident != "false") {
            // It is not found.  Expand it to 0.
            expr = expr.substr(0, q) + "0" + expr.substr(p);
            p = q + 1;
          }
        }
      } else {
        p++;
      }
    }

    // If we expanded any manifests at all that time, then go back
    // through the string and look again--we might have a manifest
    // that expands to another manifest.
  } while (manifest_found);

  return expr;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::parse_expr
//       Access: Protected
//  Description: Given a string, expand all manifests within the
//               string and evaluate it as an expression.  Returns
//               NULL if the string is not a valid expression.
//
//               This is an internal support function for
//               CPPPreprocessor; however, there is a public variant
//               of this function defined for CPPParser.
////////////////////////////////////////////////////////////////////
CPPExpression *CPPPreprocessor::
parse_expr(const string &input_expr, CPPScope *current_scope,
           CPPScope *global_scope) {
  string expr = expand_manifests(input_expr, true);

  CPPExpressionParser ep(current_scope, global_scope);
  ep._verbose = 0;
  if (ep.parse_expr(expr, *this)) {
    return ep._expr;
  } else {
    return (CPPExpression *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::internal_get_next_token
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
internal_get_next_token() {
  if (_state == S_eof || _state == S_end_nested) {
    return CPPToken::eof();
  }

  int c = _last_c;
  _last_c = '\0';
  if (c == '\0' || c == EOF) {
    c = get();
  }

  // Skip any whitespace, comments, and preprocessor directives before
  // the token.
  c = skip_whitespace(c);
  while (c == '#' && _start_of_line && !should_ignore_preprocessor()) {
    c = skip_whitespace(process_directive(c));
  }

  if (c == '\'') {
    return get_quoted_char(c);
  } else if (c == '"') {
    return get_quoted_string(c);
  } else if (isalpha(c) || c == '_') {
    return get_identifier(c);
  } else if (isdigit(c)) {
    return get_number(c);
  }

  _last_c = c;
  if (c == EOF) {
    _state = S_eof;
    return CPPToken::eof();
  }

  CPPFile first_file = get_file();
  int first_line = get_line_number();
  int first_col = get_col_number();

  // Check for a number beginning with a decimal point.
  int next_c = get();
  if (c == '.' && isdigit(next_c)) {
    return get_number(c, next_c);
  }

  // Check for two- or three-character tokens.
  _last_c = get();

  switch (c) {
  case '+':
    if (next_c == '+') return CPPToken(PLUSPLUS, first_line, first_col, first_file);
    if (next_c == '=') return CPPToken(PLUSEQUAL, first_line, first_col, first_file);
    break;

  case '-':
    if (next_c == '-') return CPPToken(MINUSMINUS, first_line, first_col, first_file);
    if (next_c == '=') return CPPToken(MINUSEQUAL, first_line, first_col, first_file);
    if (next_c == '>' && _last_c == '*') {
      _last_c = get();
      return CPPToken(POINTSAT_STAR, first_line, first_col, first_file);
    }
    if (next_c == '>') return CPPToken(POINTSAT, first_line, first_col, first_file);
    break;

  case '<':
    if (next_c == '<' && _last_c == '=') {
      _last_c = get();
      return CPPToken(LSHIFTEQUAL, first_line, first_col, first_file);
    }
    if (next_c == '<') return CPPToken(LSHIFT, first_line, first_col, first_file);
    if (next_c == '=') return CPPToken(LECOMPARE, first_line, first_col, first_file);
    if (next_c == ':') return CPPToken('[', first_line, first_col, first_file);
    if (next_c == '%') return CPPToken('{', first_line, first_col, first_file);
    break;

  case '>':
    if (next_c == '>' && _last_c == '=') {
      _last_c = get();
      return CPPToken(RSHIFTEQUAL, first_line, first_col, first_file);
    }
    if (next_c == '>') return CPPToken(RSHIFT, first_line, first_col, first_file);
    if (next_c == '=') return CPPToken(GECOMPARE, first_line, first_col, first_file);
    break;

  case '|':
    if (next_c == '|') return CPPToken(OROR, first_line, first_col, first_file);
    if (next_c == '=') return CPPToken(OREQUAL, first_line, first_col, first_file);
    break;

  case '&':
    if (next_c == '&') return CPPToken(ANDAND, first_line, first_col, first_file);
    if (next_c == '=') return CPPToken(ANDEQUAL, first_line, first_col, first_file);
    break;

  case '^':
    if (next_c == '=') return CPPToken(XOREQUAL, first_line, first_col, first_file);
    break;

  case '=':
    if (next_c == '=') return CPPToken(EQCOMPARE, first_line, first_col, first_file);
    break;

  case '!':
    if (next_c == '=') return CPPToken(NECOMPARE, first_line, first_col, first_file);
    break;

  case '.':
    if (next_c == '*') return CPPToken(DOT_STAR, first_line, first_col, first_file);
    if (next_c == '.' && _last_c == '.') {
      _last_c = get();
      return CPPToken(ELLIPSIS, first_line, first_col, first_file);
    }
    break;

  case ':':
    if (next_c == ':') return CPPToken(SCOPE, first_line, first_col, first_file);
    if (next_c == '>') return CPPToken(']', first_line, first_col, first_file);
    break;

  case '*':
    if (next_c == '=') return CPPToken(TIMESEQUAL, first_line, first_col, first_file);
    break;

  case '/':
    if (next_c == '=') return CPPToken(DIVIDEEQUAL, first_line, first_col, first_file);
    break;

  case '%':
    if (next_c == '=') return CPPToken(MODEQUAL, first_line, first_col, first_file);
    if (next_c == '>') return CPPToken('}', first_line, first_col, first_file);
    break;
  }

  // It wasn't any of the two- or three-character tokens, so put back
  // the lookahead character and return the one-character token.
  unget(_last_c);
  _last_c = next_c;

  if (_state == S_nested) {
    // If we're running a nested lexer, keep track of the paren
    // levels.  When we encounter a comma or closing angle bracket at
    // the bottom level, we stop.

    switch (c) {
    case '(':
    case '[':
      _paren_nesting++;
      break;

    case ')':
    case ']':
      _paren_nesting--;
      break;

    case ',':
      if (_paren_nesting <= 0) {
        _state = S_end_nested;
        return CPPToken::eof();
      }
      break;

    case '>':
      if (_paren_nesting <= 0) {
        _angle_bracket_found = true;
        _state = S_end_nested;
        return CPPToken::eof();
      }
    }
  }

  // We skip whitespace here again, so that we can read any comments
  // after this point before we parse this line.
  _last_c = skip_whitespace(_last_c);

  return CPPToken(c, first_line, first_col, first_file);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::skip_whitespace
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
skip_whitespace(int c) {
  while (c != EOF) {
    c = skip_comment(c);

    if (c == '\\') {
      // This does not usually occur in the middle of unquoted C++
      // code, except before a newline character.
      c = get();
      if (c != '\n') {
        unget(c);
        return '\\';
      }
    }

    if (!isspace(c)) {
      return c;
    }
    c = get();
  }
  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::skip_comment
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
skip_comment(int c) {
  if (c == '/') {
    int next_c = get();
    if (next_c == '*') {
      _last_cpp_comment = false;
      c = skip_c_comment(get());
    } else if (next_c == '/') {
      c = skip_cpp_comment(get());
    } else {
      _last_cpp_comment = false;
      unget(next_c);
      return c;
    }
  }
  if (!isspace(c)) {
    _last_cpp_comment = false;
  }
  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::skip_c_comment
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
skip_c_comment(int c) {
  if (_save_comments) {
    CPPCommentBlock *comment = new CPPCommentBlock;
    _comments.push_back(comment);

    comment->_file = get_file();
    comment->_line_number = get_line_number();
    comment->_last_line = get_line_number();
    comment->_col_number = get_col_number() - 2;
    comment->_c_style = true;
    comment->_comment = "/*";

    while (c != EOF) {
      if (c == '*') {
        comment->_comment += c;
        c = get();
        if (c == '/') {
          comment->_comment += c;
          comment->_last_line = get_line_number();
          return get();
        }
      } else {
        comment->_comment += c;
        c = get();
      }
    }

    warning("Comment is unterminated",
            comment->_line_number, comment->_col_number,
            comment->_file);

  } else {
    CPPFile first_file = get_file();
    int first_line_number = get_line_number();
    int first_col_number = get_col_number() - 2;

    while (c != EOF) {
      if (c == '*') {
        c = get();
        if (c == '/') {
          return get();
        }
      } else {
        c = get();
      }
    }

    warning("Comment is unterminated",
            first_line_number, first_col_number,
            first_file);
  }

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::skip_cpp_comment
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
skip_cpp_comment(int c) {
  if (_save_comments) {
    CPPCommentBlock *comment;

    int line_number = get_line_number();
    if (c == '\n') {
      // We have to subtract one from the line number as we just
      // fetched a newline.
      --line_number;
    }

    if (_last_cpp_comment && !_comments.empty() &&
        _comments.back()->_last_line >= line_number - 1) {
      // If the last non-whitespace character read was also part of a
      // C++ comment, then this is just a continuation of that comment
      // block.  However, if there was a line without comment in between,
      // it starts a new block anyway.
      comment = _comments.back();
      assert(!comment->_c_style);
      comment->_comment += "//";

    } else {
      // Otherwise, this begins a new comment block.
      comment = new CPPCommentBlock;

      comment->_file = get_file();
      comment->_line_number = line_number;
      comment->_last_line = line_number;
      comment->_col_number = get_col_number() - 2;
      comment->_c_style = false;
      comment->_comment = "//";

      _comments.push_back(comment);
    }

    while (c != EOF && c != '\n') {
      comment->_comment += c;
      c = get();
    }

    comment->_comment += '\n';
    comment->_last_line = line_number;

    _last_cpp_comment = true;

  } else {
    while (c != EOF && c != '\n') {
      c = get();
    }
  }

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::process_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
process_directive(int c) {
  CPPFile first_file = get_file();
  int first_line = get_line_number();
  int first_col = get_col_number();

  string command, args;
  c = get_preprocessor_command(c, command);
  c = get_preprocessor_args(c, args);

#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "#" << command << " " << args << "\n";
#endif

  if (command == "define") {
    handle_define_directive(args, first_line, first_col, first_file);
  } else if (command == "undef") {
    handle_undef_directive(args, first_line, first_col, first_file);
  } else if (command == "ifdef") {
    handle_ifdef_directive(args, first_line, first_col, first_file);
  } else if (command == "ifndef") {
    handle_ifndef_directive(args, first_line, first_col, first_file);
  } else if (command == "if") {
    handle_if_directive(args, first_line, first_col, first_file);
  } else if (command == "else" || command == "elif") {
    // Presumably this follows some #if or #ifdef.  We don't bother to
    // check this, however.
    skip_false_if_block(false);
  } else if (command == "endif") {
    // Presumably this follows some #if or #ifdef.  We don't bother to
    // check this, however.
  } else if (command == "include") {
    handle_include_directive(args, first_line, first_col, first_file);
  } else if (command == "pragma") {
    handle_pragma_directive(args, first_line, first_col, first_file);
  } else if (command == "ident") {
    // Quietly ignore idents.
  } else if (command == "error") {
    handle_error_directive(args, first_line, first_col, first_file);
  } else {
    warning("Ignoring unknown directive #" + command,
            first_line, first_col, first_file);
  }

  _start_of_line = true;
  return '\n';
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_preprocessor_command
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
get_preprocessor_command(int c, string &command) {
  // Skip the hash mark.
  assert(c == '#');
  c = get();

  // Also skip any whitespace following the hash mark--but don't skip
  // past a newline.
  while (c != EOF && (c == ' ' || c == '\t')) {
    c = get();
  }

  // The next sequence of characters is the command.
  while (c != EOF && (isalnum(c) || c == '_')) {
    command += c;
    c = get();
  }

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_preprocessor_args
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
get_preprocessor_args(int c, string &args) {
  // Following the command, the rest of the line, as well as any text
  // on successive lines, is part of the arguments to the command.

  while (c != EOF && c != '\n') {
    if (c == '\\') {
      int next_c = get();
      if (next_c == '\n') {
        // Here we have an escaped newline: a continuation.
        args += '\n';
      } else {
        // Just a backslash followed by some non-backslash, keep both.
        args += c;
        if (next_c != EOF) {
          args += next_c;
        }
      }
    } else {
      args += c;
    }
    c = skip_comment(get());
  }

  // Remove any leading and trailing whitespace from the args.
  args = trim_blanks(args);

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_define_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_define_directive(const string &args, int first_line,
                        int first_col, const CPPFile &first_file) {
  if (args.empty()) {
    warning("Ignoring empty #define directive",
            first_line, first_col, first_file);
  } else {
    CPPManifest *manifest = new CPPManifest(args, first_file);
    manifest->_vis = preprocessor_vis;
    if (!manifest->_has_parameters) {
      string expr_string = manifest->expand();
      if (!expr_string.empty()) {
        manifest->_expr = parse_expr(expr_string, global_scope, global_scope);
      }
    }

    // ok one memory leak here..
    Manifests::iterator mi = _manifests.find(manifest->_name);
    if(mi != _manifests.end())
    {
        // i do not see a goodway to compare the old and new hmmmm
        //cerr << "Warning Overwriting Constant " << manifest->_name << "\n";
        delete mi->second;
    }

    _manifests[manifest->_name] = manifest;


  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_undef_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_undef_directive(const string &args, int first_line,
                       int first_col, const CPPFile &first_file) {
  if (args.empty()) {
    warning("Ignoring empty #undef directive",
            first_line, first_col, first_file);
  } else {
    Manifests::iterator mi = _manifests.find(args);
    if (mi != _manifests.end()) {
      _manifests.erase(mi);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_ifdef_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_ifdef_directive(const string &args, int, int, const CPPFile &) {
  Manifests::const_iterator mi = _manifests.find(args);
  if (mi != _manifests.end()) {
    // The macro is defined.  We continue.
    return;
  }

  // The macro is undefined.  Skip stuff.
  skip_false_if_block(true);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_ifndef_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_ifndef_directive(const string &args, int, int, const CPPFile &) {
  Manifests::const_iterator mi = _manifests.find(args);
  if (mi == _manifests.end()) {
    // The macro is undefined.  We continue.
    return;
  }

  // The macro is defined.  Skip stuff.
  skip_false_if_block(true);
}


////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_if_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_if_directive(const string &args, int first_line,
                    int first_col, const CPPFile &first_file) {
  CPPExpression *expr = parse_expr(args, global_scope, global_scope);

  int expression_result = 0;

  if (expr != (CPPExpression *)NULL) {
    CPPExpression::Result result = expr->evaluate();
    if (result._type == CPPExpression::RT_error) {
      warning("Ignoring invalid expression " + args,
              first_line, first_col, first_file);
    } else {
      expression_result = result.as_integer();
    }
  } else {
    warning("Ignoring invalid expression " + args,
            first_line, first_col, first_file);
  }

  if (expression_result) {
    // The expression result is true.  We continue.
    return;
  }

  // The expression result is false.  Skip stuff.
  skip_false_if_block(true);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_include_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_include_directive(const string &args, int first_line,
                         int first_col, const CPPFile &first_file) {
  bool okflag = false;
  Filename filename;
  Filename filename_as_referenced;
  bool angle_quotes = false;

  string expr = args;

  // The filename to include might actually be hidden within a
  // manifest definition.  Wow.  FreeType depends on this.

  // Just to play things safe, since our manifest-expansion logic
  // might not filter out quotes and angle brackets properly, we'll
  // only expand manifests if we don't begin with a quote or bracket.
  if (!expr.empty() && (expr[0] != '"' && expr[0] != '<')) {
    expr = expand_manifests(expr, false);
  }

  if (!expr.empty()) {
    if (expr[0] == '"' && expr[expr.size() - 1] == '"') {
      filename = expr.substr(1, expr.size() - 2);
      okflag = true;

      if (_files.size() == 1) {
        // If we're currently processing a top-level file, record the
        // include directive.  We don't need to record includes from
        // included files.
        _quote_includes.insert(filename);
      }
    } else if (expr[0] == '<' && expr[expr.size() - 1] == '>') {
      filename = expr.substr(1, expr.size() - 2);
      if (!_noangles) {
        // If _noangles is true, we don't make a distinction between
        // angle brackets and quote marks--all #include statements are
        // treated the same, as if they used quote marks.
        angle_quotes = true;
      }
      okflag = true;

      if (_files.size() == 1) {
        // If we're currently processing a top-level file, record the
        // include directive.  We don't need to record includes from
        // included files.
        _angle_includes.insert(filename);
      }
    }
  }

  filename.set_text();
  filename_as_referenced = filename;

  // Now look for the filename.  If we didn't use angle quotes, look
  // first in the current directory.
  bool found_file = false;
  CPPFile::Source source = CPPFile::S_none;

  if (okflag) {
    found_file = false;

    // Search the current directory.
    if (!angle_quotes && !found_file && filename.exists()) {
      found_file = true;
      source = CPPFile::S_local;
    }

    // Search the same directory as the includer.
    if (!angle_quotes && !found_file) {
      Filename match(get_file()._filename.get_dirname(), filename);
      if (match.exists()) {
        filename = match;
        found_file = true;
        source = CPPFile::S_alternate;
      }
    }

    // Now search the angle-include-path
    if (angle_quotes && !found_file && filename.resolve_filename(_angle_include_path)) {
      found_file = true;
      source = CPPFile::S_system;
    }

    // Now search the quote-include-path
    if (!angle_quotes && !found_file) {
      for (int dir=0; dir<_quote_include_path.get_num_directories(); dir++) {
        Filename match(_quote_include_path.get_directory(dir), filename);
        if (match.exists()) {
          filename = match;
          found_file = true;
          source = _quote_include_kind[dir];
        }
      }
    }

    if (!found_file) {
      warning("Cannot find " + filename.get_fullpath(),
              first_line, first_col, first_file);
    } else {
      _last_c = '\0';

      CPPFile file(filename, filename_as_referenced, source);

      // Don't include it if we included it before and it had #pragma once.
      ParsedFiles::const_iterator it = _parsed_files.find(file);
      if (it->_pragma_once) {
        return;
      }

      if (!push_file(file)) {
        warning("Unable to read " + filename.get_fullpath(),
                first_line, first_col, first_file);
      }
    }
  } else {
    warning("Ignoring invalid #include directive",
            first_line, first_col, first_file);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_pragma_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_pragma_directive(const string &args, int first_line,
                        int first_col, const CPPFile &first_file) {
  if (args == "once") {
    ParsedFiles::iterator it = _parsed_files.find(first_file);
    assert(it != _parsed_files.end());
    it->_pragma_once = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_error_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_error_directive(const string &args, int first_line,
                       int first_col, const CPPFile &first_file) {
  error(args, first_line, first_col, first_file);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::skip_false_if_block
//       Access: Private
//  Description: We come here when we fail an #if or an #ifdef test,
//               or when we reach the #else clause to something we
//               didn't fail.  This function skips all text up until
//               the matching #endif.
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
skip_false_if_block(bool consider_elifs) {
  int level = 0;
  _save_comments = false;

  int c = skip_comment(get());
  while (c != EOF) {
    if (c == '#' && _start_of_line) {
      CPPFile first_file = get_file();
      int first_line = get_line_number();
      int first_col = get_col_number();

      // Is this it?
      string command, args;
      c = get_preprocessor_command(c, command);
      c = get_preprocessor_args(c, args);
      if (command == "if" || command == "ifdef" || command == "ifndef") {
        // Hmm, a nested if block.  Even more to skip.
        level++;
      } else if (command == "else") {
        if (level == 0 && consider_elifs) {
          // This will do!
          _save_comments = true;
          return;
        }
      } else if (command == "elif") {
        if (level == 0 && consider_elifs) {
          // If we pass this test, we're in.
          _save_comments = true;
          handle_if_directive(args, first_line, first_col, first_file);
          return;
        }
      } else if (command == "endif") {
        // Skip any args.
        if (level == 0) {
          // Here's the end!
          _save_comments = true;
          return;
        }
        level--;
      }
    } else {
      c = skip_comment(get());
    }
  }

  _save_comments = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_quoted_char
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
get_quoted_char(int c) {
  CPPFile first_file = get_file();
  int first_line = get_line_number();
  int first_col = get_col_number();
  string str = scan_quoted(c);

  YYSTYPE result;
  if (!str.empty()) {
    result.u.integer = (int)str[0];
  } else {
    result.u.integer = 0;
  }
  return CPPToken(CHAR_TOK, first_line, first_col, first_file, str, result);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_quoted_string
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
get_quoted_string(int c) {
  CPPFile first_file = get_file();
  int first_line = get_line_number();
  int first_col = get_col_number();
  string str = scan_quoted(c);
  return CPPToken(STRING, first_line, first_col, first_file, str);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_identifier
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
get_identifier(int c) {
  CPPFile first_file = get_file();
  int first_line = get_line_number();
  int first_col = get_col_number();

  string name(1, (char)c);

  c = get();
  while (c != EOF && (isalnum(c) || c == '_')) {
    name += c;
    c = get();
  }
  if (c == '\'' || c == '"') {
    // This is actually a wide-character or wide-string literal or
    // some such: a string with an alphanumeric prefix.  We don't
    // necessarily try to parse it correctly; for most purposes, we
    // don't care.
    CPPToken token(0);
    if (c == '\'') {
      token = get_quoted_char(c);
    } else {
      token = get_quoted_string(c);
    }
    token._lloc.first_column = first_col;
    return token;
  }

  _last_c = c;

  // Is it a manifest?
  Manifests::const_iterator mi = _manifests.find(name);
  if (mi != _manifests.end() && !should_ignore_manifest((*mi).second)) {
    return expand_manifest((*mi).second);
  }

  // Check for keywords.
  int kw = check_keyword(name);

  // Update our internal visibility flag.
  switch (kw) {
  case KW_BEGIN_PUBLISH:
    preprocessor_vis = V_published;
    break;

  case KW_END_PUBLISH:
    preprocessor_vis = V_public;
    break;
  }

  if (kw != 0) {
    YYSTYPE result;
    result.u.identifier = (CPPIdentifier *)NULL;
    return CPPToken(kw, first_line, first_col, first_file, name, result);
  }

  return CPPToken(SIMPLE_IDENTIFIER, first_line, first_col, first_file,
                  name);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::expand_manifest
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
expand_manifest(const CPPManifest *manifest) {
  vector_string args;

  if (manifest->_has_parameters) {
    // Hmm, we're expecting arguments.
    extract_manifest_args(manifest->_name, manifest->_num_parameters,
                          manifest->_variadic_param, args);
  }

  string expanded = " " + manifest->expand(args) + " ";
  push_string(expanded, true);

  if (!manifest->_has_parameters) {
    // If the manifest does not use arguments, then disallow recursive
    // expansion.
    _files.back()._ignore_manifest = manifest;
  }

#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "Expanding " << manifest->_name << " to " << expanded << "\n";
#endif

  return internal_get_next_token();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::extract_manifest_args
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
extract_manifest_args(const string &name, int num_args, int va_arg,
                      vector_string &args) {
  CPPFile first_file = get_file();
  int first_line = get_line_number();
  int first_col = get_col_number();

  // Skip whitespace till paren.
  int c = _last_c;
  _last_c = '\0';
  while (c != EOF && isspace(c)) {
    c = get();
  }

  if (c != '(') {
    // No paren, so we have only one arg.
    string arg;
    while (c != EOF && (isalnum(c) || c == '_')) {
      arg += c;
      c = get();
    }
    args.push_back(arg);

  } else {
    // Skip paren.
    c = skip_whitespace(get());
    int paren_level = 1;
    string arg;
    while (c != EOF) {
      if (c == ',' && paren_level == 1) {
        args.push_back(arg);
        arg = "";
        c = get();

      } else if (c == '"' || c == '\'') {
        // Quoted string or character.
        int quote_mark = c;
        arg += c;
        c = get();
        while (c != EOF && c != quote_mark && c != '\n') {
          if (c == '\\') {
            arg += c;
            c = get();
          }
          if (c != EOF) {
            arg += c;
            c = get();
          }
        }
        arg += c;
        c = get();

      } else if (c == '(') {
        arg += '(';
        ++paren_level;
        c = get();

      } else if (c == ')') {
        --paren_level;
        if (paren_level == 0) {
          break;
        }
        arg += ')';
        c = get();

      } else if (isspace(c)) {
        // Skip extra whitespace.
        c = skip_whitespace(c);
        if (!arg.empty()) {
          arg += ' ';
        }

      } else if (c == '\\') {
        // It could be a slash before a newline.
        // If so, that's whitespace as well.
        c = get();
        if (c != '\n') {
          arg += '\\';
        } else if (!arg.empty()) {
          arg += ' ';
          c = skip_whitespace(get());
        }

      } else {
        arg += c;
        c = get();
      }
    }
    if (num_args != 0 || !arg.empty()) {
      args.push_back(arg);
    }
  }

  if ((int)args.size() < num_args) {
    warning("Not enough arguments for manifest " + name,
            first_line, first_col, first_file);

  } else if (va_arg < 0 && (int)args.size() > num_args) {
    warning("Too many arguments for manifest " + name,
            first_line, first_col, first_file);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::expand_defined_function
//       Access: Private
//  Description: Expands the defined(manifest) function to either
//               1 or 0, depending on whether the manifest exists.
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
expand_defined_function(string &expr, size_t q, size_t &p) {
  string result;

  vector_string args;
  extract_manifest_args_inline("defined", 1, -1, args, expr, p);
  if (args.size() >= 1) {
    const string &manifest_name = args[0];
    Manifests::const_iterator mi = _manifests.find(manifest_name);
    if (mi != _manifests.end()) {
      // The macro is defined; the result is "1".
      result = "1";
    } else {
      // The macro is undefined; the result is "0".
      result = "0";
    }
  }

  expr = expr.substr(0, q) + result + expr.substr(p);
  p = q + result.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::expand_manifest_inline
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
expand_manifest_inline(string &expr, size_t q, size_t &p,
                       const CPPManifest *manifest) {
  vector_string args;
  if (manifest->_has_parameters) {
    extract_manifest_args_inline(manifest->_name, manifest->_num_parameters,
                                 manifest->_variadic_param, args, expr, p);
  }
  string result = manifest->expand(args);

  expr = expr.substr(0, q) + result + expr.substr(p);
  p = q + result.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::extract_manifest_args_inline
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
extract_manifest_args_inline(const string &name, int num_args,
                             int va_arg, vector_string &args,
                             const string &expr, size_t &p) {
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

  } else if (expr[p] == '"' || expr[p] == '\'') {
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

  } else {
    // Skip paren.
    p++;
    size_t q = p;
    while (p < expr.size() && expr[p] != ')') {
      if (expr[p] == ',') {
        args.push_back(expr.substr(q, p - q));
        q = p+1;

      } else if (expr[p] == '(') {
        // Nested parens.
        int paren_level = 1;
        while (p+1 < expr.size() && paren_level > 0) {
          p++;
          if (expr[p] == '(') {
            paren_level++;
          } else if (expr[p] == ')') {
            paren_level--;
          }
        }
      }
      p++;
    }
    args.push_back(expr.substr(q, p - q));

    if (p < expr.size() && expr[p] == ')') {
      p++;
    }
  }

  if ((int)args.size() < num_args) {
    warning("Not enough arguments for manifest " + name);

  } else if (va_arg < 0 && (int)args.size() > num_args) {
    warning("Too many arguments for manifest " + name);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_number
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
get_number(int c, int c2) {
  CPPFile first_file = get_file();
  int first_line = get_line_number();
  int first_col = get_col_number();

  string num(1, (char)c);
  bool leading_zero = (c == '0');
  bool decimal_point = (c == '.');

  if (c2 == 0) {
    c = get();
  } else {
    c = c2;
  }
  if (leading_zero && c == 'x') {
    // Here we have a hex number.
    num += c;
    c = get();

    while (c != EOF && (isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f'))) {
      num += c;
      c = get();
    }

    while (c == 'L' || c == 'U') {
      // We allow (and ignore) an 'L' and/or 'U' following the number.
      c = get();
    }

    _last_c = c;

    YYSTYPE result;
    result.u.integer = strtol(num.c_str(), (char **)NULL, 16);
    return CPPToken(INTEGER, first_line, first_col, first_file, num, result);
  }

  while (c != EOF && isdigit(c)) {
    num += c;
    c = get();
  }

  if (c == '.' && !decimal_point) {
    // Now we have a floating-point number.
    decimal_point = true;
    num += c;
    c = get();

    while (c != EOF && isdigit(c)) {
      num += c;
      c = get();
    }
  }

  if (decimal_point) {
    if (tolower(c) == 'e') {
      // An exponent is allowed.
      num += c;
      c = get();
      if (c == '-' || c == '+') {
        num += c;
        c = get();
      }
      while (c != EOF && isdigit(c)) {
        num += c;
        c = get();
      }
    }

    if (c == 'f') {
      // We allow (and ignore) an 'f' following the number.
      c = get();
    }

    _last_c = c;
    YYSTYPE result;
    result.u.real = pstrtod(num.c_str(), (char **)NULL);
    return CPPToken(REAL, first_line, first_col, first_file, num, result);
  }

  // This is a decimal or octal integer number.

  while (c == 'L' || c == 'U') {
    // We allow (and ignore) an 'L' and/or 'U' following the number.
    c = get();
  }

  _last_c = c;
  YYSTYPE result;

  if (leading_zero) {
    // A leading zero implies an octal number.  strtol() is supposed
    // to be able to make this distinction by itself, but we'll do it
    // explicitly just to be sure.
    result.u.integer = strtol(num.c_str(), (char **)NULL, 8);

  } else {
    // A decimal (base 10) integer.
    result.u.integer = strtol(num.c_str(), (char **)NULL, 10);
  }

  return CPPToken(INTEGER, first_line, first_col, first_file, num, result);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::check_keyword
//       Access: Private, Static
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
check_keyword(const string &name) {
  if (name == "__begin_publish") return KW_BEGIN_PUBLISH;
  if (name == "__blocking") return KW_BLOCKING;
  if (name == "bool") return KW_BOOL;
  if (name == "catch") return KW_CATCH;
  if (name == "char") return KW_CHAR;
  if (name == "char16_t") return KW_CHAR16_T;
  if (name == "char32_t") return KW_CHAR32_T;
  if (name == "class") return KW_CLASS;
  if (name == "const") return KW_CONST;
  if (name == "delete") return KW_DELETE;
  if (name == "double") return KW_DOUBLE;
  if (name == "dynamic_cast") return KW_DYNAMIC_CAST;
  if (name == "else") return KW_ELSE;
  if (name == "__end_publish") return KW_END_PUBLISH;
  if (name == "enum") return KW_ENUM;
  if (name == "extern") return KW_EXTERN;
  if (name == "__extension") return KW_EXTENSION;
  if (name == "explicit") return KW_EXPLICIT;
  if (name == "__published") return KW_PUBLISHED;
  if (name == "false") return KW_FALSE;
  if (name == "float") return KW_FLOAT;
  if (name == "friend") return KW_FRIEND;
  if (name == "for") return KW_FOR;
  if (name == "goto") return KW_GOTO;
  if (name == "if") return KW_IF;
  if (name == "inline") return KW_INLINE;
  if (name == "int") return KW_INT;
  if (name == "long") return KW_LONG;
  if (name == "__make_property") return KW_MAKE_PROPERTY;
  if (name == "__make_seq") return KW_MAKE_SEQ;
  if (name == "mutable") return KW_MUTABLE;
  if (name == "namespace") return KW_NAMESPACE;
  if (name == "noexcept") return KW_NOEXCEPT;
  if (name == "new") return KW_NEW;
  if (name == "operator") return KW_OPERATOR;
  if (name == "private") return KW_PRIVATE;
  if (name == "protected") return KW_PROTECTED;
  if (name == "public") return KW_PUBLIC;
  if (name == "register") return KW_REGISTER;
  if (name == "return") return KW_RETURN;
  if (name == "short") return KW_SHORT;
  if (name == "signed") return KW_SIGNED;
  if (name == "sizeof") return KW_SIZEOF;
  if (name == "static") return KW_STATIC;
  if (name == "static_cast") return KW_STATIC_CAST;
  if (name == "struct") return KW_STRUCT;
  if (name == "template") return KW_TEMPLATE;
  if (name == "throw") return KW_THROW;
  if (name == "true") return KW_TRUE;
  if (name == "try") return KW_TRY;
  if (name == "typedef") return KW_TYPEDEF;
  if (name == "typename") return KW_TYPENAME;
  if (name == "union") return KW_UNION;
  if (name == "unsigned") return KW_UNSIGNED;
  if (name == "using") return KW_USING;
  if (name == "virtual") return KW_VIRTUAL;
  if (name == "void") return KW_VOID;
  if (name == "volatile") return KW_VOLATILE;
  if (name == "wchar_t") return KW_WCHAR_T;
  if (name == "while") return KW_WHILE;

  // These are alternative ways to refer to built-in operators.
  if (name == "and") return ANDAND;
  if (name == "and_eq") return ANDEQUAL;
  if (name == "bitand") return '&';
  if (name == "bitor") return '|';
  if (name == "compl") return '~';
  if (name == "not") return '!';
  if (name == "not_eq") return NECOMPARE;
  if (name == "or") return OROR;
  if (name == "or_eq") return OREQUAL;
  if (name == "xor") return '^';
  if (name == "xor_eq") return XOREQUAL;

  if (!cpp_longlong_keyword.empty() && name == cpp_longlong_keyword) {
    return KW_LONGLONG;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::scan_escape_sequence
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
scan_escape_sequence(int c) {
  if (c != '\\') {
    return c;
  }

  c = get();
  switch (c) {
  case 'a':
    return '\a';

  case 'b':
    return '\b';

  case 'f':
    return '\f';

  case 'n':
    return '\n';

  case 'r':
    return '\r';

  case 't':
    return '\t';

  case 'v':
    return '\v';

  case 'e':
    // \e is non-standard, buT GCC supports it.
    return '\x1B';

  case 'x':
    // hex character.
    c = get();
    if (isxdigit(c)) {
      int val = hex_val(c);
      c = get();
      if (isxdigit(c)) {
        val = (val << 4) | hex_val(c);
      } else {
        unget(c);
      }
      return val;
    }
    break;

  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
    // Octal character.
    {
      int val = (c - '0');
      c = get();
      if (c >= '0' && c <= '7') {
        val = (val << 3) | (c - '0');
        c = get();
        if (c >= '0' && c <= '7') {
          val = (val << 3) | (c - '0');
        } else {
          unget(c);
        }
      } else {
        unget(c);
      }
      return val;
    }
  }

  // Simply output the following character.
  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::scan_quoted
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
string CPPPreprocessor::
scan_quoted(int c) {
  int quote_mark = c;

  string str;
  c = get();
  while (c != EOF && c != '\n' && c != quote_mark) {
    if (c == '\\') {
      // Backslash means a special character follows.
      c = scan_escape_sequence(c);
    }

    str += c;
    c = get();
  }

  if (c != quote_mark) {
    warning("Unclosed string");
  }
  return str;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::should_ignore_manifest
//       Access: Public
//  Description: Returns true if the manifest is one that is being
//               ignored right now (presumably because we are
//               presently expanding it).
////////////////////////////////////////////////////////////////////
bool CPPPreprocessor::
should_ignore_manifest(const CPPManifest *manifest) const {
  Files::const_iterator fi;
  for (fi = _files.begin(); fi != _files.end(); ++fi) {
    if ((*fi)._ignore_manifest == manifest) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::should_ignore_preprocessor
//       Access: Public
//  Description: Returns true if we should ignore any preprocessor
//               directives (e.g. we're presently expanding a
//               manifest).
////////////////////////////////////////////////////////////////////
bool CPPPreprocessor::
should_ignore_preprocessor() const {
  Files::const_iterator fi;
  for (fi = _files.begin(); fi != _files.end(); ++fi) {
    if ((*fi)._ignore_manifest != NULL) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
get() {
  if (_unget != '\0') {
    int c = _unget;
    _unget = '\0';
    return c;
  }

  if (_files.empty()) {
    return EOF;
  }

  int c = _files.back().get();

  while (c == EOF && !_files.empty()) {
#ifdef CPP_VERBOSE_LEX
    indent(cerr, _files.size() * 2)
      << "End of input stream, restoring to previous input\n";
#endif
    int last_c = _files.back()._prev_last_c;
    _files.pop_back();

    if (last_c != '\0') {
      c = last_c;
    } else if (!_files.empty()) {
      c = _files.back().get();
    }
  }

  if (c == '\n') {
    _start_of_line = true;
  } else if (!isspace(c) && c != '#') {
    _start_of_line = false;
  }

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::unget
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
unget(int c) {
  assert(_unget == '\0');
  _unget = c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::nested_parse_template_instantiation
//       Access: Private
//  Description: Recursively invokes yacc to parse the stuff within
//               angle brackets that's the template instantiation part
//               of an identifier.  This involves setting and
//               restoring some state flags so we can return EOF when
//               we reach the closing bracket.
////////////////////////////////////////////////////////////////////
CPPTemplateParameterList *CPPPreprocessor::
nested_parse_template_instantiation(CPPTemplateScope *scope) {
#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "Beginning nested parse\n";
#endif
  assert(scope != NULL);

  State old_state = _state;
  int old_nesting = _paren_nesting;

  const CPPTemplateParameterList &formal_params = scope->_parameters;
  CPPTemplateParameterList::Parameters::const_iterator pi;
  _angle_bracket_found = false;

  CPPToken token = internal_get_next_token();
  if (token._token == '>') {
    _angle_bracket_found = true;
  } else {
    _saved_tokens.push_back(token);
  }

  CPPTemplateParameterList *actual_params = new CPPTemplateParameterList;

  for (pi = formal_params._parameters.begin();
       pi != formal_params._parameters.end() && !_angle_bracket_found;
       ++pi) {
    _state = S_nested;
    _paren_nesting = 0;

    CPPFile first_file = get_file();
    int first_line = get_line_number();
    int first_col = get_col_number();

    CPPDeclaration *decl = (*pi);
    if (decl->as_type()) {
      _saved_tokens.push_back(CPPToken(START_TYPE));
      CPPType *type = ::parse_type(this, current_scope, global_scope);
      if (type == NULL) {
        warning("Invalid type", first_line, first_col, first_file);
        skip_to_end_nested();
        type = CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_unknown));
      }
      actual_params->_parameters.push_back(type);
    } else {
      _saved_tokens.push_back(CPPToken(START_CONST_EXPR));
      CPPExpression *expr = parse_const_expr(this, current_scope, global_scope);
      if (expr == NULL) {
        warning("Invalid expression", first_line, first_col, first_file);
        skip_to_end_nested();
        expr = new CPPExpression(0);
      }
      actual_params->_parameters.push_back(expr);
    }
  }

  if (!_angle_bracket_found) {
    warning("Ignoring extra parameters in template instantiation");
    skip_to_angle_bracket();
  }

  _state = old_state;
  _paren_nesting = old_nesting;
  _angle_bracket_found = false;

#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "Ending nested parse\n";
#endif
  return actual_params;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::skip_to_end_nested
//       Access: Private
//  Description: This is an error-recovery function, called after
//               returning from a nested parse.  If the state is not
//               S_end_nested, there was an error in parsing the
//               nested tokens, and not all of the nested tokens may
//               have been consumed.  This function will consume the
//               rest of the nested tokens.
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
skip_to_end_nested() {
#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "Skipping tokens:\n";
#endif

  // Eat any eof tokens on the pushback stack.
  while (!_saved_tokens.empty() && _saved_tokens.back().is_eof()) {
    _saved_tokens.pop_back();
  }

  while (_state != S_end_nested && _state != S_eof) {
    get_next_token();
  }

#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "Done skipping tokens.\n";
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::skip_to_angle_bracket
//       Access: Private
//  Description: This is an error-recovery function, called after
//               returning from a nested parse.  If we haven't yet
//               consumed the closing angle bracket on the template
//               instantiation, keep consuming tokens until we do.
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
skip_to_angle_bracket() {
#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "Skipping tokens:\n";
#endif

  while (!_angle_bracket_found && _state != S_eof) {
    _state = S_nested;
    while (_state != S_end_nested && _state != S_eof) {
      get_next_token();
    }
  }

  // Eat any eof tokens on the pushback stack.
  while (!_saved_tokens.empty() && _saved_tokens.back().is_eof()) {
    _saved_tokens.pop_back();
  }

#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "Done skipping tokens.\n";
#endif
}
