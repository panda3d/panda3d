// Filename: notifyCategory.h
// Created by:  drose (29Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef NOTIFYCATEGORY_H
#define NOTIFYCATEGORY_H

#include "dtoolbase.h"

#include "notifySeverity.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : NotifyCategory
// Description : A particular category of error messages.  Typically
//               there will be one of these per package, so that we
//               can turn on or off error messages at least at a
//               package level; further nested categories can be
//               created within a package if a finer grain of control
//               is required.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG NotifyCategory {
private:
  NotifyCategory(const string &fullname, const string &basename,
                 NotifyCategory *parent);

PUBLISHED:
  INLINE string get_fullname() const;
  INLINE string get_basename() const;
  INLINE NotifySeverity get_severity() const;
  INLINE void set_severity(NotifySeverity severity);

  INLINE bool is_on(NotifySeverity severity) const;

  // When NOTIFY_DEBUG is not defined, the categories will never be
  // set to "spam" or "debug" severities, and these methods are
  // redefined to be static to make it more obvious to the compiler.
  // However, we still want to present a consistent interface to our
  // scripting language, so during the interrogate pass (that is, when
  // CPPPARSER is defined), we still pretend they're nonstatic.
#if defined(NOTIFY_DEBUG) || defined(CPPPARSER)
  INLINE bool is_spam() const;
  INLINE bool is_debug() const;
#else
  INLINE static bool is_spam();
  INLINE static bool is_debug();
#endif
  INLINE bool is_info() const;
  INLINE bool is_warning() const;
  INLINE bool is_error() const;
  INLINE bool is_fatal() const;

  ostream &out(NotifySeverity severity, bool prefix = true) const;
  INLINE ostream &spam(bool prefix = true) const;
  INLINE ostream &debug(bool prefix = true) const;
  INLINE ostream &info(bool prefix = true) const;
  INLINE ostream &warning(bool prefix = true) const;
  INLINE ostream &error(bool prefix = true) const;
  INLINE ostream &fatal(bool prefix = true) const;

  int get_num_children() const;
  NotifyCategory *get_child(int i) const;

private:
  string _fullname;
  string _basename;
  NotifyCategory *_parent;
  NotifySeverity _severity;
  typedef vector<NotifyCategory *> Children;
  Children _children;

  friend class Notify;
};

INLINE ostream &operator << (ostream &out, const NotifyCategory &cat) {
  return out << cat.get_fullname();
}

#include "notifyCategory.I"

#endif
