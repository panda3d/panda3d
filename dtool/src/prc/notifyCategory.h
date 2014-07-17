// Filename: notifyCategory.h
// Created by:  drose (29Feb00)
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

#ifndef NOTIFYCATEGORY_H
#define NOTIFYCATEGORY_H

#include "dtoolbase.h"

#include "notifySeverity.h"
#include "configVariableEnum.h"
#include "configFlags.h"
#include "memoryBase.h"

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
class EXPCL_DTOOLCONFIG NotifyCategory : public MemoryBase, public ConfigFlags {
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
  CONSTEXPR static bool is_spam();
  CONSTEXPR static bool is_debug();
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

  static void set_server_delta(long delta);

private:
  string get_config_name() const;
  void update_severity_cache();
  static bool get_notify_timestamp();
  static bool get_check_debug_notify_protect();

  string _fullname;
  string _basename;
  NotifyCategory *_parent;
  ConfigVariableEnum<NotifySeverity> _severity;
  typedef vector<NotifyCategory *> Children;
  Children _children;

  static long _server_delta; // not a time_t because server delta may be signed.

  AtomicAdjust::Integer _local_modified;
  NotifySeverity _severity_cache;

  friend class Notify;
};

INLINE ostream &operator << (ostream &out, const NotifyCategory &cat);

#include "notifyCategory.I"

#endif
