// Filename: configDeclaration.h
// Created by:  drose (15Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIGDECLARATION_H
#define CONFIGDECLARATION_H

#include "dtoolbase.h"
#include "configPage.h"
#include "vector_string.h"

class ConfigVariableCore;

////////////////////////////////////////////////////////////////////
//       Class : ConfigDeclaration
// Description : A single declaration of a config variable, typically
//               defined as one line in a .prc file,
//               e.g. "show-frame-rate-meter 1".  This is really just
//               a pairing of a string name (actually, a
//               ConfigVariableCore pointer) to a string value.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigDeclaration {
private:
  ConfigDeclaration(ConfigPage *page, ConfigVariableCore *variable,
                    const string &string_value, int decl_seq);
  ~ConfigDeclaration();

public:
  INLINE bool operator < (const ConfigDeclaration &other) const;

public:
  INLINE ConfigPage *get_page() const;
  INLINE ConfigVariableCore *get_variable() const;

  INLINE const string &get_string_value() const;
  INLINE void set_string_value(const string &value);

  INLINE int get_num_words() const;

  INLINE bool has_string_word(int n) const;
  INLINE bool has_bool_word(int n) const;
  INLINE bool has_int_word(int n) const;
  INLINE bool has_double_word(int n) const;

  INLINE string get_string_word(int n) const;
  INLINE bool get_bool_word(int n) const;
  INLINE int get_int_word(int n) const;
  INLINE double get_double_word(int n) const;

  void set_string_word(int n, const string &value);
  void set_bool_word(int n, bool value);
  void set_int_word(int n, int value);
  void set_double_word(int n, double value);

  INLINE int get_decl_seq() const;

  void output(ostream &out) const;
  void write(ostream &out) const;

public:
  static int extract_words(const string &str, vector_string &words);
  static string downcase(const string &s);

private:
  void get_words();
  void check_bool_word(int n);
  void check_int_word(int n);
  void check_double_word(int n);

private:
  ConfigPage *_page;
  ConfigVariableCore *_variable;
  string _string_value;
  int _decl_seq;

  enum WordFlags {
    F_checked_bool   = 0x0001,
    F_valid_bool     = 0x0002,
    F_checked_int    = 0x0004,
    F_valid_int      = 0x0008,
    F_checked_double = 0x0010,
    F_valid_double   = 0x0020,
  };

  class Word {
  public:
    string _str;
    bool _bool;
    int _int;
    double _double;
    short _flags;
  };

  typedef pvector<Word> Words;
  Words _words;
  bool _got_words;

  friend class ConfigPage;
};

INLINE ostream &operator << (ostream &out, const ConfigDeclaration &decl);

#include "configDeclaration.I"

#endif
