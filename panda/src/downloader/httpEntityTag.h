// Filename: httpEntityTag.h
// Created by:  drose (28Jan03)
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

#ifndef HTTPENTITYTAG_H
#define HTTPENTITYTAG_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : HTTPEntityTag
// Description : A container for an "entity tag" from an HTTP server.
//               This is used to identify a particular version of a
//               document or resource, particularly useful for
//               verifying caches.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HTTPEntityTag {
PUBLISHED:
  INLINE HTTPEntityTag();
  HTTPEntityTag(const string &text);
  INLINE HTTPEntityTag(bool weak, const string &tag);
  INLINE HTTPEntityTag(const HTTPEntityTag &copy);
  INLINE void operator = (const HTTPEntityTag &copy);

  INLINE bool is_weak() const;
  INLINE const string &get_tag() const;
  string get_string() const;

  INLINE bool strong_equiv(const HTTPEntityTag &other) const;
  INLINE bool weak_equiv(const HTTPEntityTag &other) const;

  INLINE bool operator == (const HTTPEntityTag &other) const;
  INLINE bool operator != (const HTTPEntityTag &other) const;
  INLINE bool operator < (const HTTPEntityTag &other) const;
  INLINE int compare_to(const HTTPEntityTag &other) const;

  INLINE void output(ostream &out) const;

private:
  bool _weak;
  string _tag;
};

INLINE ostream &operator << (ostream &out, const HTTPEntityTag &url);

#include "httpEntityTag.I"

#endif
