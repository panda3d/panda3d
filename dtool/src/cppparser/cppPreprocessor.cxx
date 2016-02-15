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
#include "cppConstType.h"
#include "cppFunctionGroup.h"
#include "cppFunctionType.h"
#include "cppPointerType.h"
#include "cppParameterList.h"
#include "cppSimpleType.h"
#include "cppGlobals.h"
#include "cppCommentBlock.h"
#include "cppBison.h"
#include "indent.h"
#include "pstrtod.h"
#include "string_utils.h"

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
  _line_number = 0;
  _col_number = 0;
  _next_line_number = 1;
  _next_col_number = 1;
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
//  Description: Fetches a single character from the source file.
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::InputFile::
get() {
  assert(_in != NULL);

  if (!_lock_position) {
    _line_number = _next_line_number;
    _col_number = _next_col_number;
  }

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
      ++_next_line_number;
      _next_col_number = 1;
    }
    break;

  default:
    if (!_lock_position) {
      ++_next_col_number;
    }
  }

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::InputFile::peek
//       Access: Public
//  Description: Like get(), but does not advance the file pointer.
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::InputFile::
peek() {
  assert(_in != NULL);

  int c = _in->peek();

  // Quietly skip over embedded carriage-return characters.  We
  // shouldn't see any of these unless there was some DOS-to-Unix file
  // conversion problem.
  while (c == '\r') {
    _in->get();
    c = _in->peek();
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
  _parsing_template_params = false;
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
//  Description: Returns the line number of the last character
//               returned by get().
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
//  Description: Returns the column number of the last character
//               returned by get().
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

  YYLTYPE loc = token._lloc;

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

    CPPIdentifier *ident = new CPPIdentifier(name, loc);
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
      } else {
        error(string("unknown template '") + ident->get_fully_scoped_name() + "'", loc);
      }
    }

    while (token._token == SCOPE) {
      loc.last_line = token._lloc.last_line;
      loc.last_column = token._lloc.last_column;

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
          _last_token_loc = token._lloc;
          return token;
        }
        _saved_tokens.push_back(token);
        _last_token_loc = loc;
        return CPPToken(SCOPING, loc, name, result);
      }

      name += token._lval.str;
      ident->_names.push_back(token_prefix + token._lval.str);

      loc.last_line = token._lloc.last_line;
      loc.last_column = token._lloc.last_column;
      ident->_loc.last_line = loc.last_line;
      ident->_loc.last_column = loc.last_column;

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
        } else {
          error(string("unknown template '") + ident->get_fully_scoped_name() + "'", loc);
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

    _last_token_loc = loc;
    return CPPToken(token_type, loc, name, result);
  }

  // This is the normal case: just pass through whatever token we got.
  _last_token_loc = loc;
  return token;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::peek_next_token
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
peek_next_token() {
  CPPToken token(0);
  if (!_saved_tokens.empty()) {
    token = _saved_tokens.back();
  } else {
    token = internal_get_next_token();
    _saved_tokens.push_back(token);
  }
  return token;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::warning
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
warning(const string &message) {
  if (_verbose < 2) {
    return;
  }
  int line = get_line_number();
  int col = get_col_number();
  YYLTYPE loc;
  loc.first_line = line;
  loc.first_column = col;
  loc.last_line = line;
  loc.last_column = col;
  loc.file = get_file();
  warning(message, loc);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::warning
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
warning(const string &message, const YYLTYPE &loc) {
  if (_verbose >= 2) {
    if (_verbose >= 3) {
      indent(cerr, _files.size() * 2);
    }

    if (!loc.file.empty()) {
      cerr << loc.file << ':';
    }
    if (loc.first_line) {
      cerr << loc.first_line << ':';

      if (loc.first_column) {
        cerr << loc.first_column << ':';
      }
    }

    cerr << " warning: " << message << "\n";
    show_line(loc);
  }
  _warning_count++;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::error
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
error(const string &message) {
  int line = get_line_number();
  int col = get_col_number();
  YYLTYPE loc;
  loc.first_line = line;
  loc.first_column = col;
  loc.last_line = line;
  loc.last_column = col;
  loc.file = get_file();
  error(message, loc);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::error
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
error(const string &message, const YYLTYPE &loc) {
  if (_state == S_nested || _state == S_end_nested) {
    // Don't report or log errors in the nested state.  These will be
    // reported when the nesting level collapses.
    return;
  }

  if (_verbose >= 1) {
    if (_verbose >= 3) {
      indent(cerr, _files.size() * 2);
    }

    if (!loc.file.empty()) {
      cerr << loc.file << ':';
    }
    if (loc.first_line) {
      cerr << loc.first_line << ':';

      if (loc.first_column) {
        cerr << loc.first_column << ':';
      }
    }

    cerr << " error: " << message << "\n";
    show_line(loc);

    if (_error_abort) {
      cerr << "Aborting.\n";
      abort();
    }
  }
  _error_count++;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::show_line
//       Access: Public
//  Description: Shows the indicated line, useful for error messages.
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
show_line(const YYLTYPE &loc) {
  if (loc.file._filename.empty()) {
    return;
  }

  int indent_level = 0;
  if (_verbose >= 3) {
    indent_level = _files.size() * 2;
  }

  // Seek to the offending line in the file.
  ifstream stream;
  if (loc.file._filename.open_read(stream)) {
    int l = 0;
    string linestr;
    while (l < loc.first_line) {
      getline(stream, linestr);
      ++l;
    }

    // Strip off trailing whitespace.
    size_t last = linestr.length();
    while (isspace(linestr[--last])) {
      linestr = linestr.substr(0, last);
    }

    indent(cerr, indent_level) << linestr << "\n";

    // Point the user at the offending column.
    if (loc.first_column) {
      int last_column;
      if (loc.first_line == loc.last_line && loc.last_column) {
        last_column = loc.last_column;
      } else {
        last_column = linestr.length();
      }

      indent(cerr, indent_level);
      int i = 0;
      for (; i < loc.first_column - 1; ++i) {
        cerr.put(' ');
      }
      cerr.put('^');
      while (++i < last_column) {
        cerr.put('~');
      }
      cerr << "\n";
    }
  }
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
  assert(_last_c == 0);

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
expand_manifests(const string &input_expr, bool expand_undefined,
                 const YYLTYPE &loc) {
  // Get a copy of the expression string we can modify.
  string expr = input_expr;

  // Repeatedly scan the expr for any manifest names or defined()
  // function.

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
            expand_manifest_inline(expr, q, p, (*mi).second);
            manifest_found = true;

          } else if (expand_undefined && ident != "true" && ident != "false") {
            // It is not found.  Expand it to 0, but only if we are currently
            // parsing an #if expression.
            expr = expr.substr(0, q) + "0" + expr.substr(p);
            p = q + 1;
          }
        }
      } else if (expr[p] == '\'' || expr[p] == '"') {
        // Skip the next part until we find a closing quotation mark.
        char quote = expr[p];
        p++;
        while (p < expr.size() && expr[p] != quote) {
          if (expr[p] == '\\') {
            // This might be an escaped quote.  Skip an extra char.
            p++;
          }
          p++;
        }
        if (p >= expr.size()) {
          // Unclosed string.
          warning("missing terminating " + string(1, quote) + " character", loc);
        }
        p++;
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
           CPPScope *global_scope, const YYLTYPE &loc) {
  string expr = expand_manifests(input_expr, false, loc);

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

  if (c == EOF) {
    _state = S_eof;
    return CPPToken::eof();
  }

  // Check for a number beginning with a decimal point.
  int next_c = peek();
  if (c == '.' && isdigit(next_c)) {
    return get_number(c);
  }

  YYLTYPE loc;
  loc.file = get_file();
  loc.first_line = get_line_number();
  loc.first_column = get_col_number();
  loc.last_line = loc.first_line;
  loc.last_column = loc.first_column;

  // Check for two- or three-character tokens.
  int di = check_digraph(c);
  if (di != 0) {
    c = di;
    ++loc.last_column;
    get();

    int tri = check_trigraph(di);
    if (tri != 0) {
      ++loc.last_column;
      get();
      return CPPToken(tri, loc);
    }
    return CPPToken(di, loc);
  }

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
        _parsing_template_params = false;
        _state = S_end_nested;
        return CPPToken::eof();
      }
    }
  }

  // Look for an end-of-line comment, and parse it before we finish
  // this token.  This is not strictly necessary, but it allows us to
  // pick up docstrings from comments after enum values.
  while (next_c != EOF && isspace(next_c)) {
    get();
    next_c = peek();
  }
  if (next_c == '/') {
    _last_c = skip_whitespace(get());
  }

  return CPPToken(c, loc);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::check_digraph
//       Access: Private
//  Description: Checks the next character in the stream to see if
//               this might be a two-character token.
//               Returns 0 if it is only a single-character token.
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
check_digraph(int c) {
  int next_c = peek();
  switch (c) {
  case '+':
    if (next_c == '+') return PLUSPLUS;
    if (next_c == '=') return PLUSEQUAL;
    break;

  case '-':
    if (next_c == '-') return MINUSMINUS;
    if (next_c == '=') return MINUSEQUAL;
    if (next_c == '>') return POINTSAT;
    break;

  case '<':
    if (next_c == '<') return LSHIFT;
    if (next_c == '=') return LECOMPARE;
    if (next_c == ':') return '[';
    if (next_c == '%') return '{';
    break;

  case '>':
    if (_parsing_template_params && _paren_nesting <= 0) {
      // Don't parse >> as right-shift when parsing a template list, as
      // per C++11, to allow a syntax like A<B>>.
      // However, nested >> must be preserved, such as in A<(2>>1)>
      break;
    }
    if (next_c == '>') return RSHIFT;
    if (next_c == '=') return GECOMPARE;
    break;

  case '|':
    if (next_c == '|') return OROR;
    if (next_c == '=') return OREQUAL;
    break;

  case '&':
    if (next_c == '&') return ANDAND;
    if (next_c == '=') return ANDEQUAL;
    break;

  case '^':
    if (next_c == '=') return XOREQUAL;
    break;

  case '=':
    if (next_c == '=') return EQCOMPARE;
    break;

  case '!':
    if (next_c == '=') return NECOMPARE;
    break;

  case '.':
    if (next_c == '*') return DOT_STAR;
    if (next_c == '.') {
      get();
      if (peek() == '.') {
        return ELLIPSIS;
      } else {
        unget('.');
      }
    }
    break;

  case ':':
    if (next_c == ':') return SCOPE;
    if (next_c == '>') return ']';
    break;

  case '*':
    if (next_c == '=') return TIMESEQUAL;
    break;

  case '/':
    if (next_c == '=') return DIVIDEEQUAL;
    break;

  case '%':
    if (next_c == '=') return MODEQUAL;
    if (next_c == '>') return '}';
    break;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::check_trigraph
//       Access: Private
//  Description: Checks the next character in the stream to see if
//               this might be a three-character token; usually
//               called in conjunction with check_digraph.
//               Returns 0 if it is not a three-character token.
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
check_trigraph(int c) {
  int next_c = peek();
  switch (c) {
  case POINTSAT:
    if (next_c == '*') return POINTSAT_STAR;
    break;

  case LSHIFT:
    if (next_c == '=') return LSHIFTEQUAL;
    break;

  case RSHIFT:
    if (next_c == '=') return RSHIFTEQUAL;
    break;
  }

  return 0;
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
      if (peek() != '\n') {
        return '\\';
      }
      c = get();
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
  while (c == '/') {
    int next_c = peek();
    if (next_c == '*') {
      get();
      _last_cpp_comment = false;
      c = skip_c_comment(get());
    } else if (next_c == '/') {
      get();
      c = skip_cpp_comment(get());
      break;
    } else {
      _last_cpp_comment = false;
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
  YYLTYPE loc;
  loc.file = get_file();
  loc.first_line = get_line_number();
  loc.first_column = get_col_number() - 2;
  loc.last_line = 0;
  loc.last_column = 0;

  if (_save_comments) {
    CPPCommentBlock *comment = new CPPCommentBlock;
    _comments.push_back(comment);

    comment->_file = loc.file;
    comment->_line_number = loc.first_line;
    comment->_last_line = loc.last_line;
    comment->_col_number = loc.first_column;
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

    loc.last_line = get_line_number();
    comment->_last_line = loc.last_line;

    warning("Comment is unterminated", loc);

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

    loc.last_line = get_line_number();

    warning("Comment is unterminated", loc);
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
//     Function: CPPPreprocessor::skip_digit_separator
//       Access: Private
//  Description: Skips a C++14 digit separator that has just been
//               found through peek().
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
skip_digit_separator(int c) {
  if (c != '\'') {
    return c;
  }

  get();
  c = peek();

  if (isdigit(c)) {
    return c;
  }

  YYLTYPE loc;
  loc.file = get_file();
  loc.first_line = get_line_number();
  loc.first_column = get_col_number();
  loc.last_line = loc.first_line;
  loc.last_column = loc.first_column;

  if (c != '\'') {
    // This assumes that this isn't a character constant directly follows
    // a digit sequence, like 123'a' -- I can't think of a situation
    // where that's legal anyway, though.
    error("digit separator cannot occur at end of digit sequence", loc);
    return c;
  }

  while (c == '\'') {
    get();
    ++loc.last_column;
    c = peek();
  }
  error("adjacent digit separators", loc);

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::process_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
process_directive(int c) {
  assert(c == '#');
  c = skip_whitespace(get());

  int begin_line = get_line_number();
  int begin_column = get_col_number();

  string command, args;
  c = get_preprocessor_command(c, command);

  YYLTYPE loc;
  loc.file = get_file();
  loc.first_line = get_line_number();
  loc.first_column = get_col_number();

  c = get_preprocessor_args(c, args);

  loc.last_line = get_line_number();
  loc.last_column = 0;

#ifdef CPP_VERBOSE_LEX
  indent(cerr, _files.size() * 2)
    << "#" << command << " " << args << "\n";
#endif

  if (command == "define") {
    handle_define_directive(args, loc);
  } else if (command == "undef") {
    handle_undef_directive(args, loc);
  } else if (command == "ifdef") {
    handle_ifdef_directive(args, loc);
  } else if (command == "ifndef") {
    handle_ifndef_directive(args, loc);
  } else if (command == "if") {
    handle_if_directive(args, loc);
  } else if (command == "else" || command == "elif") {
    // Presumably this follows some #if or #ifdef.  We don't bother to
    // check this, however.
    skip_false_if_block(false);
  } else if (command == "endif") {
    // Presumably this follows some #if or #ifdef.  We don't bother to
    // check this, however.
  } else if (command == "include") {
    handle_include_directive(args, loc);
  } else if (command == "pragma") {
    handle_pragma_directive(args, loc);
  } else if (command == "ident") {
    // Quietly ignore idents.
  } else if (command == "error") {
    handle_error_directive(args, loc);
  } else {
    loc.first_line = begin_line;
    loc.first_column = begin_column;
    loc.last_line = begin_line;
    loc.last_column = begin_column + command.size() - 1;
    warning("Ignoring unknown directive #" + command, loc);
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
  // The next sequence of characters is the command.
  while (c != EOF && (isalnum(c) || c == '_')) {
    command += c;
    c = get();
  }

  while (c != EOF && c != '\n' && isspace(c)) {
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
handle_define_directive(const string &args, const YYLTYPE &loc) {
  if (args.empty()) {
    warning("Ignoring empty #define directive", loc);
  } else {
    CPPManifest *manifest = new CPPManifest(args, loc);
    manifest->_vis = preprocessor_vis;
    if (!manifest->_has_parameters) {
      string expr_string = manifest->expand();
      if (!expr_string.empty()) {
        manifest->_expr = parse_expr(expr_string, global_scope, global_scope, loc);
      }
    }

    pair<Manifests::iterator, bool> result =
      _manifests.insert(Manifests::value_type(manifest->_name, manifest));

    if (!result.second) {
      // There was already a macro with this name.  Delete the old.
      CPPManifest *other = result.first->second;
      warning("redefinition of macro '" + manifest->_name + "'", loc);
      warning("previous definition is here", other->_loc);
      delete other;
      result.first->second = manifest;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_undef_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_undef_directive(const string &args, const YYLTYPE &loc) {
  if (args.empty()) {
    warning("Ignoring empty #undef directive", loc);
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
handle_ifdef_directive(const string &args, const YYLTYPE &loc) {
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
handle_ifndef_directive(const string &args, const YYLTYPE &loc) {
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
handle_if_directive(const string &args, const YYLTYPE &loc) {
  // When expanding manifests, we should replace unknown macros
  // with 0.
  string expr = expand_manifests(args, true, loc);

  int expression_result = 0;
  CPPExpressionParser ep(current_scope, global_scope);
  ep._verbose = 0;
  if (ep.parse_expr(expr, *this)) {
    CPPExpression::Result result = ep._expr->evaluate();
    if (result._type == CPPExpression::RT_error) {
      ostringstream strm;
      strm << *ep._expr;
      warning("Ignoring invalid expression " + strm.str(), loc);
    } else {
      expression_result = result.as_integer();
    }
  } else {
    warning("Ignoring invalid expression " + args, loc);
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
handle_include_directive(const string &args, const YYLTYPE &loc) {
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
    expr = expand_manifests(expr, false, loc);
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
      for (size_t dir=0; dir<_quote_include_path.get_num_directories(); dir++) {
        Filename match(_quote_include_path.get_directory(dir), filename);
        if (match.exists()) {
          filename = match;
          found_file = true;
          source = _quote_include_kind[dir];
        }
      }
    }

    if (!found_file) {
      warning("Cannot find " + filename.get_fullpath(), loc);
    } else {
      _last_c = '\0';

      // If it was explicitly named on the command-line, mark it S_local.
      filename.make_absolute();
      if (_explicit_files.count(filename)) {
        source = CPPFile::S_local;
      }

      CPPFile file(filename, filename_as_referenced, source);

      // Don't include it if we included it before and it had #pragma once.
      ParsedFiles::const_iterator it = _parsed_files.find(file);
      if (it != _parsed_files.end() && it->_pragma_once) {
        return;
      }

      if (!push_file(file)) {
        warning("Unable to read " + filename.get_fullpath(), loc);
      }
    }
  } else {
    warning("Ignoring invalid #include directive", loc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::handle_pragma_directive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPreprocessor::
handle_pragma_directive(const string &args, const YYLTYPE &loc) {
  if (args == "once") {
    ParsedFiles::iterator it = _parsed_files.find(loc.file);
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
handle_error_directive(const string &args, const YYLTYPE &loc) {
  error(args, loc);
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
      c = skip_whitespace(get());

      YYLTYPE loc;
      loc.file = get_file();
      loc.first_line = get_line_number();
      loc.first_column = get_col_number();
      loc.last_line = loc.first_line;
      loc.last_column = loc.first_column;

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
          handle_if_directive(args, loc);
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
  YYLTYPE loc;
  loc.file = get_file();
  loc.first_line = get_line_number();
  loc.first_column = get_col_number();

  string str = scan_quoted(c);
  YYSTYPE result;
  if (!str.empty()) {
    result.u.integer = (int)str[0];
  } else {
    result.u.integer = 0;
  }

  return get_literal(CHAR_TOK, loc, str, result);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_quoted_string
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
get_quoted_string(int c) {
  YYLTYPE loc;
  loc.file = get_file();
  loc.first_line = get_line_number();
  loc.first_column = get_col_number();

  string str = scan_quoted(c);

  return get_literal(SIMPLE_STRING, loc, str);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_identifier
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
get_identifier(int c) {
  YYLTYPE loc;
  loc.file = get_file();
  loc.first_line = get_line_number();
  loc.first_column = get_col_number();
  loc.last_line = loc.first_line;
  loc.last_column = loc.first_column;

  string name(1, (char)c);

  c = peek();
  while (c != EOF && (isalnum(c) || c == '_')) {
    name += get();
    c = peek();
  }

  loc.last_line = get_line_number();
  loc.last_column = get_col_number();

  if ((c == '\'' || c == '"') &&
      (name == "L" || name == "u8" ||
       name == "u" || name == "U")) {
    // This is actually a wide-character or wide-string literal or
    // some such.  Figure out the correct character type to use.

    CPPExpression::Type type;
    if (name == "L") {
      type = CPPExpression::T_wstring;
    } else if (name == "u8") {
      type = CPPExpression::T_u8string;
    } else if (name == "u") {
      type = CPPExpression::T_u16string;
    } else if (name == "U") {
      type = CPPExpression::T_u32string;
    }

    get();
    string str = scan_quoted(c);

    loc.last_line = get_line_number();
    loc.last_column = get_col_number();

    YYSTYPE result;
    if (c == '\'') {
      // We don't really care about the type for now.
      if (!str.empty()) {
        result.u.integer = (int)str[0];
      } else {
        result.u.integer = 0;
      }
      return get_literal(CHAR_TOK, loc, str, result);
    } else {
      result.u.expr = new CPPExpression(str);
      result.u.expr->_type = type;
      return get_literal(STRING_LITERAL, loc, str, result);
    }
  }

  _last_c = 0;

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
    return CPPToken(kw, loc, name, result);
  }

  return CPPToken(SIMPLE_IDENTIFIER, loc, name);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::get_literal
//       Access: Private
//  Description: Under the assumption that we've just parsed a
//               string or real constant, parse a following custom
//               literal, and returns a token for it.
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
get_literal(int token, YYLTYPE loc, const string &str, const YYSTYPE &value) {
  string suffix;

  int c = peek();
  if (isalpha(c) || c == '_') {
    // A literal seems to be following directly.
    while (c != EOF && (isalnum(c) || c == '_')) {
      suffix += get();
      c = peek();
    }
  }
  loc.last_line = get_line_number();
  loc.last_column = get_col_number();

  if (suffix.empty()) {
    // There is no suffix.
    return CPPToken(token, loc, str, value);
  }

  // Handle built-in literal suffixes.
  if (token == INTEGER) {
    if (cmp_nocase(suffix, "u") == 0 ||
        cmp_nocase(suffix, "l") == 0 ||
        cmp_nocase(suffix, "ul") == 0 || cmp_nocase(suffix, "lu") == 0 ||
        cmp_nocase(suffix, "ll") == 0 ||
        cmp_nocase(suffix, "ull") == 0 || cmp_nocase(suffix, "llu") == 0) {
      // These are built-in integer suffixes.  Right now, we don't try to
      // distinguish between them.
      return CPPToken(INTEGER, loc, str, value);
    }
  } else if (token == REAL) {
    if (suffix == "f" || suffix == "F" ||
        suffix == "l" || suffix == "L") {
      return CPPToken(REAL, loc, str, value);
    }
  }

  // Find the literal operator for this literal.
  CPPIdentifier *ident = new CPPIdentifier("operator \"\" " + suffix);
  CPPDeclaration *decl = ident->find_symbol(current_scope, global_scope, this);

  if (decl == NULL || decl->get_subtype() != CPPDeclaration::ST_function_group) {
    error("unknown literal suffix " + suffix, loc);
    return CPPToken(token, loc, str, value);
  }

  // Find the overload with the appropriate signature.
  CPPExpression *expr = NULL;
  CPPInstance *instance = NULL;
  CPPInstance *raw_instance = NULL;
  CPPFunctionGroup *fgroup = decl->as_function_group();
  CPPFunctionGroup::Instances::iterator it;
  for (it = fgroup->_instances.begin(); it != fgroup->_instances.end(); ++it) {
    if ((*it)->_type == NULL) {
      continue;
    }

    CPPFunctionType *ftype = (*it)->_type->as_function_type();
    if (ftype == NULL || ftype->_parameters == NULL) {
      continue;
    }

    CPPParameterList::Parameters &params = ftype->_parameters->_parameters;
    if (token == STRING_LITERAL || token == SIMPLE_STRING) {
      // A custom string literal must take a second size_t argument.
      if (params.size() != 2) continue;
    } else {
      if (params.size() != 1) continue;
    }

    CPPInstance *param = params[0];
    if (param == NULL || param->_type == NULL) {
      continue;
    }

    CPPType *type = param->_type;
    while (type->get_subtype() == CPPDeclaration::ST_const) {
      type = type->as_const_type()->_wrapped_around;
    }
    if (type->get_subtype() == CPPDeclaration::ST_simple) {
      // It's a primitive type.  Check that it matches the appropriate token.
      CPPSimpleType::Type simple = type->as_simple_type()->_type;

      if (token == INTEGER && simple == CPPSimpleType::T_int) {
        expr = new CPPExpression(value.u.integer);
        instance = (*it);
        break;
      } else if (token == REAL && simple == CPPSimpleType::T_double) {
        expr = new CPPExpression(value.u.real);
        instance = (*it);
        break;
      } else if (token == CHAR_TOK && (simple == CPPSimpleType::T_char ||
                                       simple == CPPSimpleType::T_wchar_t ||
                                       simple == CPPSimpleType::T_char16_t ||
                                       simple == CPPSimpleType::T_char32_t)) {
        // We currently don't have the means to check the exact character type.
        expr = new CPPExpression(value.u.integer);
        instance = (*it);
        break;
      }

    } else if (type->get_subtype() == CPPDeclaration::ST_pointer) {
      // Must be a const pointer.  Unwrap it.
      type = type->as_pointer_type()->_pointing_at;
      if (type == NULL || type->get_subtype() != CPPDeclaration::ST_const) {
        continue;
      }
      type = type->as_const_type()->_wrapped_around;
      if (type == NULL || type->get_subtype() != CPPDeclaration::ST_simple) {
        continue;
      }

      CPPSimpleType::Type simple = type->as_simple_type()->_type;
      if (simple == CPPSimpleType::T_char && params.size() == 1) {
        // This is the raw literal operator.  Store it, but don't break;
        // a non-raw version of the operator might follow, which we'd prefer.
        raw_instance = (*it);

      } else if (token == SIMPLE_STRING && simple == CPPSimpleType::T_char) {
        expr = new CPPExpression(str);
        instance = (*it);
        break;

      } else if (token == STRING_LITERAL) {
        // Verify that the character type of the string literal matches
        // the character type of the parameter.
        CPPExpression::Type str_type = value.u.expr->_type;
        if ((str_type == CPPExpression::T_string && simple == CPPSimpleType::T_char) ||
            (str_type == CPPExpression::T_wstring && simple == CPPSimpleType::T_wchar_t) ||
            (str_type == CPPExpression::T_u8string && simple == CPPSimpleType::T_char) ||
            (str_type == CPPExpression::T_u16string && simple == CPPSimpleType::T_char16_t) ||
            (str_type == CPPExpression::T_u32string && simple == CPPSimpleType::T_char32_t)) {
          expr = value.u.expr;
          instance = (*it);
          break;
        }
      }
    }
  }

  YYSTYPE result;
  if (instance != NULL) {
    result.u.expr = new CPPExpression(CPPExpression::literal(expr, instance));
    return CPPToken(CUSTOM_LITERAL, loc, str, result);
  }

  if ((token == REAL || token == INTEGER) && raw_instance != NULL) {
    // For numeric constants, we can fall back to a raw literal operator.
    result.u.expr = new CPPExpression(CPPExpression::raw_literal(str, instance));
    return CPPToken(CUSTOM_LITERAL, loc, str, result);
  }

  error(fgroup->_name + " has no suitable overload for literal of this type", loc);
  result.u.expr = NULL;
  return CPPToken(CUSTOM_LITERAL, loc, str, result);
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
  if (c == 0) {
    c = get();
  }
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

  YYLTYPE loc;
  loc.first_line = first_line;
  loc.first_column = first_col;
  loc.last_line = first_line;
  loc.last_column = first_col;
  loc.file = first_file;

  if ((int)args.size() < num_args) {
    warning("Not enough arguments for manifest " + name, loc);

  } else if (va_arg < 0 && (int)args.size() > num_args) {
    warning("Too many arguments for manifest " + name, loc);
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
//  Description: Assuming that we've just read a digit or a period
//               indicating the start of a number, read the rest.
////////////////////////////////////////////////////////////////////
CPPToken CPPPreprocessor::
get_number(int c) {
  YYLTYPE loc;
  loc.file = get_file();
  loc.first_line = get_line_number();
  loc.first_column = get_col_number();
  loc.last_line = loc.first_line;
  loc.last_column = loc.first_column;

  string num(1, (char)c);
  bool leading_zero = (c == '0');
  bool decimal_point = (c == '.');

  c = skip_digit_separator(peek());

  if (leading_zero && (c == 'x' || c == 'X')) {
    // Here we have a hex number.
    num += get();
    c = peek();

    while (c != EOF && (isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f'))) {
      num += get();
      c = skip_digit_separator(peek());
    }

    loc.last_line = get_line_number();
    loc.last_column = get_col_number();

    YYSTYPE result;
    result.u.integer = strtol(num.c_str(), (char **)NULL, 16);

    return get_literal(INTEGER, loc, num, result);

  } else if (leading_zero && (c == 'b' || c == 'B')) {
    // A C++14-style binary number.
    get();
    c = peek();
    string bin(1, (char)c);

    while (c != EOF && (c == '0' || c == '1')) {
      bin += get();
      c = skip_digit_separator(peek());
    }

    loc.last_line = get_line_number();
    loc.last_column = get_col_number();

    YYSTYPE result;
    result.u.integer = strtol(bin.c_str(), (char **)NULL, 2);

    return get_literal(INTEGER, loc, bin, result);
  }

  while (c != EOF && isdigit(c)) {
    num += get();
    c = skip_digit_separator(peek());
  }

  if (c == '.' && !decimal_point) {
    // Now we have a floating-point number.
    decimal_point = true;
    num += get();
    c = peek();

    while (c != EOF && isdigit(c)) {
      num += get();
      c = peek();
    }
  }

  if (decimal_point || c == 'e' || c == 'E') {
    if (tolower(c) == 'e') {
      // An exponent is allowed.
      num += get();
      c = peek();
      if (c == '-' || c == '+') {
        num += get();
        c = peek();
      }
      while (c != EOF && isdigit(c)) {
        num += get();
        c = skip_digit_separator(peek());
      }
    }

    loc.last_line = get_line_number();
    loc.last_column = get_col_number();

    YYSTYPE result;
    result.u.real = pstrtod(num.c_str(), (char **)NULL);

    return get_literal(REAL, loc, num, result);
  }

  // This is a decimal or octal integer number.

  loc.last_line = get_line_number();
  loc.last_column = get_col_number();

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

  return get_literal(INTEGER, loc, num, result);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::check_keyword
//       Access: Private, Static
//  Description:
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
check_keyword(const string &name) {
  if (name == "alignas") return KW_ALIGNAS;
  if (name == "alignof") return KW_ALIGNOF;
  if (name == "__alignof") return KW_ALIGNOF;
  if (name == "__alignof__") return KW_ALIGNOF;
  if (name == "auto") return KW_AUTO;
  if (name == "__begin_publish") return KW_BEGIN_PUBLISH;
  if (name == "__blocking") return KW_BLOCKING;
  if (name == "bool") return KW_BOOL;
  if (name == "catch") return KW_CATCH;
  if (name == "char") return KW_CHAR;
  if (name == "char16_t") return KW_CHAR16_T;
  if (name == "char32_t") return KW_CHAR32_T;
  if (name == "class") return KW_CLASS;
  if (name == "const") return KW_CONST;
  if (name == "__const") return KW_CONST;
  if (name == "__const__") return KW_CONST;
  if (name == "constexpr") return KW_CONSTEXPR;
  if (name == "const_cast") return KW_CONST_CAST;
  if (name == "decltype") return KW_DECLTYPE;
  if (name == "default") return KW_DEFAULT;
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
  if (name == "final") return KW_FINAL;
  if (name == "float") return KW_FLOAT;
  if (name == "friend") return KW_FRIEND;
  if (name == "for") return KW_FOR;
  if (name == "goto") return KW_GOTO;
  if (name == "if") return KW_IF;
  if (name == "inline") return KW_INLINE;
  if (name == "__inline") return KW_INLINE;
  if (name == "__inline__") return KW_INLINE;
  if (name == "int") return KW_INT;
  if (name == "long") return KW_LONG;
  if (name == "__make_property") return KW_MAKE_PROPERTY;
  if (name == "__make_property2") return KW_MAKE_PROPERTY2;
  if (name == "__make_seq") return KW_MAKE_SEQ;
  if (name == "mutable") return KW_MUTABLE;
  if (name == "namespace") return KW_NAMESPACE;
  if (name == "noexcept") return KW_NOEXCEPT;
  if (name == "nullptr") return KW_NULLPTR;
  if (name == "new") return KW_NEW;
  if (name == "operator") return KW_OPERATOR;
  if (name == "override") return KW_OVERRIDE;
  if (name == "private") return KW_PRIVATE;
  if (name == "protected") return KW_PROTECTED;
  if (name == "public") return KW_PUBLIC;
  if (name == "register") return KW_REGISTER;
  if (name == "reinterpret_cast") return KW_REINTERPRET_CAST;
  if (name == "return") return KW_RETURN;
  if (name == "short") return KW_SHORT;
  if (name == "signed") return KW_SIGNED;
  if (name == "sizeof") return KW_SIZEOF;
  if (name == "static") return KW_STATIC;
  if (name == "static_assert") return KW_STATIC_ASSERT;
  if (name == "static_cast") return KW_STATIC_CAST;
  if (name == "struct") return KW_STRUCT;
  if (name == "template") return KW_TEMPLATE;
  if (name == "thread_local") return KW_THREAD_LOCAL;
  if (name == "throw") return KW_THROW;
  if (name == "true") return KW_TRUE;
  if (name == "try") return KW_TRY;
  if (name == "typedef") return KW_TYPEDEF;
  if (name == "typeid") return KW_TYPEID;
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
    // \e is non-standard, but GCC supports it.
    return '\x1B';

  case 'x':
    // hex character.
    c = get();
    if (isxdigit(c)) {
      int val = hex_val(c);
      if (isxdigit(peek())) {
        val = (val << 4) | hex_val(get());
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
      c = peek();
      if (c >= '0' && c <= '7') {
        val = (val << 3) | (get() - '0');
        c = peek();
        if (c >= '0' && c <= '7') {
          val = (val << 3) | (get() - '0');
        }
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
    _files.pop_back();

    // Synthesize a newline, just in case the file doesn't already
    // end with one.
    c = '\n';
  }

  if (c == '\n') {
    _start_of_line = true;
  } else if (!isspace(c) && c != '#') {
    _start_of_line = false;
  }

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::peek
//       Access: Private
//  Description: Like get(), but does not alter the current state.
////////////////////////////////////////////////////////////////////
int CPPPreprocessor::
peek() {
  if (_unget != '\0') {
    return _unget;
  }

  if (_files.empty()) {
    return EOF;
  }

  Files::reverse_iterator it = _files.rbegin();
  int c = (*it).peek();

  while (c == EOF && it != _files.rend()) {
    int last_c = (*it)._prev_last_c;
    ++it;

    if (last_c != '\0') {
      c = last_c;
    } else if (it != _files.rend()) {
      c = (*it).peek();
    }
  }

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPreprocessor::unget
//       Access: Private
//  Description: Undoes the effects of a previous get().  Not
//               recommended, use peek() instead where possible, as
//               it doesn't cause the column index to be off.
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
  bool old_parsing_params = _parsing_template_params;

  const CPPTemplateParameterList &formal_params = scope->_parameters;
  CPPTemplateParameterList::Parameters::const_iterator pi;

  _state = S_nested;
  _paren_nesting = 0;
  _parsing_template_params = true;

  CPPToken token = internal_get_next_token();
  if (token._token == '>') {
    _parsing_template_params = false;
  } else {
    _saved_tokens.push_back(token);
  }

  CPPTemplateParameterList *actual_params = new CPPTemplateParameterList;

  for (pi = formal_params._parameters.begin();
       pi != formal_params._parameters.end() && _parsing_template_params;
       ++pi) {
    CPPToken token = peek_next_token();
    YYLTYPE loc = token._lloc;

    CPPDeclaration *decl = (*pi);
    if (decl->as_type()) {
      // Parse a typename template parameter.
      _saved_tokens.push_back(CPPToken(START_TYPE));
      CPPType *type = ::parse_type(this, current_scope, global_scope);
      if (type == NULL) {
        loc.last_line = get_line_number();
        loc.last_column = get_col_number() - 1;
        warning("Invalid type", loc);
        skip_to_end_nested();
        type = CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_unknown));
      }
      actual_params->_parameters.push_back(type);
    } else {
      // Parse a constant expression template parameter.
      _saved_tokens.push_back(CPPToken(START_CONST_EXPR));
      CPPExpression *expr = parse_const_expr(this, current_scope, global_scope);
      if (expr == NULL) {
        loc.last_line = get_line_number();
        loc.last_column = get_col_number() - 1;
        warning("Invalid expression", loc);
        skip_to_end_nested();
        expr = new CPPExpression(0);
      }
      actual_params->_parameters.push_back(expr);
    }

    _state = S_nested;
    _paren_nesting = 0;
  }

  if (_parsing_template_params) {
    warning("Ignoring extra parameters in template instantiation");
    skip_to_angle_bracket();
  }

  _state = old_state;
  _paren_nesting = old_nesting;
  _parsing_template_params = old_parsing_params;

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

  while (_parsing_template_params && _state != S_eof) {
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
