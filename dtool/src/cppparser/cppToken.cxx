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
 *
 */
CPPToken::
CPPToken(const CPPToken &copy) :
  _token(copy._token),
  _lloc(copy._lloc)
{
  _lval.str = copy._lval.str;
  _lval.u = copy._lval.u;
}

/**
 *
 */
void CPPToken::
operator = (const CPPToken &copy) {
  _token = copy._token;
  _lval.str = copy._lval.str;
  _lval.u = copy._lval.u;
  _lloc = copy._lloc;
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

  case KW_BOOL:
    out << "KW_BOOL";
    break;

  case KW_CATCH:
    out << "KW_CATCH";
    break;

  case KW_CHAR:
    out << "KW_CHAR";
    break;

  case KW_CLASS:
    out << "KW_CLASS";
    break;

  case KW_CONST:
    out << "KW_CONST";
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

  case KW_EXTERN:
    out << "KW_EXTERN";
    break;

  case KW_FALSE:
    out << "KW_FALSE";
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

  case KW_IF:
    out << "KW_IF";
    break;

  case KW_INLINE:
    out << "KW_INLINE";
    break;

  case KW_INT:
    out << "KW_INT";
    break;

  case KW_LONG:
    out << "KW_LONG";
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

  case KW_OPERATOR:
    if (_lval.u.identifier != nullptr) {
      out << *_lval.u.identifier << "::";
    }
    out << "KW_OPERATOR";
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

  case KW_REGISTER:
    out << "KW_REGISTER";
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

  case KW_STATIC_CAST:
    out << "KW_STATIC_CAST";
    break;

  case KW_STRUCT:
    out << "KW_STRUCT";
    break;

  case KW_TEMPLATE:
    out << "KW_TEMPLATE";
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

  case KW_TYPENAME:
    out << "KW_TYPENAME";
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
