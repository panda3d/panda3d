/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppPreprocessor.cxx
 * @author drose
 * @date 1999-10-22
 */

#include "cppPreprocessor.h"
#include "cppExpressionParser.h"
#include "cppExpression.h"
#include "cppScope.h"
#include "cppIdentifier.h"
#include "cppTemplateScope.h"
#include "cppTemplateParameterList.h"
#include "cppClassTemplateParameter.h"
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
#include <set>

using std::cerr;
using std::string;

// We manage our own visibility counter, in addition to that managed by
// cppBison.y.  We do this just so we can define manifests with the correct
// visibility when they are declared.  (Asking the parser for the current
// visibility is prone to error, since the parser might be several tokens
// behind the preprocessor.)
static CPPVisibility preprocessor_vis = V_public;

// Don't forget to update CPPToken::output() when adding entries.
static const std::unordered_map<std::string, int> keywords = {
  {"alignas", KW_ALIGNAS},
  {"alignof", KW_ALIGNOF},
  {"__alignof", KW_ALIGNOF},
  {"__alignof__", KW_ALIGNOF},
  {"auto", KW_AUTO},
  {"__begin_publish", KW_BEGIN_PUBLISH},
  {"__blocking", KW_BLOCKING},
  {"bool", KW_BOOL},
  {"__builtin_va_list", KW_BUILTIN_VA_LIST},
  {"catch", KW_CATCH},
  {"char", KW_CHAR},
  {"char8_t", KW_CHAR8_T},
  {"char16_t", KW_CHAR16_T},
  {"char32_t", KW_CHAR32_T},
  {"class", KW_CLASS},
  {"const", KW_CONST},
  {"__const", KW_CONST},
  {"__const__", KW_CONST},
  {"consteval", KW_CONSTEVAL},
  {"constexpr", KW_CONSTEXPR},
  {"constinit", KW_CONSTINIT},
  {"const_cast", KW_CONST_CAST},
  {"decltype", KW_DECLTYPE},
  {"default", KW_DEFAULT},
  {"delete", KW_DELETE},
  {"double", KW_DOUBLE},
  {"dynamic_cast", KW_DYNAMIC_CAST},
  {"else", KW_ELSE},
  {"__end_publish", KW_END_PUBLISH},
  {"enum", KW_ENUM},
  {"extern", KW_EXTERN},
  {"__extension", KW_EXTENSION},
  {"explicit", KW_EXPLICIT},
  {"__published", KW_PUBLISHED},
  {"false", KW_FALSE},
  {"final", KW_FINAL},
  {"float", KW_FLOAT},
  {"friend", KW_FRIEND},
  {"for", KW_FOR},
  {"goto", KW_GOTO},
  {"__has_virtual_destructor", KW_HAS_VIRTUAL_DESTRUCTOR},
  {"if", KW_IF},
  {"inline", KW_INLINE},
  {"__inline", KW_INLINE},
  {"__inline__", KW_INLINE},
  {"int", KW_INT},
  {"__is_abstract", KW_IS_ABSTRACT},
  {"__is_base_of", KW_IS_BASE_OF},
  {"__is_class", KW_IS_CLASS},
  {"__is_constructible", KW_IS_CONSTRUCTIBLE},
  {"__is_convertible_to", KW_IS_CONVERTIBLE_TO},
  {"__is_destructible", KW_IS_DESTRUCTIBLE},
  {"__is_empty", KW_IS_EMPTY},
  {"__is_enum", KW_IS_ENUM},
  {"__is_final", KW_IS_FINAL},
  {"__is_fundamental", KW_IS_FUNDAMENTAL},
  {"__is_pod", KW_IS_POD},
  {"__is_polymorphic", KW_IS_POLYMORPHIC},
  {"__is_standard_layout", KW_IS_STANDARD_LAYOUT},
  {"__is_trivial", KW_IS_TRIVIAL},
  {"__is_trivially_copyable", KW_IS_TRIVIALLY_COPYABLE},
  {"__is_union", KW_IS_UNION},
  {"long", KW_LONG},
  {"__make_map_keys_seq", KW_MAKE_MAP_KEYS_SEQ},
  {"__make_map_property", KW_MAKE_MAP_PROPERTY},
  {"__make_property", KW_MAKE_PROPERTY},
  {"__make_property2", KW_MAKE_PROPERTY2},
  {"__make_seq", KW_MAKE_SEQ},
  {"__make_seq_property", KW_MAKE_SEQ_PROPERTY},
  {"mutable", KW_MUTABLE},
  {"namespace", KW_NAMESPACE},
  {"noexcept", KW_NOEXCEPT},
  {"nullptr", KW_NULLPTR},
  {"new", KW_NEW},
  {"operator", KW_OPERATOR},
  {"override", KW_OVERRIDE},
  {"private", KW_PRIVATE},
  {"protected", KW_PROTECTED},
  {"public", KW_PUBLIC},
  {"register", KW_REGISTER},
  {"reinterpret_cast", KW_REINTERPRET_CAST},
  //{"restrict", KW_RESTRICT},
  {"__restrict", KW_RESTRICT},
  {"__restrict__", KW_RESTRICT},
  {"return", KW_RETURN},
  {"short", KW_SHORT},
  {"signed", KW_SIGNED},
  {"sizeof", KW_SIZEOF},
  {"static", KW_STATIC},
  {"static_assert", KW_STATIC_ASSERT},
  {"static_cast", KW_STATIC_CAST},
  {"struct", KW_STRUCT},
  {"template", KW_TEMPLATE},
  {"thread_local", KW_THREAD_LOCAL},
  {"throw", KW_THROW},
  {"true", KW_TRUE},
  {"try", KW_TRY},
  {"typedef", KW_TYPEDEF},
  {"typeid", KW_TYPEID},
  {"typename", KW_TYPENAME},
  {"__underlying_type", KW_UNDERLYING_TYPE},
  {"union", KW_UNION},
  {"unsigned", KW_UNSIGNED},
  {"using", KW_USING},
  {"virtual", KW_VIRTUAL},
  {"void", KW_VOID},
  {"volatile", KW_VOLATILE},
  {"wchar_t", KW_WCHAR_T},
  {"while", KW_WHILE},

  // These are alternative ways to refer to built-in operators.
  {"and", ANDAND},
  {"and_eq", ANDEQUAL},
  {"bitand", '&'},
  {"bitor", '|'},
  {"compl", '~'},
  {"not", '!'},
  {"not_eq", NECOMPARE},
  {"or", OROR},
  {"or_eq", OREQUAL},
  {"xor", '^'},
  {"xor_eq", XOREQUAL},
};

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

/**
 *
 */
CPPPreprocessor::InputFile::
InputFile() {
  _in = nullptr;
  _manifest = nullptr;
  _line_number = 0;
  _col_number = 0;
  _next_line_number = 1;
  _next_col_number = 1;
  _lock_position = false;
  _ignore_manifest = false;
}

/**
 *
 */
CPPPreprocessor::InputFile::
~InputFile() {
  if (_in != nullptr) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting the
    // stream pointer does not call the appropriate global delete function;
    // instead apparently calling the system delete function.  So we call the
    // delete function by hand instead.
#if !defined(USE_MEMORY_NOWRAPPERS) && defined(REDEFINE_GLOBAL_OPERATOR_NEW)
    _in->~istream();
    (*global_operator_delete)(_in);
#else
    delete _in;
#endif
  }
}

/**
 *
 */
bool CPPPreprocessor::InputFile::
open(const CPPFile &file) {
  assert(_in == nullptr);

  _file = file;
  pifstream *in = new pifstream;
  _in = in;

  return _file._filename.open_read(*in);
}

/**
 *
 */
bool CPPPreprocessor::InputFile::
connect_input(const string &input) {
  assert(_in == nullptr);

  _input = input;
  _in = new std::istringstream(_input);
  return !_in->fail();
}

/**
 * Fetches a single character from the source file.
 */
int CPPPreprocessor::InputFile::
get() {
  assert(_in != nullptr);

  if (!_lock_position) {
    _line_number = _next_line_number;
    _col_number = _next_col_number;
  }

  int c = _in->get();

  // Quietly skip over embedded carriage-return characters.  We shouldn't see
  // any of these unless there was some DOS-to-Unix file conversion problem.
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

/**
 * Like get(), but does not advance the file pointer.
 */
int CPPPreprocessor::InputFile::
peek() {
  assert(_in != nullptr);

  int c = _in->peek();

  // Quietly skip over embedded carriage-return characters.  We shouldn't see
  // any of these unless there was some DOS-to-Unix file conversion problem.
  while (c == '\r') {
    _in->get();
    c = _in->peek();
  }

  return c;
}

/**
 *
 */
CPPPreprocessor::
CPPPreprocessor() {
  _noangles = false;
  _state = S_eof;
  _paren_nesting = 0;
  _parsing_template_params = false;
  _parsing_attribute = false;
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

/**
 *
 */
bool CPPPreprocessor::
preprocess_file(const Filename &filename) {
  Filename canonical(filename);
  canonical.make_canonical();

  CPPFile file(canonical, filename, CPPFile::S_local);

  // Don't read it if we included it before and it had #pragma once.
  ParsedFiles::iterator it = _parsed_files.find(file);
  if (it != _parsed_files.end() && it->_pragma_once) {
    // But mark it as local.
    it->_source = CPPFile::S_local;
    return true;
  }

  if (!init_cpp(file)) {
    std::cerr << "Unable to read " << filename << "\n";
    return false;
  }

  int line_number = 1;
  int nesting = 0;
  bool next_space = false;
  CPPToken token = get_next_token();
  while (!token.is_eof()) {
    if (token._token == '}') {
      nesting -= 1;
    }
    if (token._lloc.first_line > line_number) {
      // Token is on a different line, so insert a newline.
      std::cout << "\n";
      line_number = token._lloc.first_line;
      indent(std::cout, nesting * 2);
    }
    else if (next_space && token._token != ';' && token._token != ':' && token._token != ',' && token._token != ')') {
      // The above tokens never need a preceding space
      std::cout << " ";
    }
    if (token._token == '{') {
      nesting += 1;
    }
    next_space = (token._token != '(' && token._token != '~');
    token.output_code(std::cout);

    CPPToken next_token = get_next_token();
    if (next_token._lloc.file != token._lloc.file) {
      // Switched to a new file, reset the line number.
      line_number = 0;
    }
    token = std::move(next_token);
  }
  std::cout << "\n";

  return get_error_count() == 0;
}

/**
 * Sets the verbosity level of the parser.  At 0, no warnings will be
 * reported; at 1 or higher, expect to get spammed.
 */
void CPPPreprocessor::
set_verbose(int verbose) {
  _verbose = verbose;
}

/**
 * Returns the verbosity level of the parser.
 */
int CPPPreprocessor::
get_verbose() const {
  return _verbose;
}

/**
 *
 */
void CPPPreprocessor::
copy_filepos(const CPPPreprocessor &other) {
  InputFile *infile = _infile;
  assert(infile != nullptr);
  infile->_file = other.get_file();
  infile->_line_number = other.get_line_number();
  infile->_col_number = other.get_col_number();
}

/**
 *
 */
CPPFile CPPPreprocessor::
get_file() const {
  InputFile *infile = _infile;
  if (infile != nullptr) {
    return infile->_file;
  } else {
    return CPPFile("");
  }
}

/**
 * Returns the line number of the last character returned by get().
 */
int CPPPreprocessor::
get_line_number() const {
  InputFile *infile = _infile;
  if (infile != nullptr) {
    return infile->_line_number;
  } else {
    return 0;
  }
}

/**
 * Returns the column number of the last character returned by get().
 */
int CPPPreprocessor::
get_col_number() const {
  InputFile *infile = _infile;
  if (infile != nullptr) {
    return infile->_col_number;
  } else {
    return 0;
  }
}

/**
 *
 */
CPPToken CPPPreprocessor::
get_next_token() {

#ifdef CPP_VERBOSE_LEX
  CPPToken tok = get_next_token0();
  indent(cerr, get_file_depth() * 2)
    << _token_index++ << ". " << tok << "\n";
  return tok;
}

CPPToken CPPPreprocessor::
get_next_token0() {
#endif

  // We make a nested call to internal_get_next_token(), so we can combine
  // sequences of identifiers and scoping symbols into a single identifier,
  // for yacc's convenience.

  CPPToken token(0);
  if (!_saved_tokens.empty()) {
    token = std::move(_saved_tokens.back());
    _saved_tokens.pop_back();
  } else {
    token = internal_get_next_token();
  }

  YYLTYPE loc = token._lloc;

  if (_resolve_identifiers &&
      (token._token == SIMPLE_IDENTIFIER || token._token == SCOPE)) {
    // We will be returning a scoped identifier, or a scoping.  Keep pulling
    // off tokens until we reach the end of the scopeidentifier sequence.

    string name;

    // If we started the ball with an identifier, use it and get the next
    // token.  Otherwise, we started with :: (global scope), and we indicate
    // this with an empty string at the beginning of the scoping sequence.
    if (token._token == SIMPLE_IDENTIFIER) {
      name = token._lval.str;
      token = internal_get_next_token();
    }

    CPPIdentifier *ident = new CPPIdentifier(name, loc);
    YYSTYPE result;
    result.u.identifier = ident;

    if (token._token == '<') {
      // If the next token is an angle bracket and the current identifier
      // wants template instantiation, assume the angle bracket begins the
      // instantiation and call yacc recursively to parse the template
      // parameters.
      CPPDeclaration *decl = ident->find_template(current_scope, global_scope);
      if (decl != nullptr) {
        ident->_names.back().set_templ
          (nested_parse_template_instantiation(decl->get_template_scope()));
        token = internal_get_next_token();
      //} else {
      //  error(string("unknown template '") + ident->get_fully_scoped_name() + "'", loc);
      }
    }

    while (token._token == SCOPE) {
      loc.last_line = token._lloc.last_line;
      loc.last_column = token._lloc.last_column;

      name += "::";
      token = internal_get_next_token();
      string token_prefix;

      if (token._token == '~') {
        // A scoping operator followed by a tilde can only be the start of a
        // scoped destructor name.  Make the tilde be part of the name.
        name += "~";
        token_prefix = "~";
        token = internal_get_next_token();
      }

      if (token._token != SIMPLE_IDENTIFIER) {
        // The last useful token was a SCOPE, thus this is a scoping token.

        if (token._token == KW_OPERATOR) {
          // Unless the last token we came across was the "operator" keyword.
          // We make a special case for this, because it's occasionally scoped
          // in normal use.
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
        // If the next token is an angle bracket and the current indentifier
        // wants template instantiation, assume the angle bracket begins the
        // instantiation and call yacc recursively to parse the template
        // parameters.
        CPPDeclaration *decl =
          ident->find_template(current_scope, global_scope);
        if (decl != nullptr) {
          ident->_names.back().set_templ
            (nested_parse_template_instantiation(decl->get_template_scope()));
          token = internal_get_next_token();
        } else {
          error(string("unknown template '") + ident->get_fully_scoped_name() + "'", loc);
        }
      }
    }
    // The last useful token was a SIMPLE_IDENTIFIER, thus this is a normal
    // scoped identifier.
    _saved_tokens.push_back(token);

    int token_type = IDENTIFIER;
    CPPDeclaration *decl = ident->find_symbol(current_scope, global_scope);
    if (decl != nullptr && decl->as_type() != nullptr) {
      // We need to see type pack template parameters as a different type of
      // identifier to resolve a parser ambiguity.
      CPPClassTemplateParameter *ctp = decl->as_class_template_parameter();
      if (ctp && ctp->_packed) {
        token_type = TYPEPACK_IDENTIFIER;
      } else {
        token_type = TYPENAME_IDENTIFIER;
      }
    }

    _last_token_loc = loc;
    return CPPToken(token_type, loc, name, result);
  }

  // This is the normal case: just pass through whatever token we got.
  _last_token_loc = loc;
  return token;
}

/**
 *
 */
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

/**
 *
 */
void CPPPreprocessor::
warning(const string &message) const {
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

/**
 *
 */
void CPPPreprocessor::
warning(const string &message, const YYLTYPE &loc) const {
  if (_verbose >= 2) {
    if (_verbose >= 3) {
      indent(cerr, get_file_depth() * 2);
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

/**
 *
 */
void CPPPreprocessor::
error(const string &message) const {
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

/**
 *
 */
void CPPPreprocessor::
error(const string &message, const YYLTYPE &loc) const {
  if (_state == S_nested || _state == S_end_nested) {
    // Don't report or log errors in the nested state.  These will be reported
    // when the nesting level collapses.
    return;
  }

  if (_verbose >= 1) {
    if (_verbose >= 3) {
      indent(cerr, get_file_depth() * 2);
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

    InputFile *infile = _infile;
    if (infile != nullptr && !loc.file.empty()) {
      // Add all the expansions to a vector for easy reverse iteration.
      std::vector<InputFile *> infiles;
      while (infile != nullptr && infile->_file == loc.file && infile->_manifest != nullptr) {
        infiles.push_back(infile);
        infile = infile->_parent;
      }
      if (!infiles.empty()) {
        auto rit = infiles.rbegin();
        if (_verbose >= 3) {
          cerr << "Expansion of " << (*rit)->_manifest->_name << ":\n";
          while (rit != infiles.rend()) {
            cerr << " -> " << trim_blanks((*rit)->_input) << "\n";
            ++rit;
          }
        }
        else {
          cerr << "with " << (*rit)->_manifest->_name;
          if ((*rit)->_manifest->_has_parameters) {
            cerr << "()";
          }
          cerr << " expanded to: " << trim_blanks(_infile->_input) << "\n";
        }
        cerr << std::endl;
      }
    }

    if (_error_abort) {
      cerr << "Aborting.\n";
      abort();
    }
  }
  _error_count++;
}

/**
 * Shows the indicated line, useful for error messages.
 */
void CPPPreprocessor::
show_line(const YYLTYPE &loc) const {
  if (loc.file._filename.empty()) {
    return;
  }

  int indent_level = 0;
  if (_verbose >= 3) {
    indent_level = get_file_depth() * 2;
  }

  // Seek to the offending line in the file.
  std::ifstream stream;
  if (loc.file._filename.open_read(stream)) {
    int l = 0;
    string linestr;
    while (l < loc.first_line) {
      std::getline(stream, linestr);
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

/**
 *
 */
int CPPPreprocessor::
get_warning_count() const {
  return _warning_count;
}

/**
 *
 */
int CPPPreprocessor::
get_error_count() const {
  return _error_count;
}

/**
 * Returns the CPPCommentBlock immediately preceding the indicated line, if
 * any.  If there is no such comment, returns NULL.
 */
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
        return nullptr;
      }
    } else {
      wrong_file_count++;
      if (wrong_file_count > 10) {
        return nullptr;
      }
    }

    ++ci;
  }

  return nullptr;
}

/**
 * Returns the CPPCommentBlock that starts on the indicated line, if any.  If
 * there is no such comment, returns NULL.
 */
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
        return nullptr;
      }
    }

    ++ci;
  }

  return nullptr;
}

/**
 *
 */
bool CPPPreprocessor::
init_cpp(const CPPFile &file) {
  _state = S_normal;
  _saved_tokens.push_back(CPPToken(START_CPP));
  _last_c = '\0';

  return push_file(file);
}

/**
 *
 */
bool CPPPreprocessor::
init_const_expr(const string &expr) {
  _state = S_normal;
  _saved_tokens.push_back(CPPToken(START_CONST_EXPR));

  return push_string(expr);
}

/**
 *
 */
bool CPPPreprocessor::
init_type(const string &type) {
  _state = S_normal;
  _saved_tokens.push_back(CPPToken(START_TYPE));

  return push_string(type);
}

/**
 *
 */
bool CPPPreprocessor::
push_file(const CPPFile &file) {
  if (_verbose >= 3) {
    indent(cerr, get_file_depth() * 2)
      << "Reading " << file << "\n";
  }
  assert(_last_c == 0);

  InputFile *infile = new InputFile;
  if (infile->open(file)) {
    infile->_parent = _infile;
    _infile = infile;

    // Record the fact that we opened the file for the benefit of user code.
    _parsed_files.insert(file);

    infile->_prev_last_c = _last_c;
    _last_c = '\0';
    _start_of_line = true;
    return true;
  }

  delete infile;
  return false;
}

/**
 *
 */
bool CPPPreprocessor::
push_string(const string &input) {
#ifdef CPP_VERBOSE_LEX
  indent(cerr, get_file_depth() * 2)
    << "Pushing to string \"" << input
    << "\"\n";
#endif

  InputFile *infile = new InputFile;
  if (infile->connect_input(input)) {
    infile->_prev_last_c = _last_c;
    infile->_parent = _infile;
    _infile = infile;
    _last_c = '\0';
    return true;
  }

#ifdef CPP_VERBOSE_LEX
  indent(cerr, get_file_depth() * 2)
    << "Unable to read string\n";
#endif

  delete infile;
  return false;
}

/**
 *
 */
bool CPPPreprocessor::
push_expansion(const string &input, const CPPManifest *manifest, const YYLTYPE &loc) {
#ifdef CPP_VERBOSE_LEX
  indent(cerr, get_file_depth() * 2)
    << "Pushing to expansion \"" << input
    << "\"\n";
#endif

  InputFile *infile = new InputFile;
  if (infile->connect_input(input)) {
    infile->_manifest = manifest;
    infile->_file = loc.file;
    infile->_line_number = loc.first_line;
    infile->_col_number = loc.first_column;
    infile->_lock_position = true;

    if (!manifest->_has_parameters) {
      // If the manifest does not use arguments, then disallow recursive
      // expansion.
      infile->_ignore_manifest = true;
    }

    infile->_prev_last_c = _last_c;
    infile->_parent = _infile;
    _infile = infile;
    _last_c = '\0';
    return true;
  }

#ifdef CPP_VERBOSE_LEX
  indent(cerr, get_file_depth() * 2)
    << "Unable to read expansion\n";
#endif

  delete infile;
  return false;
}

/**
 * Given a string, expand all manifests within the string.
 */
void CPPPreprocessor::
expand_manifests(string &expr, bool expand_undefined,
                 const CPPManifest::Ignores &ignores) const {
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
      }
      else if (expand_undefined && ident == "__has_include") {
        expand_has_include_function(expr, q, p);
      }
      else if (ident == "L" && p < expr.size() && (expr[p] == '\'' || expr[p] == '\"')) {
        // Special exception for the wide string literal suffix, which is
        // never expanded.
      }
      else {
        // Is it a manifest?
        Manifests::const_iterator mi = _manifests.find(ident);
        if (mi != _manifests.end() && ignores.count((*mi).second) == 0) {
          const CPPManifest *manifest = (*mi).second;
          vector_string args;
          if (manifest->_has_parameters) {
            // If it's not followed by a parenthesis, don't expand it.
            while (p < expr.size() && isspace(expr[p])) {
              p++;
            }
            if (p >= expr.size() || expr[p] != '(') {
              continue;
            }

            manifest->extract_args(args, expr, p);
          }

          // Don't consider this manifest when expanding the arguments or
          // result, to prevent recursion.
          CPPManifest::Ignores nested_ignores(ignores);
          nested_ignores.insert(manifest);

          string result = manifest->expand(args, expand_undefined, nested_ignores);
          expand_manifests(result, expand_undefined, nested_ignores);

          expr = expr.substr(0, q) + result + expr.substr(p);
          p = q + result.size();
        }
        else if (ident == "__FILE__") {
          // Special case: this is a dynamic definition.
          CPPFile file = get_file();
          string result = string("\"") + file._filename_as_referenced.get_fullpath() + "\"";
          expr = expr.substr(0, q) + result + expr.substr(p);
          p = q + result.size();

        }
        else if (ident == "__LINE__") {
          // So is this.
          string line = format_string(get_line_number());
          expr = expr.substr(0, q) + line + expr.substr(p);
          p = q + line.size();
        }
        else if (expand_undefined && ident != "true" && ident != "false") {
          // It is not found.  Expand it to 0, but only if we are currently
          // parsing an #if expression.
          expr = expr.substr(0, q) + "0" + expr.substr(p);
          p = q + 1;
        }
      }
    }
    else if (expr[p] == '\'' || expr[p] == '"') {
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
        warning("missing terminating " + string(1, quote) + " character");
      }
      p++;
    }
    else {
      p++;
    }
  }
}

/**
 * Given a string, expand all manifests within the string and evaluate it as
 * an expression.  Returns NULL if the string is not a valid expression.
 *
 * This is an internal support function for CPPPreprocessor; however, there is
 * a public variant of this function defined for CPPParser.
 */
CPPExpression *CPPPreprocessor::
parse_expr(const string &input_expr, CPPScope *current_scope,
           CPPScope *global_scope, const YYLTYPE &loc) {
  string expr = input_expr;
  expand_manifests(expr, false);

  CPPExpressionParser ep(current_scope, global_scope);
  ep._verbose = 0;
  if (ep.parse_expr(expr, *this)) {
    return ep._expr;
  } else {
    return nullptr;
  }
}

/**
 *
 */
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

  // Skip any whitespace, comments, and preprocessor directives before the
  // token.
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
    // If we're running a nested lexer, keep track of the paren levels.  When
    // we encounter a comma or closing angle bracket at the bottom level, we
    // stop.

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
        return CPPToken(0, loc);
      }
      break;

    case '>':
      if (_paren_nesting <= 0) {
        _parsing_template_params = false;
        _state = S_end_nested;
        return CPPToken(0, loc);
      }
    }
  } else if (_parsing_attribute) {
    // If we're parsing an attribute, also keep track of the paren nesting.
    if (c == '[' || c == '(') {
      ++_paren_nesting;
    } else if (c == ']' || c == ')') {
      --_paren_nesting;
    }
  }

  // Look for an end-of-line comment, and parse it before we finish this
  // token.  This is not strictly necessary, but it allows us to pick up
  // docstrings from comments after enum values.
  while (next_c != EOF && isspace(next_c)) {
    get();
    next_c = peek();
  }
  if (next_c == '/') {
    _last_c = skip_whitespace(get());
  }

  return CPPToken(c, loc);
}

/**
 * Checks the next character in the stream to see if this might be a two-
 * character token.  Returns 0 if it is only a single-character token.
 */
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
      // Don't parse >> as right-shift when parsing a template list, as per
      // C++11, to allow a syntax like A<B>>. However, nested >> must be
      // preserved, such as in A<(2>>1)>
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

  case '[':
    if (next_c == '[' && !_parsing_attribute) {
      _parsing_attribute = true;
      return ATTR_LEFT;
    }
    break;

  case ']':
    if (next_c == ']' && _parsing_attribute && _paren_nesting == 0) {
      _parsing_attribute = false;
      return ATTR_RIGHT;
    }
    break;
  }

  return 0;
}

/**
 * Checks the next character in the stream to see if this might be a three-
 * character token; usually called in conjunction with check_digraph.  Returns
 * 0 if it is not a three-character token.
 */
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

  case LECOMPARE:
    if (next_c == '>') return SPACESHIP;
    break;
  }

  return 0;
}

/**
 *
 */
int CPPPreprocessor::
skip_whitespace(int c) {
  while (c != EOF) {
    c = skip_comment(c);

    if (c == '\\') {
      // This does not usually occur in the middle of unquoted C++ code,
      // except before a newline character.
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

/**
 *
 */
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

/**
 *
 */
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

/**
 *
 */
int CPPPreprocessor::
skip_cpp_comment(int c) {
  if (_save_comments) {
    CPPCommentBlock *comment;

    int line_number = get_line_number();
    if (c == '\n') {
      // We have to subtract one from the line number as we just fetched a
      // newline.
      --line_number;
    }

    if (_last_cpp_comment && !_comments.empty() &&
        _comments.back()->_last_line >= line_number - 1) {
      // If the last non-whitespace character read was also part of a C++
      // comment, then this is just a continuation of that comment block.
      // However, if there was a line without comment in between, it starts a
      // new block anyway.
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

/**
 * Skips a C++14 digit separator that has just been found through peek().
 */
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
    // This assumes that this isn't a character constant directly follows a
    // digit sequence, like 123'a' -- I can't think of a situation where
    // that's legal anyway, though.
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

/**
 *
 */
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
  indent(cerr, get_file_depth() * 2)
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
    // Presumably this follows some #if or #ifdef.  We don't bother to check
    // this, however.
    skip_false_if_block(false);
  } else if (command == "endif") {
    // Presumably this follows some #if or #ifdef.  We don't bother to check
    // this, however.
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

/**
 *
 */
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

/**
 *
 */
int CPPPreprocessor::
get_preprocessor_args(int c, string &args) {
  // Following the command, the rest of the line, as well as any text on
  // successive lines, is part of the arguments to the command.

  // Check for comments first.
  c = skip_comment(c);

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

/**
 *
 */
void CPPPreprocessor::
handle_define_directive(const string &args, const YYLTYPE &loc) {
  if (args.empty()) {
    warning("Ignoring empty #define directive", loc);
  } else {
    CPPManifest *manifest = new CPPManifest(*this, args, loc);
    manifest->_vis = preprocessor_vis;
    if (!manifest->_has_parameters) {
      string expr_string = manifest->expand();
      if (!expr_string.empty()) {
        manifest->_expr = parse_expr(expr_string, global_scope, global_scope, loc);
      }
    }

    std::pair<Manifests::iterator, bool> result =
      _manifests.insert(Manifests::value_type(manifest->_name, manifest));

    if (!result.second) {
      // There was already a macro with this name.  Delete the old.
      CPPManifest *other = result.first->second;
      if (!manifest->is_equal(other)) {
        warning("redefinition of macro '" + manifest->_name + "'", loc);
        warning("previous definition is here", other->_loc);
      }
      result.first->second = manifest;
    }
  }
}

/**
 *
 */
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

/**
 *
 */
void CPPPreprocessor::
handle_ifdef_directive(const string &args, const YYLTYPE &loc) {
  if (!is_manifest_defined(args)) {
    // The macro is undefined.  Skip stuff.
    skip_false_if_block(true);
  }
}

/**
 *
 */
void CPPPreprocessor::
handle_ifndef_directive(const string &args, const YYLTYPE &loc) {
  if (is_manifest_defined(args)) {
    // The macro is defined.  Skip stuff.
    skip_false_if_block(true);
  }
}

/**
 *
 */
void CPPPreprocessor::
handle_if_directive(const string &args, const YYLTYPE &loc) {
  // When expanding manifests, we should replace unknown macros with 0.
  string expr = args;
  expand_manifests(expr, true);

  int expression_result = 0;
  CPPExpressionParser ep(current_scope, global_scope);
  ep._verbose = 0;
  if (ep.parse_expr(expr, *this)) {
    CPPExpression::Result result = ep._expr->evaluate();
    if (result._type == CPPExpression::RT_error) {
      std::ostringstream strm;
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

/**
 *
 */
void CPPPreprocessor::
handle_include_directive(const string &args, const YYLTYPE &loc) {
  Filename filename;
  Filename filename_as_referenced;
  bool angle_quotes = false;

  string expr = args;

  // The filename to include might actually be hidden within a manifest
  // definition.  Wow.  FreeType depends on this.

  // Just to play things safe, since our manifest-expansion logic might not
  // filter out quotes and angle brackets properly, we'll only expand
  // manifests if we don't begin with a quote or bracket.
  if (!expr.empty() && (expr[0] != '"' && expr[0] != '<')) {
    expand_manifests(expr, false);
  }

  if (!expr.empty()) {
    if (expr[0] == '"' && expr[expr.size() - 1] == '"') {
      filename = expr.substr(1, expr.size() - 2);

      if (_infile->_parent == nullptr) {
        // If we're currently processing a top-level file, record the include
        // directive.  We don't need to record includes from included files.
        _quote_includes.insert(filename);
      }
    } else if (expr[0] == '<' && expr[expr.size() - 1] == '>') {
      filename = expr.substr(1, expr.size() - 2);
      if (!_noangles) {
        // If _noangles is true, we don't make a distinction between angle
        // brackets and quote marks--all #include statements are treated the
        // same, as if they used quote marks.
        angle_quotes = true;
      }

      if (_infile->_parent == nullptr) {
        // If we're currently processing a top-level file, record the include
        // directive.  We don't need to record includes from included files.
        _angle_includes.insert(filename);
      }
    }
  } else {
    warning("Ignoring invalid #include directive", loc);
  }

  filename.set_text();
  filename_as_referenced = filename;

  // Now look for the filename.  If we didn't use angle quotes, look first in
  // the current directory.
  CPPFile::Source source = CPPFile::S_none;

  if (find_include(filename, angle_quotes, source)) {
    _last_c = '\0';

    // If it was explicitly named on the command-line, mark it S_local.
    filename.make_canonical();
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
  } else {
    warning("Cannot find " + filename.get_fullpath(), loc);
  }
}

/**
 *
 */
void CPPPreprocessor::
handle_pragma_directive(const string &args, const YYLTYPE &loc) {
  if (args == "once") {
    ParsedFiles::iterator it = _parsed_files.find(loc.file);
    assert(it != _parsed_files.end());
    it->_pragma_once = true;
  }

  char macro[64];
  if (sscanf(args.c_str(), "push_macro ( \"%63[^\"]\" )", macro) == 1) {
    // We just mark it as pushed for now, so that the next time someone tries
    // to override it, we save the old value.
    Manifests::iterator mi = _manifests.find(macro);
    if (mi != _manifests.end()) {
      _manifest_stack[macro].push_back(mi->second);
    } else {
      _manifest_stack[macro].push_back(nullptr);
    }

  } else if (sscanf(args.c_str(), "pop_macro ( \"%63[^\"]\" )", macro) == 1) {
    ManifestStack &stack = _manifest_stack[macro];
    if (stack.size() > 0) {
      CPPManifest *manifest = stack.back();
      stack.pop_back();
      Manifests::iterator mi = _manifests.find(macro);
      if (manifest == nullptr) {
        // It was undefined when it was pushed, so make it undefined again.
        if (mi != _manifests.end()) {
          _manifests.erase(mi);
        }
      } else if (mi != _manifests.end()) {
        mi->second = manifest;
      } else {
        _manifests.insert(Manifests::value_type(macro, manifest));
      }
    } else {
      warning("pop_macro without matching push_macro", loc);
    }
  }
}

/**
 *
 */
void CPPPreprocessor::
handle_error_directive(const string &args, const YYLTYPE &loc) {
  error(args, loc);
}

/**
 * We come here when we fail an #if or an #ifdef test, or when we reach the
 * #else clause to something we didn't fail.  This function skips all text up
 * until the matching #endif.
 */
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

/**
 * Returns true if the given manifest is defined.
 */
bool CPPPreprocessor::
is_manifest_defined(const string &manifest_name) const {
  Manifests::const_iterator mi = _manifests.find(manifest_name);
  if (mi != _manifests.end()) {
    return true;
  }

  if (manifest_name == "__has_include" ||
      manifest_name == "__FILE__" ||
      manifest_name == "__LINE__") {
    // Special built-in directives that are considered "defined".
    return true;
  }

  return false;
}

/**
 * Locates the given filename.  Changes the first argument to the full path.
 */
bool CPPPreprocessor::
find_include(Filename &filename, bool angle_quotes, CPPFile::Source &source) const {
  // Now look for the filename.  If we didn't use angle quotes, look first in
  // the current directory.
  if (!angle_quotes && filename.exists()) {
    source = CPPFile::S_local;
    return true;
  }

  // Search the same directory as the includer.
  if (!angle_quotes) {
    Filename match(get_file()._filename.get_dirname(), filename);
    if (match.exists()) {
      filename = match;
      source = CPPFile::S_alternate;
      return true;
    }
  }

  // Now search the angle-include-path
  if (angle_quotes && filename.resolve_filename(_angle_include_path)) {
    source = CPPFile::S_system;
    return true;
  }

  // Now search the quote-include-path
  if (!angle_quotes) {
    for (size_t dir = 0; dir < _quote_include_path.get_num_directories(); ++dir) {
      Filename match(_quote_include_path.get_directory(dir), filename);
      if (match.exists()) {
        filename = match;
        source = _quote_include_kind[dir];
        return true;
      }
    }
  }

  return false;
}

/**
 *
 */
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

/**
 *
 */
CPPToken CPPPreprocessor::
get_quoted_string(int c) {
  YYLTYPE loc;
  loc.file = get_file();
  loc.first_line = get_line_number();
  loc.first_column = get_col_number();

  string str = scan_quoted(c);

  return get_literal(SIMPLE_STRING, loc, str);
}

/**
 *
 */
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
      (name == "L" || name == "u8" || name == "u" || name == "U" ||
       name == "R" || name == "LR" || name == "u8R" || name == "uR" || name == "UR")) {
    // This is actually a wide-character or wide-string literal or some such.
    get();
    string str;
    if (name[name.size() - 1] == 'R') {
      name.resize(name.size() - 1);
      str = scan_raw(c);
    } else {
      str = scan_quoted(c);
    }

    // Figure out the correct character type to use.
    CPPExpression::Type type;
    if (name == "L") {
      type = CPPExpression::T_wstring;
    } else if (name == "u8") {
      type = CPPExpression::T_u8string;
    } else if (name == "u") {
      type = CPPExpression::T_u16string;
    } else if (name == "U") {
      type = CPPExpression::T_u32string;
    } else {
      type = CPPExpression::T_string;
    }

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
    // If the manifest is expecting arguments, we don't expand it unless the
    // the next token is an open-parenthesis.
    CPPManifest *manifest = (*mi).second;
    if (manifest->_has_parameters) {
      while (c != EOF && isspace(c)) {
        get();
        c = peek();
      }
      if (c == '(') {
        // It is followed by a parenthesis, so we can expand this.
        return expand_manifest(manifest, loc);
      }
    } else {
      // Non-function-like macros are always expanded.
      return expand_manifest(manifest, loc);
    }
  }
  if (name == "__FILE__") {
    return get_literal(SIMPLE_STRING, loc, loc.file._filename_as_referenced);
  }
  if (name == "__LINE__") {
    YYSTYPE result;
    result.u.integer = loc.first_line;
    return CPPToken(INTEGER, loc, "", result);
  }

  // Check for keywords.
  auto kwit = keywords.find(name);
  int kw = (kwit != keywords.end()) ? (*kwit).second : 0;

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
    if (kw == KW_EXPLICIT || kw == KW_NOEXCEPT) {
      // These can be followed by a left-paren.  Doing this helps to avoid
      // shift/reduce conflicts in the parser.
      while (c != EOF && isspace(c)) {
        get();
        c = peek();
      }
      if (c == '(') {
        if (kw == KW_EXPLICIT) {
          kw = KW_EXPLICIT_LPAREN;
        }
        else if (kw == KW_NOEXCEPT) {
          kw = KW_NOEXCEPT_LPAREN;
        }
        get();
      }
    }

    YYSTYPE result;
    result.u.identifier = nullptr;
    return CPPToken(kw, loc, name, result);
  }

  return CPPToken(SIMPLE_IDENTIFIER, loc, name);
}

/**
 * Under the assumption that we've just parsed a string or real constant,
 * parse a following custom literal, and returns a token for it.
 */
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

  if (decl == nullptr) {
    // Special case to handle C code, which allows a macro to directly follow
    // a string, like "str"SUFFIX.  In this case, it becomes a separate token.
    Manifests::const_iterator mi = _manifests.find(suffix);
    if (mi != _manifests.end() && !should_ignore_manifest((*mi).second)) {
      CPPManifest *manifest = (*mi).second;
      _saved_tokens.push_back(expand_manifest(manifest, loc));
      return CPPToken(token, loc, str, value);
    }
  }

  if (decl == nullptr || decl->get_subtype() != CPPDeclaration::ST_function_group) {
    error("unknown literal suffix " + suffix, loc);
    return CPPToken(token, loc, str, value);
  }

  // Find the overload with the appropriate signature.
  CPPExpression *expr = nullptr;
  CPPInstance *instance = nullptr;
  CPPInstance *raw_instance = nullptr;
  CPPFunctionGroup *fgroup = decl->as_function_group();
  CPPFunctionGroup::Instances::iterator it;
  for (it = fgroup->_instances.begin(); it != fgroup->_instances.end(); ++it) {
    if ((*it)->_type == nullptr) {
      continue;
    }

    CPPFunctionType *ftype = (*it)->_type->as_function_type();
    if (ftype == nullptr || ftype->_parameters == nullptr) {
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
    if (param == nullptr || param->_type == nullptr) {
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
                                       simple == CPPSimpleType::T_char8_t ||
                                       simple == CPPSimpleType::T_char16_t ||
                                       simple == CPPSimpleType::T_char32_t)) {
        // We currently don't have the means to check the exact character
        // type.
        expr = new CPPExpression(value.u.integer);
        instance = (*it);
        break;
      }

    } else if (type->get_subtype() == CPPDeclaration::ST_pointer) {
      // Must be a const pointer.  Unwrap it.
      type = type->as_pointer_type()->_pointing_at;
      if (type == nullptr || type->get_subtype() != CPPDeclaration::ST_const) {
        continue;
      }
      type = type->as_const_type()->_wrapped_around;
      if (type == nullptr || type->get_subtype() != CPPDeclaration::ST_simple) {
        continue;
      }

      CPPSimpleType::Type simple = type->as_simple_type()->_type;
      if (simple == CPPSimpleType::T_char && params.size() == 1) {
        // This is the raw literal operator.  Store it, but don't break; a
        // non-raw version of the operator might follow, which we'd prefer.
        raw_instance = (*it);

      } else if (token == SIMPLE_STRING && simple == CPPSimpleType::T_char) {
        expr = new CPPExpression(str);
        instance = (*it);
        break;

      } else if (token == STRING_LITERAL) {
        // Verify that the character type of the string literal matches the
        // character type of the parameter.
        CPPExpression::Type str_type = value.u.expr->_type;
        if ((str_type == CPPExpression::T_string && simple == CPPSimpleType::T_char) ||
            (str_type == CPPExpression::T_wstring && simple == CPPSimpleType::T_wchar_t) ||
            (str_type == CPPExpression::T_u8string && (simple == CPPSimpleType::T_char || simple == CPPSimpleType::T_char8_t)) ||
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
  if (instance != nullptr) {
    result.u.expr = new CPPExpression(CPPExpression::literal(expr, instance));
    return CPPToken(CUSTOM_LITERAL, loc, str, result);
  }

  if ((token == REAL || token == INTEGER) && raw_instance != nullptr) {
    // For numeric constants, we can fall back to a raw literal operator.
    result.u.expr = new CPPExpression(CPPExpression::raw_literal(str, instance));
    return CPPToken(CUSTOM_LITERAL, loc, str, result);
  }

  error(fgroup->_name + " has no suitable overload for literal of this type", loc);
  result.u.expr = nullptr;
  return CPPToken(CUSTOM_LITERAL, loc, str, result);
}

/**
 *
 */
CPPToken CPPPreprocessor::
expand_manifest(const CPPManifest *manifest, const YYLTYPE &loc) {
  vector_string args;

  if (manifest->_has_parameters) {
    // Hmm, we're expecting arguments.
    extract_manifest_args(manifest->_name, manifest->_num_parameters,
                          manifest->_variadic_param, args);
  }

  // Keep track of the manifests we're supposed to ignore.
  CPPManifest::Ignores ignores;
  ignores.insert(manifest);

  InputFile *infile = _infile;
  while (infile != nullptr) {
    if (infile->_ignore_manifest) {
      ignores.insert(infile->_manifest);
    }
    infile = infile->_parent;
  }

  string expanded = " " + manifest->expand(args, false, ignores) + " ";
  push_expansion(expanded, manifest, loc);

#ifdef CPP_VERBOSE_LEX
  indent(cerr, get_file_depth() * 2)
    << "Expanding " << manifest->_name << " to " << expanded << "\n";
#endif

  return internal_get_next_token();
}

/**
 *
 */
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
        args.push_back(trim_blanks(arg));
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
        // It could be a slash before a newline.  If so, that's whitespace as
        // well.
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
      args.push_back(trim_blanks(arg));
    }
  }

  YYLTYPE loc;
  loc.first_line = first_line;
  loc.first_column = first_col;
  loc.last_line = first_line;
  loc.last_column = first_col;
  loc.file = first_file;

  if ((int)args.size() < num_args - (va_arg >= 0)) {
    warning("Not enough arguments for manifest " + name, loc);

  } else if (va_arg < 0 && (int)args.size() > num_args) {
    warning("Too many arguments for manifest " + name, loc);
  }
}

/**
 * Expands the defined(manifest) function to either 1 or 0, depending on
 * whether the manifest exists.
 */
void CPPPreprocessor::
expand_defined_function(string &expr, size_t q, size_t &p) const {
  while (p < expr.size() && isspace(expr[p])) {
    p++;
  }

  bool has_paren = false;
  if (expr[p] == '(') {
    has_paren = true;
    p++;
  }

  size_t r = p;
  while (p < expr.size() && (isalnum(expr[p]) || expr[p] == '_')) {
    p++;
  }
  string name = expr.substr(r, p - r);

  if (has_paren) {
    if (expr[p] == ')') {
      p++;
    } else {
      error("missing ')' after 'defined'");
    }
  }

  char result = is_manifest_defined(name) ? '1' : '0';
  expr = expr.substr(0, q) + result + expr.substr(p);
  p = q + 1;
}

/**
 * Expands the __has_include(manifest) function to either 1 or 0, depending on
 * whether the include file exists.
 */
void CPPPreprocessor::
expand_has_include_function(string &expr, size_t q, size_t &p) const {
  bool found_file = false;

  // Skip whitespace till paren.
  while (p < expr.size() && isspace(expr[p])) {
    p++;
  }

  if (expr[p] != '(') {
    error("expected '(' after '__has_include'");
    return;
  }
  p++;
  while (p < expr.size() && isspace(expr[p])) {
    p++;
  }

  int paren_level = 1;
  bool needs_expansion = false;
  size_t r = p;
  while (p < expr.size()) {
    if (expr[p] == '"' || expr[p] == '\'' || expr[p] == '<') {
      // Quoted string or angle bracket.
      int quote_mark = expr[p];
      if (quote_mark == '<') {
        quote_mark = '>';
      }
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
    else if (isalnum(expr[p]) || expr[p] == '_') {
      needs_expansion = true;
    }
    p++;
  }

  if (p >= expr.size() || expr[p] != ')') {
    error("missing ')' after '__has_include'");
    return;
  }

  // Back up to strip trailing whitespace.
  size_t t = p;
  while (t > r && isspace(expr[t - 1])) {
    --t;
  }
  string inc = expr.substr(r, t - r);
  p++;

  // Only expand if we've encountered unquoted identifier-valid characters,
  // to be on the safe side.
  if (needs_expansion) {
    expand_manifests(inc, false);
  }

  Filename filename;
  bool angle_quotes = false;

  if (!inc.empty() && inc[0] == '"' && inc[inc.size() - 1] == '"') {
    filename = inc.substr(1, inc.size() - 2);
  }
  else if (!inc.empty() && inc[0] == '<' && inc[inc.size() - 1] == '>') {
    filename = inc.substr(1, inc.size() - 2);
    if (!_noangles) {
      // If _noangles is true, we don't make a distinction between angle
      // brackets and quote marks--all #inc statements are treated the
      // same, as if they used quote marks.
      angle_quotes = true;
    }
  }
  else {
    warning("invalid argument for __has_include() directive: " + inc);
    expr = expr.substr(0, q) + "0" + expr.substr(p);
    p = q + 1;
    return;
  }

  filename.set_text();

  CPPFile::Source source = CPPFile::S_none;
  found_file = find_include(filename, angle_quotes, source);

  string result = found_file ? "1" : "0";
  expr = expr.substr(0, q) + result + expr.substr(p);
  p = q + result.size();
}

/**
 * Assuming that we've just read a digit or a period indicating the start of a
 * number, read the rest.
 */
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
    result.u.integer = strtol(num.c_str(), nullptr, 16);

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
    result.u.integer = strtol(bin.c_str(), nullptr, 2);

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
    result.u.real = (long double)pstrtod(num.c_str(), nullptr);

    return get_literal(REAL, loc, num, result);
  }

  // This is a decimal or octal integer number.

  loc.last_line = get_line_number();
  loc.last_column = get_col_number();

  YYSTYPE result;

  if (leading_zero) {
    // A leading zero implies an octal number.  strtol() is supposed to be
    // able to make this distinction by itself, but we'll do it explicitly
    // just to be sure.
    result.u.integer = strtol(num.c_str(), nullptr, 8);

  } else {
    // A decimal (base 10) integer.
    result.u.integer = strtol(num.c_str(), nullptr, 10);
  }

  return get_literal(INTEGER, loc, num, result);
}

/**
 *
 */
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

/**
 *
 */
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

/**
 * Parses a C++11 raw string.
 */
string CPPPreprocessor::
scan_raw(int c) {
  int quote_mark = c;

  string delimiter = ")";

  string str;
  c = get();
  while (c != EOF && c != '(') {
    delimiter += c;
    c = get();
  }

  // OK, now start parsing the string, until we see the delimiter again.
  c = get();
  while (c != EOF) {
    if (c == quote_mark) {
      // We encountered a quote mark - did the last part of the string end
      // with the given delimiter?  If so, we've reached the end.
      if (str.compare(str.size() - delimiter.size(), delimiter.size(), delimiter) == 0) {
        str.resize(str.size() - delimiter.size());
        break;
      }
    }
    str += c;
    c = get();
  }

  if (c != quote_mark) {
    warning("Unclosed string");
  }
  return str;
}

/**
 * Returns true if the manifest is one that is being ignored right now
 * (presumably because we are presently expanding it).
 */
bool CPPPreprocessor::
should_ignore_manifest(const CPPManifest *manifest) const {
  InputFile *infile = _infile;
  while (infile != nullptr) {
    if (infile->_ignore_manifest && infile->_manifest == manifest) {
      return true;
    }
    infile = infile->_parent;
  }

  return false;
}

/**
 * Returns true if we should ignore any preprocessor directives (e.g.  we're
 * presently expanding a manifest).
 */
bool CPPPreprocessor::
should_ignore_preprocessor() const {
  InputFile *infile = _infile;
  while (infile != nullptr) {
    if (infile->_ignore_manifest) {
      return true;
    }
    infile = infile->_parent;
  }

  return false;
}

/**
 *
 */
int CPPPreprocessor::
get() {
  if (_unget != '\0') {
    int c = _unget;
    _unget = '\0';
    return c;
  }

  if (UNLIKELY(_infile == nullptr)) {
    return EOF;
  }

  int c = _infile->get();

  while (UNLIKELY(c == EOF && _infile != nullptr)) {
#ifdef CPP_VERBOSE_LEX
    indent(cerr, get_file_depth() * 2)
      << "End of input stream, restoring to previous input\n";
#endif
    // Pop the last file off the end.
    InputFile *infile = _infile;
    _infile = infile->_parent;
    delete infile;

    // Synthesize a newline, just in case the file doesn't already end with
    // one.
    c = '\n';
  }

  if (c == '\n') {
    _start_of_line = true;
  } else if (!isspace(c) && c != '#') {
    _start_of_line = false;
  }

  return c;
}

/**
 * Like get(), but does not alter the current state.
 */
int CPPPreprocessor::
peek() {
  if (_unget != '\0') {
    return _unget;
  }

  InputFile *infile = _infile;
  if (UNLIKELY(infile == nullptr)) {
    return EOF;
  }

  int c = infile->peek();

  while (UNLIKELY(c == EOF && infile != nullptr)) {
    int last_c = infile->_prev_last_c;
    infile = infile->_parent;

    if (last_c != '\0') {
      c = last_c;
    } else if (infile != nullptr) {
      c = infile->peek();
    }
  }

  return c;
}

/**
 * Undoes the effects of a previous get().  Not recommended, use peek()
 * instead where possible, as it doesn't cause the column index to be off.
 */
void CPPPreprocessor::
unget(int c) {
  assert(_unget == '\0');
  _unget = c;
}

/**
 * Recursively invokes yacc to parse the stuff within angle brackets that's
 * the template instantiation part of an identifier.  This involves setting
 * and restoring some state flags so we can return EOF when we reach the
 * closing bracket.
 */
CPPTemplateParameterList *CPPPreprocessor::
nested_parse_template_instantiation(CPPTemplateScope *scope) {
#ifdef CPP_VERBOSE_LEX
  indent(cerr, get_file_depth() * 2)
    << "Beginning nested parse\n";
#endif
  assert(scope != nullptr);

  State old_state = _state;
  int old_nesting = _paren_nesting;
  bool old_parsing_params = _parsing_template_params;

  const CPPTemplateParameterList &formal_params = scope->_parameters;
  CPPTemplateParameterList::Parameters::const_iterator pi;

  _state = S_nested;
  _paren_nesting = 0;
  _parsing_template_params = true;

  CPPToken token = internal_get_next_token();
  if (token._token == '>' || token._token == 0) {
    _parsing_template_params = false;
  } else {
    _saved_tokens.push_back(token);
  }

  CPPTemplateParameterList *actual_params = new CPPTemplateParameterList;

  for (pi = formal_params._parameters.begin();
       pi != formal_params._parameters.end() && _parsing_template_params;) {
    CPPToken token = peek_next_token();
    YYLTYPE loc = token._lloc;

    CPPDeclaration *decl = (*pi);
    CPPClassTemplateParameter *param = decl->as_class_template_parameter();
    CPPInstance *inst = decl->as_instance();
    if (param) {
      // Parse a typename template parameter.
      _saved_tokens.push_back(CPPToken(START_TYPE));
      CPPType *type = ::parse_type(this, current_scope, global_scope);
      if (type == nullptr) {
        loc.last_line = get_line_number();
        loc.last_column = get_col_number() - 1;
        warning("invalid type", loc);
        skip_to_end_nested();
        type = CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_unknown));
      }
      actual_params->_parameters.push_back(type);

      // If this is a variadic template, keep reading using this parameter.
      if (!param->_packed) {
        ++pi;
      }
    } else if (inst) {
      // Parse a constant expression template parameter.
      _saved_tokens.push_back(CPPToken(START_CONST_EXPR));
      CPPExpression *expr = parse_const_expr(this, current_scope, global_scope);
      if (expr == nullptr) {
        loc.last_line = get_line_number();
        loc.last_column = get_col_number() - 1;
        warning("invalid expression", loc);
        skip_to_end_nested();
        expr = new CPPExpression(0);
      }
      actual_params->_parameters.push_back(expr);

      // If this is a variadic template, keep reading using this parameter.
      if ((inst->_storage_class & CPPInstance::SC_parameter_pack) == 0) {
        ++pi;
      }
    } else {
      loc.last_line = get_line_number();
      loc.last_column = get_col_number() - 1;
      warning("invalid template parameter", loc);
      skip_to_end_nested();
      ++pi;
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
  indent(cerr, get_file_depth() * 2)
    << "Ending nested parse\n";
#endif
  return actual_params;
}


/**
 * This is an error-recovery function, called after returning from a nested
 * parse.  If the state is not S_end_nested, there was an error in parsing the
 * nested tokens, and not all of the nested tokens may have been consumed.
 * This function will consume the rest of the nested tokens.
 */
void CPPPreprocessor::
skip_to_end_nested() {
#ifdef CPP_VERBOSE_LEX
  indent(cerr, get_file_depth() * 2)
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
  indent(cerr, get_file_depth() * 2)
    << "Done skipping tokens.\n";
#endif
}

/**
 * This is an error-recovery function, called after returning from a nested
 * parse.  If we haven't yet consumed the closing angle bracket on the
 * template instantiation, keep consuming tokens until we do.
 */
void CPPPreprocessor::
skip_to_angle_bracket() {
#ifdef CPP_VERBOSE_LEX
  indent(cerr, get_file_depth() * 2)
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
  indent(cerr, get_file_depth() * 2)
    << "Done skipping tokens.\n";
#endif
}

/**
 * Returns the number of files on the _infile list.
 */
int CPPPreprocessor::
get_file_depth() const{
  int depth = 0;
  InputFile *infile = _infile;
  while (infile != nullptr) {
    ++depth;
    infile = infile->_parent;
  }
  return depth;
}
