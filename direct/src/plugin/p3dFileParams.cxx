/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dFileParams.cxx
 * @author drose
 * @date 2009-06-23
 */

#include "p3dFileParams.h"
#include <ctype.h>

using std::string;

/**
 *
 */
P3DFileParams::
P3DFileParams() : _p3d_offset(0) {
}

/**
 *
 */
P3DFileParams::
P3DFileParams(const P3DFileParams &copy) :
  _p3d_filename(copy._p3d_filename),
  _p3d_offset(copy._p3d_offset),
  _p3d_url(copy._p3d_url),
  _tokens(copy._tokens),
  _args(copy._args)
{
}

/**
 *
 */
void P3DFileParams::
operator = (const P3DFileParams &other) {
  _p3d_filename = other._p3d_filename;
  _p3d_offset = other._p3d_offset;
  _p3d_url = other._p3d_url;
  _tokens = other._tokens;
  _args = other._args;
}

/**
 * Specifies the file that contains the instance data.
 */
void P3DFileParams::
set_p3d_filename(const string &p3d_filename) {
  _p3d_filename = p3d_filename;
}

/**
 * Specifies the location in the file where the p3d file data starts.
 */
void P3DFileParams::
set_p3d_offset(const int &p3d_offset) {
  _p3d_offset = p3d_offset;
}

/**
 * Specifies the original URL that hosted the p3d file, if any.  This is for
 * documentation purposes only; it is communicated to the child Panda3D
 * process.
 */
void P3DFileParams::
set_p3d_url(const string &p3d_url) {
  _p3d_url = p3d_url;
}

/**
 * Replaces all the tokens associated with the instance.
 */
void P3DFileParams::
set_tokens(const P3D_token tokens[], size_t num_tokens) {
  _tokens.clear();

  for (size_t i = 0; i < num_tokens; ++i) {
    set_token(tokens[i]._keyword, tokens[i]._value);
  }
}

/**
 * Sets an individual token value.
 */
void P3DFileParams::
set_token(const char *keyword, const char *value) {
  Token token;
  if (keyword != nullptr) {
    // Make the token lowercase, since HTML is case-insensitive but we're not.
    for (const char *p = keyword; *p; ++p) {
      token._keyword += tolower(*p);
    }
  }
  if (value != nullptr) {
    token._value = value;
  }
  _tokens.push_back(token);
}

/**
 * Specifies the command-line arguments associated with the instance.
 */
void P3DFileParams::
set_args(int argc, const char *argv[]) {
  _args.clear();

  for (int i = 0; i < argc; ++i) {
    const char *arg = argv[i];
    if (arg == nullptr) {
      arg = "";
    }
    _args.push_back(arg);
  }
}

/**
 * Returns the value associated with the first appearance of the named token,
 * or empty string if the token does not appear.
 */
string P3DFileParams::
lookup_token(const string &keyword) const {
  Tokens::const_iterator ti;
  for (ti = _tokens.begin(); ti != _tokens.end(); ++ti) {
    if ((*ti)._keyword == keyword) {
      return (*ti)._value;
    }
  }

  return string();
}

/**
 * Returns the integer value associated with the first appearance of the named
 * token, or zero if the token does not appear or is not an integer.
 */
int P3DFileParams::
lookup_token_int(const string &keyword) const {
  string value = lookup_token(keyword);
  return atoi(value.c_str());
}

/**
 * Returns true if the named token appears in the list, false otherwise.
 */
bool P3DFileParams::
has_token(const string &keyword) const {
  Tokens::const_iterator ti;
  for (ti = _tokens.begin(); ti != _tokens.end(); ++ti) {
    if ((*ti)._keyword == keyword) {
      return true;
    }
  }

  return false;
}

/**
 * Returns a newly-allocated XML structure that corresponds to the file
 * parameter data within this instance.
 */
TiXmlElement *P3DFileParams::
make_xml() {
  TiXmlElement *xfparams = new TiXmlElement("fparams");

  xfparams->SetAttribute("p3d_filename", _p3d_filename);
  xfparams->SetAttribute("p3d_offset", _p3d_offset);
  xfparams->SetAttribute("p3d_url", _p3d_url);

  Tokens::const_iterator ti;
  for (ti = _tokens.begin(); ti != _tokens.end(); ++ti) {
    const Token &token = (*ti);
    TiXmlElement *xtoken = new TiXmlElement("token");
    xtoken->SetAttribute("keyword", token._keyword);
    xtoken->SetAttribute("value", token._value);
    xfparams->LinkEndChild(xtoken);
  }

  Args::const_iterator ai;
  for (ai = _args.begin(); ai != _args.end(); ++ai) {
    TiXmlElement *xarg = new TiXmlElement("arg");
    xarg->SetAttribute("value", (*ai));
    xfparams->LinkEndChild(xarg);
  }

  return xfparams;
}
