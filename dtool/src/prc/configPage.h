// Filename: configPage.h
// Created by:  drose (15Oct04)
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

#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include "dtoolbase.h"

#include <vector>

class ConfigDeclaration;
class ConfigVariableCore;

////////////////////////////////////////////////////////////////////
//       Class : ConfigPage
// Description : A page of ConfigDeclarations that may be loaded or
//               unloaded.  Typically this represents a single .prc
//               file that is read from disk at runtime, but it may
//               also represent a list of declarations built up
//               by application code and explicitly loaded.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigPage {
private:
  ConfigPage(const string &name, bool implicit_load, int page_seq);
  ~ConfigPage();

public:
  INLINE bool operator < (const ConfigPage &other) const;

PUBLISHED:
  static ConfigPage *get_default_page();
  static ConfigPage *get_local_page();

  INLINE const string &get_name() const;

  INLINE bool is_special() const;
  INLINE bool is_implicit() const;

  void set_sort(int sort);
  INLINE int get_sort() const;

  INLINE int get_page_seq() const;
  INLINE int get_trust_level() const;
  INLINE void set_trust_level(int trust_level);
  INLINE const string &get_signature() const;

  void clear();
  bool read_prc(istream &in);
  bool read_encrypted_prc(istream &in, const string &password);

  ConfigDeclaration *make_declaration(const string &variable, const string &value);
  ConfigDeclaration *make_declaration(ConfigVariableCore *variable, const string &value);
  bool delete_declaration(ConfigDeclaration *decl);

  int get_num_declarations() const;
  const ConfigDeclaration *get_declaration(int n) const;
  ConfigDeclaration *modify_declaration(int n);
  string get_variable_name(int n) const;
  string get_string_value(int n) const;
  bool is_variable_used(int n) const;

  void output(ostream &out) const;
  void output_brief_signature(ostream &out) const;
  void write(ostream &out) const;

private:
  INLINE void make_dirty();
  void read_prc_line(const string &line);
  static unsigned int hex_digit(unsigned char digit);

  string _name;
  bool _implicit_load;
  int _page_seq;
  int _sort;
  int _next_decl_seq;
  int _trust_level;

  typedef vector<ConfigDeclaration *> Declarations;
  Declarations _declarations;

  string _signature;

#ifdef HAVE_OPENSSL
  // This maintains the hash of the prc file as we are scanning it, so
  // we can compare its signature which we discover at the end.
  void *_md_ctx;
#endif  // HAVE_OPENSSL

  static ConfigPage *_default_page;
  static ConfigPage *_local_page;

  friend class ConfigPageManager;
};

INLINE ostream &operator << (ostream &out, const ConfigPage &page);

#include "configPage.I"

#endif
