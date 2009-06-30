// Filename: p3dFileParams.cxx
// Created by:  drose (23Jun09)
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


#include "p3dFileParams.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DFileParams::
P3DFileParams() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DFileParams::
P3DFileParams(const string &p3d_filename, 
              const P3D_token tokens[], size_t num_tokens) :
  _p3d_filename(p3d_filename)
{
  for (size_t i = 0; i < num_tokens; ++i) {
    Token token;
    if (tokens[i]._keyword != NULL) {
      token._keyword = tokens[i]._keyword;
    }
    if (tokens[i]._value != NULL) {
      token._value = tokens[i]._value;
    }
    _tokens.push_back(token);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::Copy Assignment
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DFileParams::
operator = (const P3DFileParams &other) {
  _p3d_filename = other._p3d_filename;
  _tokens = other._tokens;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::lookup_token
//       Access: Public
//  Description: Returns the value associated with the first
//               appearance of the named token, or empty string if the
//               token does not appear.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::has_token
//       Access: Public
//  Description: Returns true if the named token appears in the list,
//               false otherwise.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::make_xml
//       Access: Public
//  Description: Returns a newly-allocated XML structure that
//               corresponds to the file parameter data within this
//               instance.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DFileParams::
make_xml() {
  TiXmlElement *xfparams = new TiXmlElement("fparams");

  xfparams->SetAttribute("p3d_filename", _p3d_filename.c_str());

  Tokens::const_iterator ti;
  for (ti = _tokens.begin(); ti != _tokens.end(); ++ti) {
    const Token &token = (*ti);
    TiXmlElement *xtoken = new TiXmlElement("token");
    xtoken->SetAttribute("keyword", token._keyword.c_str());
    xtoken->SetAttribute("value", token._value.c_str());
    xfparams->LinkEndChild(xtoken);
  }

  return xfparams;
}
