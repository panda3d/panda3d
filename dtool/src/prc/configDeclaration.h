/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configDeclaration.h
 * @author drose
 * @date 2004-10-15
 */

#ifndef CONFIGDECLARATION_H
#define CONFIGDECLARATION_H

#include "dtoolbase.h"
#include "configFlags.h"
#include "configPage.h"
#include "vector_string.h"
#include "numeric_types.h"
#include "filename.h"

#include <vector>

class ConfigVariableCore;

/**
 * A single declaration of a config variable, typically defined as one line in
 * a .prc file, e.g.  "show-frame-rate-meter 1".  This is really just a
 * pairing of a string name (actually, a ConfigVariableCore pointer) to a
 * string value.
 */
class EXPCL_DTOOL_PRC ConfigDeclaration : public ConfigFlags {
private:
  ConfigDeclaration(ConfigPage *page, ConfigVariableCore *variable,
                    const std::string &string_value, int decl_seq);
  ~ConfigDeclaration();

public:
  INLINE bool operator < (const ConfigDeclaration &other) const;

PUBLISHED:
  INLINE ConfigPage *get_page() const;
  INLINE ConfigVariableCore *get_variable() const;
  MAKE_PROPERTY(page, get_page);
  MAKE_PROPERTY(variable, get_variable);

  INLINE const std::string &get_string_value() const;
  INLINE void set_string_value(const std::string &value);

  INLINE size_t get_num_words() const;

  INLINE bool has_string_word(size_t n) const;
  INLINE bool has_bool_word(size_t n) const;
  INLINE bool has_int_word(size_t n) const;
  INLINE bool has_int64_word(size_t n) const;
  INLINE bool has_double_word(size_t n) const;

  INLINE std::string get_string_word(size_t n) const;
  INLINE bool get_bool_word(size_t n) const;
  INLINE int get_int_word(size_t n) const;
  INLINE int64_t get_int64_word(size_t n) const;
  INLINE double get_double_word(size_t n) const;

  void set_string_word(size_t n, const std::string &value);
  void set_bool_word(size_t n, bool value);
  void set_int_word(size_t n, int value);
  void set_int64_word(size_t n, int64_t value);
  void set_double_word(size_t n, double value);

  Filename get_filename_value() const;

  INLINE int get_decl_seq() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

public:
  static size_t extract_words(const std::string &str, vector_string &words);
  static std::string downcase(const std::string &s);

private:
  void get_words();
  void check_bool_word(size_t n);
  void check_int_word(size_t n);
  void check_int64_word(size_t n);
  void check_double_word(size_t n);

private:
  ConfigPage *_page;
  ConfigVariableCore *_variable;
  std::string _string_value;
  int _decl_seq;

  enum WordFlags {
    F_checked_bool   = 0x0001,
    F_valid_bool     = 0x0002,
    F_checked_int    = 0x0004,
    F_valid_int      = 0x0008,
    F_checked_double = 0x0010,
    F_valid_double   = 0x0020,
    F_checked_int64  = 0x0040,
    F_valid_int64    = 0x0080,
  };

  class Word {
  public:
    std::string _str;
    bool _bool;
    int _int;
    int64_t _int_64;
    double _double;
    short _flags;
  };

  typedef std::vector<Word> Words;
  Words _words;
  bool _got_words;

  friend class ConfigPage;
};

INLINE std::ostream &operator << (std::ostream &out, const ConfigDeclaration &decl);

#include "configDeclaration.I"

#endif
