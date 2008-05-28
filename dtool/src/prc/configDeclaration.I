// Filename: configDeclaration.I
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


////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::operator <
//       Access: Public
//  Description: Sorts two declarations in order based on the order in
//               which their respective pages were loaded, and the
//               order in which they appear within the same page.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigDeclaration::
operator < (const ConfigDeclaration &other) const {
  if (get_page() == other.get_page()) {
    // On the same page, we compare based on the decl_seq.
    return get_decl_seq() < other.get_decl_seq();
  }

  // On different pages, we compare based on the page importance.
  return *get_page() < *other.get_page();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_page
//       Access: Public
//  Description: Returns the page on which this declaration can be
//               found.
////////////////////////////////////////////////////////////////////
INLINE ConfigPage *ConfigDeclaration::
get_page() const {
  return _page;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_variable
//       Access: Public
//  Description: Returns the variable that this declaration names.
//               This variable may or may not have been defined by the
//               time the declaration is read.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableCore *ConfigDeclaration::
get_variable() const {
  return _variable;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_string_value
//       Access: Public
//  Description: Returns the value assigned to this variable.  This is
//               the original one-line text defined for the variable
//               in the .prc file (or passed to
//               ConfigPage::make_declaration()).
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigDeclaration::
get_string_value() const {
  return _string_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::set_string_value
//       Access: Public
//  Description: Changes the value assigned to this variable.
////////////////////////////////////////////////////////////////////
INLINE void ConfigDeclaration::
set_string_value(const string &string_value) {
  _string_value = string_value;
  _got_words = false;
  invalidate_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_num_words
//       Access: Public
//  Description: Returns the number of words in the declaration's
//               value.  A word is defined as a sequence of
//               non-whitespace characters delimited by whitespace.
////////////////////////////////////////////////////////////////////
INLINE int ConfigDeclaration::
get_num_words() const {
  if (!_got_words) {
    ((ConfigDeclaration *)this)->get_words();
  }
  return _words.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::has_string_word
//       Access: Public
//  Description: Returns true if the declaration's value has a valid
//               string value for the nth word.  This is really the
//               same thing as asking if there are at least n words in
//               the value.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigDeclaration::
has_string_word(int n) const {
  if (!_got_words) {
    ((ConfigDeclaration *)this)->get_words();
  }
  return (n >= 0 && n < (int)_words.size());
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::has_bool_word
//       Access: Public
//  Description: Returns true if the declaration's value has a valid
//               boolean value for the nth word.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigDeclaration::
has_bool_word(int n) const {
  if (has_string_word(n)) {
    ((ConfigDeclaration *)this)->check_bool_word(n);
    return (_words[n]._flags & F_valid_bool) != 0;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::has_int_word
//       Access: Public
//  Description: Returns true if the declaration's value has a valid
//               integer value for the nth word.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigDeclaration::
has_int_word(int n) const {
  if (has_string_word(n)) {
    ((ConfigDeclaration *)this)->check_int_word(n);
    return (_words[n]._flags & F_valid_int) != 0;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::has_int64_word
//       Access: Public
//  Description: Returns true if the declaration's value has a valid
//               int64 value for the nth word.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigDeclaration::
has_int64_word(int n) const {
  if (has_string_word(n)) {
    ((ConfigDeclaration *)this)->check_int64_word(n);
    return (_words[n]._flags & F_valid_int64) != 0;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::has_double_word
//       Access: Public
//  Description: Returns true if the declaration's value has a valid
//               integer value for the nth word.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigDeclaration::
has_double_word(int n) const {
  if (has_string_word(n)) {
    ((ConfigDeclaration *)this)->check_double_word(n);
    return (_words[n]._flags & F_valid_double) != 0;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_string_word
//       Access: Public
//  Description: Returns the string value of the nth word of the
//               declaration's value, or empty string if there is no
//               nth value.  See also has_string_word().
////////////////////////////////////////////////////////////////////
INLINE string ConfigDeclaration::
get_string_word(int n) const {
  if (has_string_word(n)) {
    return _words[n]._str;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_bool_word
//       Access: Public
//  Description: Returns the boolean value of the nth word of the
//               declaration's value, or false if there is no nth
//               value.  See also has_bool_word().
////////////////////////////////////////////////////////////////////
INLINE bool ConfigDeclaration::
get_bool_word(int n) const {
  // We use has_string_word() instead of has_bool_word(), so we can
  // return a partial answer if there was one.
  if (has_string_word(n)) {
    ((ConfigDeclaration *)this)->check_bool_word(n);
    return _words[n]._bool;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_int_word
//       Access: Public
//  Description: Returns the integer value of the nth word of the
//               declaration's value, or 0 if there is no nth value.
//               See also has_int_word().
////////////////////////////////////////////////////////////////////
INLINE int ConfigDeclaration::
get_int_word(int n) const {
  // We use has_string_word() instead of has_int_word(), so we can
  // return a partial answer if there was one.
  if (has_string_word(n)) {
    ((ConfigDeclaration *)this)->check_int_word(n);
    return _words[n]._int;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_int64_word
//       Access: Public
//  Description: Returns the int64 value of the nth word of the
//               declaration's value, or 0 if there is no nth value.
//               See also has_int64_word().
////////////////////////////////////////////////////////////////////
INLINE PN_int64 ConfigDeclaration::
get_int64_word(int n) const {
  // We use has_string_word() instead of has_int64_word(), so we can
  // return a partial answer if there was one.
  if (has_string_word(n)) {
    ((ConfigDeclaration *)this)->check_int64_word(n);
    return _words[n]._int_64;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_double_word
//       Access: Public
//  Description: Returns the integer value of the nth word of the
//               declaration's value, or 0 if there is no nth value.
//               See also has_double_word().
////////////////////////////////////////////////////////////////////
INLINE double ConfigDeclaration::
get_double_word(int n) const {
  // We use has_string_word() instead of has_double_word(), so we can
  // return a partial answer if there was one.
  if (has_string_word(n)) {
    ((ConfigDeclaration *)this)->check_double_word(n);
    return _words[n]._double;
  }
  return 0.0;
}


////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_decl_seq
//       Access: Public
//  Description: Returns the sequence number of the declaration within
//               the page.  Sequence numbers are assigned as each
//               declaration is created; each declaration is given a
//               higher sequence number than all the declarations
//               created in the page before it.
////////////////////////////////////////////////////////////////////
INLINE int ConfigDeclaration::
get_decl_seq() const {
  return _decl_seq;
}

INLINE ostream &
operator << (ostream &out, const ConfigDeclaration &decl) {
  decl.output(out);
  return out;
}
