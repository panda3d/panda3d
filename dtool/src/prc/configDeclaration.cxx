// Filename: configDeclaration.cxx
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

#include "configDeclaration.h"
#include "configVariableCore.h"
#include "config_prc.h"
#include "pstrtod.h"
#include "string_utils.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::Constructor
//       Access: Private
//  Description: Use the ConfigPage::make_declaration() interface to
//               create a new declaration.
////////////////////////////////////////////////////////////////////
ConfigDeclaration::
ConfigDeclaration(ConfigPage *page, ConfigVariableCore *variable,
                  const string &string_value, int decl_seq) :
  _page(page),
  _variable(variable),
  _string_value(string_value),
  _decl_seq(decl_seq),
  _got_words(false)
{
  if (!_page->is_special()) {
    _variable->add_declaration(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::Destructor
//       Access: Private
//  Description: Use the ConfigPage::delete_declaration() interface to
//               delete a declaration.
////////////////////////////////////////////////////////////////////
ConfigDeclaration::
~ConfigDeclaration() {
  if (!_page->is_special()) {
    _variable->remove_declaration(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::set_string_word
//       Access: Public
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
set_string_word(int n, const string &value) {
  if (!_got_words) {
    get_words();
  }

  while (n >= (int)_words.size()) {
    Word w;
    w._flags = 0;
    _words.push_back(w);
  }

  _words[n]._str = value;
  _words[n]._flags = 0;

  // Now recompose the overall string value.
  Words::const_iterator wi = _words.begin();
  _string_value = (*wi)._str;
  ++wi;

  while (wi != _words.end()) {
    _string_value += " ";
    _string_value += (*wi)._str;
    ++wi;
  }
  invalidate_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::set_bool_word
//       Access: Public
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
set_bool_word(int n, bool value) {
  set_string_word(n, value ? "1" : "0");

  _words[n]._flags |= (F_checked_bool | F_valid_bool);
  _words[n]._bool = value;
  invalidate_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::set_int_word
//       Access: Public
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
set_int_word(int n, int value) {
  set_string_word(n, format_string(value));

  _words[n]._flags |= (F_checked_int | F_valid_int);
  _words[n]._int = value;
  invalidate_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::set_int64_word
//       Access: Public
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
set_int64_word(int n, PN_int64 value) {
  set_string_word(n, format_string(value));

  _words[n]._flags |= (F_checked_int64 | F_valid_int64);
  _words[n]._int_64 = value;
  invalidate_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::set_double_word
//       Access: Public
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
set_double_word(int n, double value) {
  set_string_word(n, format_string(value));

  _words[n]._flags |= (F_checked_double | F_valid_double);
  _words[n]._double = value;
  invalidate_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
output(ostream &out) const {
  out << get_variable()->get_name() << " " << get_string_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
write(ostream &out) const {
  out << get_variable()->get_name() << " " << get_string_value();
  if (!get_variable()->is_used()) {
    out << "  (not used)";
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::get_words
//       Access: Private
//  Description: Separates the string value into words.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
get_words() {
  if (!_got_words) {
    _words.clear();
    vector_string words;
    extract_words(_string_value, words);
    _words.reserve(words.size());

    for (vector_string::const_iterator wi = words.begin();
         wi != words.end();
         ++wi) {
      Word w;
      w._str = (*wi);
      w._flags = 0;
      _words.push_back(w);
    }

    _got_words = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::check_bool_word
//       Access: Private
//  Description: Checks whether the nth word can be interpreted as a
//               boolean value.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
check_bool_word(int n) {
  if (!_got_words) {
    get_words();
  }

  if (n >= 0 && n < (int)_words.size()) {
    Word &word = _words[n];
    if ((word._flags & F_checked_bool) == 0) {
      word._flags |= F_checked_bool;

      string s = downcase(word._str);
      if (s.empty()) {
        word._bool = false;

      } else if (s == "#t" || s == "1" || s[0] == 't') {
        word._bool = true;

      } else if (s == "#f" || s == "0" || s[0] == 'f') {
        word._bool = false;

      } else {
        // Not a recognized bool value.
        check_double_word(n);
        if ((word._flags & F_checked_double) != 0) {
          word._bool = (word._double != 0.0);
        } else {
          word._bool = false;
        }

        prc_cat->warning()
          << "Invalid bool value for ConfigVariable "
          << get_variable()->get_name() << ": " << word._str << "\n";
        return;
      }

      word._flags |= F_valid_bool;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::check_int_word
//       Access: Private
//  Description: Checks whether the nth word can be interpreted as an
//               integer value.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
check_int_word(int n) {
  if (!_got_words) {
    get_words();
  }

  if (n >= 0 && n < (int)_words.size()) {
    Word &word = _words[n];
    if ((word._flags & F_checked_int) == 0) {
      word._flags |= F_checked_int;

      // We scan the word by hand, rather than relying on strtol(), so
      // we can check for overflow of the 32-bit value.
      word._int = 0;
      bool overflow = false;

      string::const_iterator pi = word._str.begin();
      if (pi != word._str.end() && (*pi) == '-') {
        ++pi;
        // Negative number.
        while (pi != word._str.end() && isdigit(*pi)) {
          int next = word._int * 10 - (int)((*pi) - '0');
          if ((int)(next / 10) != word._int) {
            // Overflow.
            overflow = true;
          }
          word._int = next;
          ++pi;
        }

      } else {
        // Positive number.
        while (pi != word._str.end() && isdigit(*pi)) {
          int next = word._int * 10 + (int)((*pi) - '0');
          if ((int)(next / 10) != word._int) {
            // Overflow.
            overflow = true;
          }
          word._int = next;
          ++pi;
        }
      }

      if (pi == word._str.end() && !overflow) {
        word._flags |= F_valid_int;
      } else {
        prc_cat->warning()
          << "Invalid integer value for ConfigVariable "
          << get_variable()->get_name() << ": " << word._str << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::check_int64_word
//       Access: Private
//  Description: Checks whether the nth word can be interpreted as an
//               integer value.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
check_int64_word(int n) {
  if (!_got_words) {
    get_words();
  }

  if (n >= 0 && n < (int)_words.size()) {
    Word &word = _words[n];
    if ((word._flags & F_checked_int64) == 0) {
      word._flags |= F_checked_int64;

      word._int_64 = 0;
      bool overflow = false;

      string::const_iterator pi = word._str.begin();
      if (pi != word._str.end() && (*pi) == '-') {
        ++pi;
        // Negative number.
        while (pi != word._str.end() && isdigit(*pi)) {
          PN_int64 next = word._int_64 * 10 - (int)((*pi) - '0');
          if ((PN_int64)(next / 10) != word._int_64) {
            // Overflow.
            overflow = true;
          }
          word._int_64 = next;
          ++pi;
        }

      } else {
        // Positive number.
        while (pi != word._str.end() && isdigit(*pi)) {
          PN_int64 next = word._int_64 * 10 + (int)((*pi) - '0');
          if ((PN_int64)(next / 10) != word._int_64) {
            // Overflow.
            overflow = true;
          }
          word._int_64 = next;
          ++pi;
        }
      }

      if (pi == word._str.end() && !overflow) {
        word._flags |= F_valid_int64;
      } else {
        prc_cat->warning()
          << "Invalid int64 value for ConfigVariable "
          << get_variable()->get_name() << ": " << word._str << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::check_double_word
//       Access: Private
//  Description: Checks whether the nth word can be interpreted as a
//               floating-point value.
////////////////////////////////////////////////////////////////////
void ConfigDeclaration::
check_double_word(int n) {
  if (!_got_words) {
    get_words();
  }

  if (n >= 0 && n < (int)_words.size()) {
    Word &word = _words[n];
    if ((word._flags & F_checked_double) == 0) {
      word._flags |= F_checked_double;

      const char *nptr = word._str.c_str();
      char *endptr;
      word._double = pstrtod(nptr, &endptr);

      if (*endptr == '\0') {
        word._flags |= F_valid_double;
      } else {
        prc_cat->warning()
          << "Invalid floating-point value for ConfigVariable "
          << get_variable()->get_name() << ": " << word._str << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::extract_words
//       Access: Public, Static
//  Description: Divides the string into a number of words according
//               to whitespace.  The words vector should be cleared by
//               the user before calling; otherwise, the list of words
//               in the string will be appended to the end of whatever
//               was there before.
//
//               The return value is the number of words extracted.
////////////////////////////////////////////////////////////////////
int ConfigDeclaration::
extract_words(const string &str, vector_string &words) {
  int num_words = 0;

  size_t pos = 0;
  while (pos < str.length() && isspace((unsigned int)str[pos])) {
    pos++;
  }
  while (pos < str.length()) {
    size_t word_start = pos;
    while (pos < str.length() && !isspace((unsigned int)str[pos])) {
      pos++;
    }
    words.push_back(str.substr(word_start, pos - word_start));
    num_words++;

    while (pos < str.length() && isspace((unsigned int)str[pos])) {
      pos++;
    }
  }

  return num_words;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigDeclaration::downcase
//       Access: Public, Static
//  Description: Returns the input string with all uppercase letters
//               converted to lowercase.
////////////////////////////////////////////////////////////////////
string ConfigDeclaration::
downcase(const string &s) {
  string result;
  result.reserve(s.size());
  string::const_iterator p;
  for (p = s.begin(); p != s.end(); ++p) {
    result += tolower(*p);
  }
  return result;
}
