/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpDate.h
 * @author drose
 * @date 2003-01-28
 */

#ifndef HTTPDATE_H
#define HTTPDATE_H

#include "pandabase.h"

#include <time.h>

/**
 * A container for an HTTP-legal time/date indication.  This can accept a
 * string from an HTTP header and will decode it into a C time_t value;
 * conversely, it can accept a time_t value and encode it for output as a
 * string.
 */
class EXPCL_PANDA_DOWNLOADER HTTPDate {
PUBLISHED:
  INLINE HTTPDate();
  INLINE HTTPDate(time_t time);
  HTTPDate(const std::string &format);
  INLINE HTTPDate(const HTTPDate &copy);
  INLINE void operator = (const HTTPDate &copy);
  INLINE static HTTPDate now();

  INLINE bool is_valid() const;

  std::string get_string() const;
  INLINE time_t get_time() const;

  INLINE bool operator == (const HTTPDate &other) const;
  INLINE bool operator != (const HTTPDate &other) const;
  INLINE bool operator < (const HTTPDate &other) const;
  INLINE bool operator > (const HTTPDate &other) const;
  INLINE int compare_to(const HTTPDate &other) const;

  INLINE void operator += (int seconds);
  INLINE void operator -= (int seconds);

  INLINE HTTPDate operator + (int seconds) const;
  INLINE HTTPDate operator - (int seconds) const;
  INLINE int operator - (const HTTPDate &other) const;

  bool input(std::istream &in);
  void output(std::ostream &out) const;

private:
  static std::string get_token(const std::string &str, size_t &pos);

  time_t _time;
};

INLINE std::istream &operator >> (std::istream &in, HTTPDate &date);
INLINE std::ostream &operator << (std::ostream &out, const HTTPDate &date);

#include "httpDate.I"

#endif
