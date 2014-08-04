// Filename: notifyCategory.I
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


////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::get_fullname
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE string NotifyCategory::
get_fullname() const {
  return _fullname;
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::get_basename
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE string NotifyCategory::
get_basename() const {
  return _basename;
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::get_severity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NotifySeverity NotifyCategory::
get_severity() const {
  TAU_PROFILE("NotifyCategory NotifyCategory::get_severity() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    ((NotifyCategory *)this)->update_severity_cache();
  }
  return _severity_cache;
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::set_severity
//       Access: Public
//  Description: Sets the severity level of messages that will be
//               reported from this Category.  This allows any message
//               of this severity level or higher.
////////////////////////////////////////////////////////////////////
INLINE void NotifyCategory::
set_severity(NotifySeverity severity) {
#if defined(NOTIFY_DEBUG)
  _severity = severity;
#else
  // enforce the no-debug, no-spam rule.
  _severity = max(severity, NS_info);
#endif
  invalidate_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::is_on
//       Access: Public
//  Description: Returns true if messages of the indicated severity
//               level ought to be reported for this Category.
////////////////////////////////////////////////////////////////////
INLINE bool NotifyCategory::
is_on(NotifySeverity severity) const {
  TAU_PROFILE("bool NotifyCategory::is_on(NotifySeverity) const", " ", TAU_USER);
  return (int)severity >= (int)get_severity();
}

#if defined(NOTIFY_DEBUG) || defined(CPPPARSER)
////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::is_spam
//       Access: Public
//  Description: A shorthand way to write is_on(NS_spam).
////////////////////////////////////////////////////////////////////
INLINE bool NotifyCategory::
is_spam() const {
  return is_on(NS_spam);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::is_debug
//       Access: Public
//  Description: A shorthand way to write is_on(NS_debug).
////////////////////////////////////////////////////////////////////
INLINE bool NotifyCategory::
is_debug() const {
  return is_on(NS_debug);
}
#else
////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::is_spam
//       Access: Public, Static
//  Description: When NOTIFY_DEBUG is not defined, the categories are
//               never set to "spam" or "debug" severities, and these
//               methods are redefined to be static to make it more
//               obvious to the compiler.
////////////////////////////////////////////////////////////////////
CONSTEXPR bool NotifyCategory::
is_spam() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::is_debug
//       Access: Public
//  Description: When NOTIFY_DEBUG is not defined, the categories are
//               never set to "spam" or "debug" severities, and these
//               methods are redefined to be static to make it more
//               obvious to the compiler.
////////////////////////////////////////////////////////////////////
CONSTEXPR bool NotifyCategory::
is_debug() {
  return false;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::is_info
//       Access: Public
//  Description: A shorthand way to write is_on(NS_info).
////////////////////////////////////////////////////////////////////
INLINE bool NotifyCategory::
is_info() const {
  return is_on(NS_info);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::is_warning
//       Access: Public
//  Description: A shorthand way to write is_on(NS_warning).
////////////////////////////////////////////////////////////////////
INLINE bool NotifyCategory::
is_warning() const {
  return is_on(NS_warning);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::is_error
//       Access: Public
//  Description: A shorthand way to write is_on(NS_error).
////////////////////////////////////////////////////////////////////
INLINE bool NotifyCategory::
is_error() const {
  return is_on(NS_error);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::is_fatal
//       Access: Public
//  Description: A shorthand way to write is_on(NS_fatal).
////////////////////////////////////////////////////////////////////
INLINE bool NotifyCategory::
is_fatal() const {
  return is_on(NS_fatal);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::spam
//       Access: Public
//  Description: A shorthand way to write out(NS_spam).
////////////////////////////////////////////////////////////////////
INLINE ostream &NotifyCategory::
spam(bool prefix) const {
#if defined(NOTIFY_DEBUG)
  return out(NS_spam, prefix);
#else
  return Notify::null();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::debug
//       Access: Public
//  Description: A shorthand way to write out(NS_debug).
////////////////////////////////////////////////////////////////////
INLINE ostream &NotifyCategory::
debug(bool prefix) const {
#if defined(NOTIFY_DEBUG)
  return out(NS_debug, prefix);
#else
  return Notify::null();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::info
//       Access: Public
//  Description: A shorthand way to write out(NS_info).
////////////////////////////////////////////////////////////////////
INLINE ostream &NotifyCategory::
info(bool prefix) const {
  return out(NS_info, prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::warning
//       Access: Public
//  Description: A shorthand way to write out(NS_warning).
////////////////////////////////////////////////////////////////////
INLINE ostream &NotifyCategory::
warning(bool prefix) const {
  return out(NS_warning, prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::error
//       Access: Public
//  Description: A shorthand way to write out(NS_error).
////////////////////////////////////////////////////////////////////
INLINE ostream &NotifyCategory::
error(bool prefix) const {
  return out(NS_error, prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::fatal
//       Access: Public
//  Description: A shorthand way to write out(NS_fatal).
////////////////////////////////////////////////////////////////////
INLINE ostream &NotifyCategory::
fatal(bool prefix) const {
  return out(NS_fatal, prefix);
}

INLINE ostream &
operator << (ostream &out, const NotifyCategory &cat) {
  return out << cat.get_fullname();
}
