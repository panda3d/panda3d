/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file notifyCategory.h
 * @author drose
 * @date 2000-02-29
 */

#ifndef NOTIFYCATEGORY_H
#define NOTIFYCATEGORY_H

#include "dtoolbase.h"

#include "notifySeverity.h"
#include "configVariableEnum.h"
#include "configFlags.h"
#include "memoryBase.h"

#include <vector>

/**
 * A particular category of error messages.  Typically there will be one of
 * these per package, so that we can turn on or off error messages at least at
 * a package level; further nested categories can be created within a package
 * if a finer grain of control is required.
 */
class EXPCL_DTOOL_PRC NotifyCategory : public MemoryBase, public ConfigFlags {
private:
  NotifyCategory(const std::string &fullname, const std::string &basename,
                 NotifyCategory *parent);

PUBLISHED:
  INLINE std::string get_fullname() const;
  INLINE std::string get_basename() const;
  INLINE NotifySeverity get_severity() const;
  INLINE void set_severity(NotifySeverity severity);
  MAKE_PROPERTY(fullname, get_fullname);
  MAKE_PROPERTY(basename, get_basename);
  MAKE_PROPERTY(severity, get_severity, set_severity);

  INLINE bool is_on(NotifySeverity severity) const;

  // When NOTIFY_DEBUG is not defined, the categories will never be set to
  // "spam" or "debug" severities, and these methods are redefined to be
  // static to make it more obvious to the compiler.  However, we still want
  // to present a consistent interface to our scripting language, so during
  // the interrogate pass (that is, when CPPPARSER is defined), we still
  // pretend they're nonstatic.
  INLINE bool is_spam() const;
  INLINE bool is_debug() const;
  INLINE bool is_info() const;
  INLINE bool is_warning() const;
  INLINE bool is_error() const;
  INLINE bool is_fatal() const;

  std::ostream &out(NotifySeverity severity, bool prefix = true) const;
  INLINE std::ostream &spam(bool prefix = true) const;
  INLINE std::ostream &debug(bool prefix = true) const;
  INLINE std::ostream &info(bool prefix = true) const;
  INLINE std::ostream &warning(bool prefix = true) const;
  INLINE std::ostream &error(bool prefix = true) const;
  INLINE std::ostream &fatal(bool prefix = true) const;

  size_t get_num_children() const;
  NotifyCategory *get_child(size_t i) const;
  MAKE_SEQ(get_children, get_num_children, get_child);
  MAKE_SEQ_PROPERTY(children, get_num_children, get_child);

  static void set_server_delta(long delta);

private:
  std::string get_config_name() const;
  void update_severity_cache();
  static bool get_notify_timestamp();
  static bool get_check_debug_notify_protect();

  std::string _fullname;
  std::string _basename;
  NotifyCategory *_parent;
  ConfigVariableEnum<NotifySeverity> _severity;
  typedef std::vector<NotifyCategory *> Children;
  Children _children;

  static long _server_delta; // not a time_t because server delta may be signed.

  AtomicAdjust::Integer _local_modified;
  NotifySeverity _severity_cache;

  friend class Notify;
};

INLINE std::ostream &operator << (std::ostream &out, const NotifyCategory &cat);

#include "notifyCategory.I"

#endif
