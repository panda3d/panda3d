// Filename: p3dFileParams.h
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

#ifndef P3DFILEPARAMS_H
#define P3DFILEPARAMS_H

#include "p3d_plugin_common.h"
#include "get_tinyxml.h"
#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : P3DFileParams
// Description : Encapsulates the file parameters: the p3d_filename,
//               and extra tokens.
////////////////////////////////////////////////////////////////////
class P3DFileParams {
public:
  P3DFileParams();
  P3DFileParams(const P3DFileParams &copy);
  void operator = (const P3DFileParams &other);

  void set_p3d_filename(const string &p3d_filename);
  void set_p3d_offset(const int &p3d_offset);
  void set_p3d_url(const string &p3d_url);
  void set_tokens(const P3D_token tokens[], size_t num_tokens);
  void set_token(const char *keyword, const char *value);
  void set_args(int argc, const char *argv[]);

  inline const string &get_p3d_filename() const;
  inline int get_p3d_offset() const;
  inline const string &get_p3d_url() const;
  string lookup_token(const string &keyword) const;
  int lookup_token_int(const string &keyword) const;
  bool has_token(const string &keyword) const;

  inline int get_num_tokens() const;
  inline const string &get_token_keyword(int n) const;
  inline const string &get_token_value(int n) const;

  TiXmlElement *make_xml();

private:
  class Token {
  public:
    string _keyword;
    string _value;
  };
  typedef vector<Token> Tokens;
  typedef vector<string> Args;

  string _p3d_filename;
  int _p3d_offset;
  string _p3d_url;
  Tokens _tokens;
  Args _args;
};

#include "p3dFileParams.I"

#endif
