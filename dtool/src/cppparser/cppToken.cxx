/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppToken.cxx
 * @author drose
 * @date 1999-10-22
 */

#include "cppToken.h"
#include "cppExpression.h"
#include "cppIdentifier.h"
#include "cppBison.h"
#include "pdtoa.h"

#include <ctype.h>

/**
 *
 */
CPPToken::
CPPToken(int token, int line_number, int col_number,
         const CPPFile &file, const std::string &str,
         const YYSTYPE &lval) :
  _token(token), _lval(lval)
{
  _lval.str = str;
  _lloc.first_line = line_number;
  _lloc.first_column = col_number;
  _lloc.last_line = line_number;
  _lloc.last_column = col_number;
  _lloc.file = file;
}

/**
 *
 */
CPPToken::
CPPToken(int token, const YYLTYPE &loc, const std::string &str, const YYSTYPE &val) :
  _token(token), _lval(val), _lloc(loc)
{
  _lval.str = str;
}

/**
 * A named constructor for the token returned when the end of file has been
 * reached.
 */
CPPToken CPPToken::
eof() {
  return CPPToken(0);
}

/**
 * Returns true if this is the EOF token.
 */
bool CPPToken::
is_eof() const {
  return _token == 0;
}

/**
 *
 */
void CPPToken::
output(std::ostream &out) const {
  switch (_token) {
  case REAL:
    out << "REAL " << _lval.u.real;
    break;

  case INTEGER:
    out << "INTEGER " << _lval.u.integer;
    break;

  case CHAR_TOK:
    out << "CHAR_TOK " << _lval.u.integer << " = " << _lval.str;
    break;

  case SIMPLE_STRING:
    out << "SIMPLE_STRING " << _lval.str;
    break;

  case STRING_LITERAL:
    out << "STRING_LITERAL " << *_lval.u.expr;
    break;

  case SIMPLE_IDENTIFIER:
    out << "SIMPLE_IDENTIFIER " << _lval.str;
    break;

  case IDENTIFIER:
    out << "IDENTIFIER " << *_lval.u.identifier;
    break;

  case TYPENAME_IDENTIFIER:
    out << "TYPENAME_IDENTIFIER " << *_lval.u.identifier;
    break;

  case SCOPING:
    out << "SCOPING " << *_lval.u.identifier << "::";
    break;

  case TYPEDEFNAME:
    out << "TYPEDEFNAME " << _lval.str;
    break;

  case ELLIPSIS:
    out << "ELLIPSIS";
    break;

  case OROR:
    out << "OROR";
    break;

  case ANDAND:
    out << "ANDAND";
    break;

  case EQCOMPARE:
    out << "EQCOMPARE";
    break;

  case NECOMPARE:
    out << "NECOMPARE";
    break;

  case LECOMPARE:
    out << "LECOMPARE";
    break;

  case GECOMPARE:
    out << "GECOMPARE";
    break;

  case LSHIFT:
    out << "LSHIFT";
    break;

  case RSHIFT:
    out << "RSHIFT";
    break;

  case POINTSAT_STAR:
    out << "POINTSAT_STAR";
    break;

  case DOT_STAR:
    out << "DOT_STAR";
    break;

  case UNARY_NOT:
    out << "UNARY_NOT";
    break;

  case UNARY_MINUS:
    out << "UNARY_MINUS";
    break;

  case UNARY_PLUS:
    out << "UNARY_PLUS";
    break;

  case UNARY_NEGATE:
    out << "UNARY_NEGATE";
    break;

  case UNARY_STAR:
    out << "UNARY_STAR";
    break;

  case UNARY_REF:
    out << "UNARY_REF";
    break;

  case POINTSAT:
    out << "POINTSAT";
    break;

  case SCOPE:
    out << "SCOPE";
    break;

  case PLUSPLUS:
    out << "PLUSPLUS";
    break;

  case MINUSMINUS:
    out << "MINUSMINUS";
    break;

  case TIMESEQUAL:
    out << "TIMESEQUAL";
    break;

  case DIVIDEEQUAL:
    out << "DIVIDEEQUAL";
    break;

  case MODEQUAL:
    out << "MODEQUAL";
    break;

  case PLUSEQUAL:
    out << "PLUSEQUAL";
    break;

  case MINUSEQUAL:
    out << "MINUSEQUAL";
    break;

  case OREQUAL:
    out << "OREQUAL";
    break;

  case ANDEQUAL:
    out << "ANDEQUAL";
    break;

  case XOREQUAL:
    out << "XOREQUAL";
    break;

  case LSHIFTEQUAL:
    out << "LSHIFTEQUAL";
    break;

  case RSHIFTEQUAL:
    out << "RSHIFTEQUAL";
    break;

  case ATTR_LEFT:
    out << "ATTR_LEFT";
    break;

  case ATTR_RIGHT:
    out << "ATTR_RIGHT";
    break;

  case KW_ALIGNAS:
    out << "KW_ALIGNAS";
    break;

  case KW_ALIGNOF:
    out << "KW_ALIGNOF";
    break;

  case KW_AUTO:
    out << "KW_AUTO";
    break;

  case KW_BEGIN_PUBLISH:
    out << "KW_BEGIN_PUBLISH";
    break;

  case KW_BLOCKING:
    out << "KW_BLOCKING";
    break;

  case KW_BOOL:
    out << "KW_BOOL";
    break;

  case KW_BUILTIN_VA_LIST:
    out << "KW_BUILTIN_VA_LIST";
    break;

  case KW_CATCH:
    out << "KW_CATCH";
    break;

  case KW_CHAR:
    out << "KW_CHAR";
    break;

  case KW_CHAR8_T:
    out << "KW_CHAR8_T";
    break;

  case KW_CHAR16_T:
    out << "KW_CHAR16_T";
    break;

  case KW_CHAR32_T:
    out << "KW_CHAR32_T";
    break;

  case KW_CLASS:
    out << "KW_CLASS";
    break;

  case KW_CONST:
    out << "KW_CONST";
    break;

  case KW_CONSTEVAL:
    out << "KW_CONSTEVAL";
    break;

  case KW_CONSTEXPR:
    out << "KW_CONSTEXPR";
    break;

  case KW_CONSTINIT:
    out << "KW_CONSTINIT";
    break;

  case KW_DECLTYPE:
    out << "KW_DECLTYPE";
    break;

  case KW_DEFAULT:
    out << "KW_DECLTYPE";
    break;

  case KW_DELETE:
    out << "KW_DELETE";
    break;

  case KW_DOUBLE:
    out << "KW_DOUBLE";
    break;

  case KW_DYNAMIC_CAST:
    out << "KW_DYNAMIC_CAST";
    break;

  case KW_ELSE:
    out << "KW_ELSE";
    break;

  case KW_ENUM:
    out << "KW_ENUM";
    break;

  case KW_EXPLICIT:
    out << "KW_EXPLICIT";
    break;

  case KW_EXPLICIT_LPAREN:
    out << "KW_EXPLICIT_LPAREN";
    break;

  case KW_EXTENSION:
    out << "KW_EXTENSION";
    break;

  case KW_EXTERN:
    out << "KW_EXTERN";
    break;

  case KW_FALSE:
    out << "KW_FALSE";
    break;

  case KW_FINAL:
    out << "KW_FINAL";
    break;

  case KW_FLOAT:
    out << "KW_FLOAT";
    break;

  case KW_FRIEND:
    out << "KW_FRIEND";
    break;

  case KW_FOR:
    out << "KW_FOR";
    break;

  case KW_GOTO:
    out << "KW_GOTO";
    break;

  case KW_HAS_VIRTUAL_DESTRUCTOR:
    out << "KW_HAS_VIRTUAL_DESTRUCTOR";
    break;

  case KW_IF:
    out << "KW_IF";
    break;

  case KW_INLINE:
    out << "KW_INLINE";
    break;

  case KW_INT:
    out << "KW_INT";
    break;

  case KW_IS_ABSTRACT:
    out << "KW_IS_ABSTRACT";
    break;

  case KW_IS_BASE_OF:
    out << "KW_IS_BASE_OF";
    break;

  case KW_IS_CLASS:
    out << "KW_IS_CLASS";
    break;

  case KW_IS_CONSTRUCTIBLE:
    out << "KW_IS_CONSTRUCTIBLE";
    break;

  case KW_IS_CONVERTIBLE_TO:
    out << "KW_IS_CONVERTIBLE_TO";
    break;

  case KW_IS_DESTRUCTIBLE:
    out << "KW_IS_DESTRUCTIBLE";
    break;

  case KW_IS_EMPTY:
    out << "KW_IS_EMPTY";
    break;

  case KW_IS_ENUM:
    out << "KW_IS_ENUM";
    break;

  case KW_IS_FINAL:
    out << "KW_IS_FINAL";
    break;

  case KW_IS_FUNDAMENTAL:
    out << "KW_IS_FUNDAMENTAL";
    break;

  case KW_IS_POD:
    out << "KW_IS_POD";
    break;

  case KW_IS_POLYMORPHIC:
    out << "KW_IS_POLYMORPHIC";
    break;

  case KW_IS_STANDARD_LAYOUT:
    out << "KW_IS_STANDARD_LAYOUT";
    break;

  case KW_IS_TRIVIAL:
    out << "KW_IS_TRIVIAL";
    break;

  case KW_IS_TRIVIALLY_COPYABLE:
    out << "KW_IS_TRIVIALLY_COPYABLE";
    break;

  case KW_IS_UNION:
    out << "KW_IS_UNION";
    break;

  case KW_LONG:
    out << "KW_LONG";
    break;

  case KW_MAKE_MAP_KEYS_SEQ:
    out << "KW_MAKE_MAP_KEYS_SEQ";
    break;

  case KW_MAKE_MAP_PROPERTY:
    out << "KW_MAKE_MAP_PROPERTY";
    break;

  case KW_MAKE_PROPERTY:
    out << "KW_MAKE_PROPERTY";
    break;

  case KW_MAKE_PROPERTY2:
    out << "KW_MAKE_PROPERTY2";
    break;

  case KW_MAKE_SEQ:
    out << "KW_MAKE_SEQ";
    break;

  case KW_MAKE_SEQ_PROPERTY:
    out << "KW_MAKE_SEQ_PROPERTY";
    break;

  case KW_MUTABLE:
    out << "KW_MUTABLE";
    break;

  case KW_NAMESPACE:
    out << "KW_NAMESPACE";
    break;

  case KW_NEW:
    out << "KW_NEW";
    break;

  case KW_NOEXCEPT:
    out << "KW_NOEXCEPT";
    break;

  case KW_NOEXCEPT_LPAREN:
    out << "KW_NOEXCEPT_LPAREN";
    break;

  case KW_NULLPTR:
    out << "KW_NULLPTR";
    break;

  case KW_OPERATOR:
    if (_lval.u.identifier != nullptr) {
      out << *_lval.u.identifier << "::";
    }
    out << "KW_OPERATOR";
    break;

  case KW_OVERRIDE:
    out << "KW_OVERRIDE";
    break;

  case KW_PRIVATE:
    out << "KW_PRIVATE";
    break;

  case KW_PROTECTED:
    out << "KW_PROTECTED";
    break;

  case KW_PUBLIC:
    out << "KW_PUBLIC";
    break;

  case KW_PUBLISHED:
    out << "KW_PUBLISHED";
    break;

  case KW_REGISTER:
    out << "KW_REGISTER";
    break;

  case KW_REINTERPRET_CAST:
    out << "KW_REINTERPRET_CAST";
    break;

  case KW_RESTRICT:
    out << "KW_RESTRICT";
    break;

  case KW_RETURN:
    out << "KW_RETURN";
    break;

  case KW_SHORT:
    out << "KW_SHORT";
    break;

  case KW_SIGNED:
    out << "KW_SIGNED";
    break;

  case KW_SIZEOF:
    out << "KW_SIZEOF";
    break;

  case KW_STATIC:
    out << "KW_STATIC";
    break;

  case KW_STATIC_ASSERT:
    out << "KW_STATIC_ASSERT";
    break;

  case KW_STATIC_CAST:
    out << "KW_STATIC_CAST";
    break;

  case KW_STRUCT:
    out << "KW_STRUCT";
    break;

  case KW_TEMPLATE:
    out << "KW_TEMPLATE";
    break;

  case KW_THREAD_LOCAL:
    out << "KW_THREAD_LOCAL";
    break;

  case KW_THROW:
    out << "KW_THROW";
    break;

  case KW_TRUE:
    out << "KW_TRUE";
    break;

  case KW_TRY:
    out << "KW_TRY";
    break;

  case KW_TYPEDEF:
    out << "KW_TYPEDEF";
    break;

  case KW_TYPEID:
    out << "KW_TYPEID";
    break;

  case KW_TYPENAME:
    out << "KW_TYPENAME";
    break;

  case KW_UNDERLYING_TYPE:
    out << "KW_UNDERLYING_TYPE";
    break;

  case KW_USING:
    out << "KW_USING";
    break;

  case KW_UNION:
    out << "KW_UNION";
    break;

  case KW_UNSIGNED:
    out << "KW_UNSIGNED";
    break;

  case KW_VIRTUAL:
    out << "KW_VIRTUAL";
    break;

  case KW_VOID:
    out << "KW_VOID";
    break;

  case KW_VOLATILE:
    out << "KW_VOLATILE";
    break;

  case KW_WCHAR_T:
    out << "KW_WCHAR_T";
    break;

  case KW_WHILE:
    out << "KW_WHILE";
    break;

  case START_CPP:
    out << "START_CPP";
    break;

  case START_CONST_EXPR:
    out << "START_CONST_EXPR";
    break;

  case START_TYPE:
    out << "START_TYPE";
    break;

  default:
    if (_token < 128 && isprint(_token)) {
      out << "'" << (char)_token << "'";
    } else {
      out << "token " << _token << "\n";
    }
  }
}

/**
 *
 */
void CPPToken::
output_code(std::ostream &out) const {
  switch (_token) {
  case REAL:
    {
      char buffer[128];
      pdtoa(_lval.u.real, buffer);
      out << buffer;
    }
    break;

  case INTEGER:
    out << _lval.u.integer;
    break;

  case CHAR_TOK:
  case SIMPLE_STRING:
    out << (_token == CHAR_TOK ? '\'' : '"');
    for (char c : _lval.str) {
      switch (c) {
      case '\n':
        out << "\\n";
        break;

      case '\t':
        out << "\\t";
        break;

      case '\r':
        out << "\\r";
        break;

      case '\a':
        out << "\\a";
        break;

      case '\b':
        out << "\\b";
        break;

      case '\v':
        out << "\\v";
        break;

      case '\f':
        out << "\\f";
        break;

      case '\'':
        out << (_token == CHAR_TOK ? "\\\'" : "'");
        break;

      case '"':
        out << (_token == CHAR_TOK ? "\"" : "\\\"");
        break;

      case '\\':
        out << "\\\\";
        break;

      default:
        if (isprint(c)) {
          out << c;
        } else {
          out << '\\' << std::oct << std::setw(3) << std::setfill('0') << (int)(c)
              << std::dec << std::setw(0);
        }
        break;
      }
    }
    out << (_token == CHAR_TOK ? '\'' : '"');
    break;

  case STRING_LITERAL:
    out << *_lval.u.expr;
    break;

  case SIMPLE_IDENTIFIER:
    out << _lval.str;
    break;

  case IDENTIFIER:
    out << *_lval.u.identifier;
    break;

  case TYPENAME_IDENTIFIER:
    out << *_lval.u.identifier;
    break;

  case SCOPING:
    out << *_lval.u.identifier << "::";
    break;

  case TYPEDEFNAME:
    out << _lval.str;
    break;

  case ELLIPSIS:
    out << "...";
    break;

  case OROR:
    out << "||";
    break;

  case ANDAND:
    out << "&&";
    break;

  case EQCOMPARE:
    out << "==";
    break;

  case NECOMPARE:
    out << "!=";
    break;

  case LECOMPARE:
    out << "<=";
    break;

  case GECOMPARE:
    out << ">=";
    break;

  case LSHIFT:
    out << "<<";
    break;

  case RSHIFT:
    out << ">>";
    break;

  case POINTSAT_STAR:
    out << "->*";
    break;

  case DOT_STAR:
    out << ".*";
    break;

  case UNARY_NOT:
    out << "!";
    break;

  case UNARY_MINUS:
    out << "-";
    break;

  case UNARY_PLUS:
    out << "+";
    break;

  case UNARY_NEGATE:
    out << "~";
    break;

  case UNARY_STAR:
    out << "*";
    break;

  case UNARY_REF:
    out << "&";
    break;

  case POINTSAT:
    out << "->";
    break;

  case SCOPE:
    out << "::";
    break;

  case PLUSPLUS:
    out << "++";
    break;

  case MINUSMINUS:
    out << "--";
    break;

  case TIMESEQUAL:
    out << "*=";
    break;

  case DIVIDEEQUAL:
    out << "/=";
    break;

  case MODEQUAL:
    out << "%=";
    break;

  case PLUSEQUAL:
    out << "+=";
    break;

  case MINUSEQUAL:
    out << "-=";
    break;

  case OREQUAL:
    out << "|=";
    break;

  case ANDEQUAL:
    out << "&=";
    break;

  case XOREQUAL:
    out << "^=";
    break;

  case LSHIFTEQUAL:
    out << "<<=";
    break;

  case RSHIFTEQUAL:
    out << ">>=";
    break;

  case ATTR_LEFT:
    out << "[[";
    break;

  case ATTR_RIGHT:
    out << "]]";
    break;

  case KW_ALIGNAS:
    out << "alignas";
    break;

  case KW_ALIGNOF:
    out << "alignof";
    break;

  case KW_AUTO:
    out << "auto";
    break;

  case KW_BEGIN_PUBLISH:
    out << "__begin_publish";
    break;

  case KW_BLOCKING:
    out << "__blocking";
    break;

  case KW_BOOL:
    out << "bool";
    break;

  case KW_BUILTIN_VA_LIST:
    out << "__builtin_va_list";
    break;

  case KW_CATCH:
    out << "catch";
    break;

  case KW_CHAR:
    out << "char";
    break;

  case KW_CHAR8_T:
    out << "char8_t";
    break;

  case KW_CHAR16_T:
    out << "char16_t";
    break;

  case KW_CHAR32_T:
    out << "char32_t";
    break;

  case KW_CLASS:
    out << "class";
    break;

  case KW_CONST:
    out << "const";
    break;

  case KW_CONSTEVAL:
    out << "consteval";
    break;

  case KW_CONSTEXPR:
    out << "constexpr";
    break;

  case KW_CONSTINIT:
    out << "constinit";
    break;

  case KW_CONST_CAST:
    out << "const_cast";
    break;

  case KW_DECLTYPE:
    out << "decltype";
    break;

  case KW_DEFAULT:
    out << "default";
    break;

  case KW_DELETE:
    out << "delete";
    break;

  case KW_DOUBLE:
    out << "double";
    break;

  case KW_DYNAMIC_CAST:
    out << "dynamic_cast";
    break;

  case KW_ELSE:
    out << "else";
    break;

  case KW_ENUM:
    out << "enum";
    break;

  case KW_EXPLICIT:
    out << "explicit";
    break;

  case KW_EXPLICIT_LPAREN:
    out << "explicit(";
    break;

  case KW_EXTENSION:
    out << "__extension";
    break;

  case KW_EXTERN:
    out << "extern";
    break;

  case KW_FALSE:
    out << "false";
    break;

  case KW_FINAL:
    out << "final";
    break;

  case KW_FLOAT:
    out << "float";
    break;

  case KW_FRIEND:
    out << "friend";
    break;

  case KW_FOR:
    out << "for";
    break;

  case KW_GOTO:
    out << "goto";
    break;

  case KW_HAS_VIRTUAL_DESTRUCTOR:
    out << "__has_virtual_destructor";
    break;

  case KW_IF:
    out << "if";
    break;

  case KW_INLINE:
    out << "inline";
    break;

  case KW_INT:
    out << "int";
    break;

  case KW_IS_ABSTRACT:
    out << "__is_abstract";
    break;

  case KW_IS_BASE_OF:
    out << "__is_base_of";
    break;

  case KW_IS_CLASS:
    out << "__is_class";
    break;

  case KW_IS_CONSTRUCTIBLE:
    out << "__is_constructible";
    break;

  case KW_IS_CONVERTIBLE_TO:
    out << "__is_convertible_to";
    break;

  case KW_IS_DESTRUCTIBLE:
    out << "__is_destructible";
    break;

  case KW_IS_EMPTY:
    out << "__is_empty";
    break;

  case KW_IS_ENUM:
    out << "__is_enum";
    break;

  case KW_IS_FINAL:
    out << "__is_final";
    break;

  case KW_IS_FUNDAMENTAL:
    out << "__is_fundamental";
    break;

  case KW_IS_POD:
    out << "__is_pod";
    break;

  case KW_IS_POLYMORPHIC:
    out << "__is_polymorphic";
    break;

  case KW_IS_STANDARD_LAYOUT:
    out << "__is_standard_layout";
    break;

  case KW_IS_TRIVIAL:
    out << "__is_trivial";
    break;

  case KW_IS_TRIVIALLY_COPYABLE:
    out << "__is_trivially_copyable";
    break;

  case KW_IS_UNION:
    out << "__is_union";
    break;

  case KW_LONG:
    out << "long";
    break;

  case KW_MAKE_MAP_KEYS_SEQ:
    out << "__make_map_keys_seq";
    break;

  case KW_MAKE_MAP_PROPERTY:
    out << "__make_map_property";
    break;

  case KW_MAKE_PROPERTY:
    out << "__make_property";
    break;

  case KW_MAKE_PROPERTY2:
    out << "__make_property2";
    break;

  case KW_MAKE_SEQ:
    out << "__make_seq";
    break;

  case KW_MAKE_SEQ_PROPERTY:
    out << "__make_seq_property";
    break;

  case KW_MUTABLE:
    out << "mutable";
    break;

  case KW_NAMESPACE:
    out << "namespace";
    break;

  case KW_NEW:
    out << "new";
    break;

  case KW_NOEXCEPT:
    out << "noexcept";
    break;

  case KW_NOEXCEPT_LPAREN:
    out << "noexcept(";
    break;

  case KW_NULLPTR:
    out << "nullptr";
    break;

  case KW_OPERATOR:
    if (_lval.u.identifier != nullptr) {
      out << *_lval.u.identifier << "::";
    }
    out << "operator";
    break;

  case KW_OVERRIDE:
    out << "override";
    break;

  case KW_PRIVATE:
    out << "private";
    break;

  case KW_PROTECTED:
    out << "protected";
    break;

  case KW_PUBLIC:
    out << "public";
    break;

  case KW_PUBLISHED:
    out << "__published";
    break;

  case KW_REGISTER:
    out << "register";
    break;

  case KW_REINTERPRET_CAST:
    out << "reinterpret_cast";
    break;

  case KW_RESTRICT:
    out << "__restrict";
    break;

  case KW_RETURN:
    out << "return";
    break;

  case KW_SHORT:
    out << "short";
    break;

  case KW_SIGNED:
    out << "signed";
    break;

  case KW_SIZEOF:
    out << "sizeof";
    break;

  case KW_STATIC:
    out << "static";
    break;

  case KW_STATIC_ASSERT:
    out << "static_assert";
    break;

  case KW_STATIC_CAST:
    out << "static_cast";
    break;

  case KW_STRUCT:
    out << "struct";
    break;

  case KW_TEMPLATE:
    out << "template";
    break;

  case KW_THREAD_LOCAL:
    out << "thread_local";
    break;

  case KW_THROW:
    out << "throw";
    break;

  case KW_TRUE:
    out << "true";
    break;

  case KW_TRY:
    out << "try";
    break;

  case KW_TYPEDEF:
    out << "typedef";
    break;

  case KW_TYPEID:
    out << "typeid";
    break;

  case KW_TYPENAME:
    out << "typename";
    break;

  case KW_UNDERLYING_TYPE:
    out << "__underlying_type";
    break;

  case KW_USING:
    out << "using";
    break;

  case KW_UNION:
    out << "union";
    break;

  case KW_UNSIGNED:
    out << "unsigned";
    break;

  case KW_VIRTUAL:
    out << "virtual";
    break;

  case KW_VOID:
    out << "void";
    break;

  case KW_VOLATILE:
    out << "volatile";
    break;

  case KW_WCHAR_T:
    out << "wchar_t";
    break;

  case KW_WHILE:
    out << "while";
    break;

  case START_CPP:
  case START_CONST_EXPR:
  case START_TYPE:
    break;

  default:
    if (_token < 128 && isprint(_token)) {
      out << (char)_token;
    } else {
      out << "<token " << _token << ">\n";
    }
  }
}
