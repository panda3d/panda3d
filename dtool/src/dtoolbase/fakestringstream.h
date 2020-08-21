/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fakestringstream.h
 * @author cary
 * @date 1999-02-04
 */

#ifndef FAKESTRINGSTREAM_H
#define FAKESTRINGSTREAM_H

#include <strstream.h>
#include <string.h>
#include <string>

class fake_istream_buffer {
public:
  fake_istream_buffer() {
    _len = 0;
    _str = "";
  }
  fake_istream_buffer(const std::string &source) {
    _len = source.length();
    if (_len == 0) {
      _str = "";
    } else {
      _str = new char[_len];
      memcpy(_str, source.data(), _len);
    }
  }
  ~fake_istream_buffer() {
    if (_len != 0) {
      delete[] _str;
    }
  }

  int _len;
  char *_str;
};

class std::istringstream : public fake_istream_buffer, public istrstream {
public:
  std::istringstream(const std::string &input) :
    fake_istream_buffer(input),
    istrstream(_str, _len) { }
};

class std::ostringstream : public ostrstream {
public:
  std::string str() {
    // We must capture the length before we take the str().
    int length = pcount();
    char *s = ostrstream::str();
    std::string result(s, length);
    delete[] s;
    return result;
  }
};

class stringstream : public fake_istream_buffer, public strstream {
public:
  stringstream() : strstream() {
    _owns_str = true;
  }
  std::stringstream(const std::string &input) :
    fake_istream_buffer(input),
    strstream(_str, _len, std::ios::in)
  {
    _owns_str = false;
  }

  // str() doesn't seem to compile cross-platform too reliably--Irix doesn't
  // define pcount() for some reason.  On the other hand, why are you calling
  // str() on a stringstream?  Just use an ostringstream.

  /*
  string str() {
    int length = pcount();
    char *s = strstream::str();
    string result(s, length);
    if (_owns_str) {
      delete[] s;
    }
    return result;
  }
  */

private:
  bool _owns_str;
};

#endif
