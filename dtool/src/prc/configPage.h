/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configPage.h
 * @author drose
 * @date 2004-10-15
 */

#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include "dtoolbase.h"

#include <vector>

class ConfigDeclaration;
class ConfigVariableCore;

/**
 * A page of ConfigDeclarations that may be loaded or unloaded.  Typically
 * this represents a single .prc file that is read from disk at runtime, but
 * it may also represent a list of declarations built up by application code
 * and explicitly loaded.
 */
class EXPCL_DTOOL_PRC ConfigPage {
private:
  ConfigPage(const std::string &name, bool implicit_load, int page_seq);
  ~ConfigPage();

public:
  INLINE bool operator < (const ConfigPage &other) const;

PUBLISHED:
  static ConfigPage *get_default_page();
  static ConfigPage *get_local_page();

  INLINE const std::string &get_name() const;
  MAKE_PROPERTY(name, get_name);

  INLINE bool is_special() const;
  INLINE bool is_implicit() const;
  MAKE_PROPERTY(special, is_special);
  MAKE_PROPERTY(implicit, is_implicit);

  void set_sort(int sort);
  INLINE int get_sort() const;
  MAKE_PROPERTY(sort, get_sort, set_sort);

  INLINE int get_page_seq() const;
  INLINE int get_trust_level() const;
  INLINE void set_trust_level(int trust_level);
  INLINE const std::string &get_signature() const;
  MAKE_PROPERTY(page_seq, get_page_seq);
  MAKE_PROPERTY(trust_level, get_trust_level, set_trust_level);
  MAKE_PROPERTY(signature, get_signature);

  void clear();
  bool read_prc(std::istream &in);
  bool read_encrypted_prc(std::istream &in, const std::string &password);

  ConfigDeclaration *make_declaration(const std::string &variable, const std::string &value);
  ConfigDeclaration *make_declaration(ConfigVariableCore *variable, const std::string &value);
  bool delete_declaration(ConfigDeclaration *decl);

  size_t get_num_declarations() const;
  const ConfigDeclaration *get_declaration(size_t n) const;
  ConfigDeclaration *modify_declaration(size_t n);
  std::string get_variable_name(size_t n) const;
  std::string get_string_value(size_t n) const;
  bool is_variable_used(size_t n) const;

  MAKE_SEQ_PROPERTY(declarations, get_num_declarations, modify_declaration);

  void output(std::ostream &out) const;
  void output_brief_signature(std::ostream &out) const;
  void write(std::ostream &out) const;

private:
  INLINE void make_dirty();
  void read_prc_line(const std::string &line);
  static unsigned int hex_digit(unsigned char digit);

  std::string _name;
  bool _implicit_load;
  int _page_seq;
  int _sort;
  int _next_decl_seq;
  int _trust_level;

  typedef std::vector<ConfigDeclaration *> Declarations;
  Declarations _declarations;

  std::string _signature;

#ifdef HAVE_OPENSSL
  // This maintains the hash of the prc file as we are scanning it, so we can
  // compare its signature which we discover at the end.
  void *_md_ctx;
#endif  // HAVE_OPENSSL

  static ConfigPage *_default_page;
  static ConfigPage *_local_page;

  friend class ConfigPageManager;
};

INLINE std::ostream &operator << (std::ostream &out, const ConfigPage &page);

#include "configPage.I"

#endif
