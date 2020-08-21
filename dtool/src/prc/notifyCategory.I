/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file notifyCategory.I
 * @author drose
 * @date 2000-02-29
 */

/**
 *
 */
INLINE std::string NotifyCategory::
get_fullname() const {
  return _fullname;
}

/**
 *
 */
INLINE std::string NotifyCategory::
get_basename() const {
  return _basename;
}

/**
 *
 */
NotifySeverity NotifyCategory::
get_severity() const {
  TAU_PROFILE("NotifyCategory NotifyCategory::get_severity() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    ((NotifyCategory *)this)->update_severity_cache();
  }
  return _severity_cache;
}

/**
 * Sets the severity level of messages that will be reported from this
 * Category.  This allows any message of this severity level or higher.
 */
INLINE void NotifyCategory::
set_severity(NotifySeverity severity) {
#if defined(NOTIFY_DEBUG)
  _severity = severity;
#else
  // enforce the no-debug, no-spam rule.
  _severity = std::max(severity, NS_info);
#endif
  invalidate_cache();
}

/**
 * Returns true if messages of the indicated severity level ought to be
 * reported for this Category.
 */
INLINE bool NotifyCategory::
is_on(NotifySeverity severity) const {
  TAU_PROFILE("bool NotifyCategory::is_on(NotifySeverity) const", " ", TAU_USER);
  return (int)severity >= (int)get_severity();
}

/**
 * A shorthand way to write is_on(NS_spam).
 */
INLINE bool NotifyCategory::
is_spam() const {
#if defined(NOTIFY_DEBUG) || defined(CPPPARSER)
  // Instruct the compiler to optimize for the usual case.
  return UNLIKELY(is_on(NS_spam));
#else
  return false;
#endif
}

/**
 * A shorthand way to write is_on(NS_debug).
 */
INLINE bool NotifyCategory::
is_debug() const {
#if defined(NOTIFY_DEBUG) || defined(CPPPARSER)
  // Instruct the compiler to optimize for the usual case.
  return UNLIKELY(is_on(NS_debug));
#else
  return false;
#endif
}

/**
 * A shorthand way to write is_on(NS_info).
 */
INLINE bool NotifyCategory::
is_info() const {
  return is_on(NS_info);
}

/**
 * A shorthand way to write is_on(NS_warning).
 */
INLINE bool NotifyCategory::
is_warning() const {
  return is_on(NS_warning);
}

/**
 * A shorthand way to write is_on(NS_error).
 */
INLINE bool NotifyCategory::
is_error() const {
  return is_on(NS_error);
}

/**
 * A shorthand way to write is_on(NS_fatal).
 */
INLINE bool NotifyCategory::
is_fatal() const {
  return is_on(NS_fatal);
}

/**
 * A shorthand way to write out(NS_spam).
 */
INLINE std::ostream &NotifyCategory::
spam(bool prefix) const {
#if defined(NOTIFY_DEBUG)
  return out(NS_spam, prefix);
#else
  return Notify::null();
#endif
}

/**
 * A shorthand way to write out(NS_debug).
 */
INLINE std::ostream &NotifyCategory::
debug(bool prefix) const {
#if defined(NOTIFY_DEBUG)
  return out(NS_debug, prefix);
#else
  return Notify::null();
#endif
}

/**
 * A shorthand way to write out(NS_info).
 */
INLINE std::ostream &NotifyCategory::
info(bool prefix) const {
  return out(NS_info, prefix);
}

/**
 * A shorthand way to write out(NS_warning).
 */
INLINE std::ostream &NotifyCategory::
warning(bool prefix) const {
  return out(NS_warning, prefix);
}

/**
 * A shorthand way to write out(NS_error).
 */
INLINE std::ostream &NotifyCategory::
error(bool prefix) const {
  return out(NS_error, prefix);
}

/**
 * A shorthand way to write out(NS_fatal).
 */
INLINE std::ostream &NotifyCategory::
fatal(bool prefix) const {
  return out(NS_fatal, prefix);
}

INLINE std::ostream &
operator << (std::ostream &out, const NotifyCategory &cat) {
  return out << cat.get_fullname();
}
