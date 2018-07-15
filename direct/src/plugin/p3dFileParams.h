/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dFileParams.h
 * @author drose
 * @date 2009-06-23
 */

#ifndef P3DFILEPARAMS_H
#define P3DFILEPARAMS_H

#include "p3d_plugin_common.h"
#include "get_tinyxml.h"
#include <vector>

/**
 * Encapsulates the file parameters: the p3d_filename, and extra tokens.
 */
class P3DFileParams {
public:
  P3DFileParams();
  P3DFileParams(const P3DFileParams &copy);
  void operator = (const P3DFileParams &other);

  void set_p3d_filename(const std::string &p3d_filename);
  void set_p3d_offset(const int &p3d_offset);
  void set_p3d_url(const std::string &p3d_url);
  void set_tokens(const P3D_token tokens[], size_t num_tokens);
  void set_token(const char *keyword, const char *value);
  void set_args(int argc, const char *argv[]);

  inline const std::string &get_p3d_filename() const;
  inline int get_p3d_offset() const;
  inline const std::string &get_p3d_url() const;
  std::string lookup_token(const std::string &keyword) const;
  int lookup_token_int(const std::string &keyword) const;
  bool has_token(const std::string &keyword) const;

  inline int get_num_tokens() const;
  inline const std::string &get_token_keyword(int n) const;
  inline const std::string &get_token_value(int n) const;

  TiXmlElement *make_xml();

private:
  class Token {
  public:
    std::string _keyword;
    std::string _value;
  };
  typedef std::vector<Token> Tokens;
  typedef std::vector<std::string> Args;

  std::string _p3d_filename;
  int _p3d_offset;
  std::string _p3d_url;
  Tokens _tokens;
  Args _args;
};

#include "p3dFileParams.I"

#endif
