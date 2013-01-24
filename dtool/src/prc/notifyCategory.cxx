// Filename: notifyCategory.cxx
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

#include "notifyCategory.h"
#include "pnotify.h"
#include "configPageManager.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "config_prc.h"

#ifdef ANDROID
#include "androidLogStream.h"
#endif

#include <time.h>  // for strftime().
#include <assert.h>

long NotifyCategory::_server_delta = 0;

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
NotifyCategory::
NotifyCategory(const string &fullname, const string &basename,
               NotifyCategory *parent) :
  _fullname(fullname),
  _basename(basename),
  _parent(parent),
  _severity(get_config_name(), NS_unspecified, 
            "Default severity of this notify category", 
            ConfigVariable::F_dynamic),
  _local_modified(initial_invalid_cache())
{
  if (_parent != (NotifyCategory *)NULL) {
    _parent->_children.push_back(this);
  }

  // Only the unnamed top category is allowed not to have a parent.
  nassertv(_parent != (NotifyCategory *)NULL || _fullname.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::out
//       Access: Published
//  Description: Begins a new message to this Category at the
//               indicated severity level.  If the indicated severity
//               level is enabled, this writes a prefixing string to
//               the Notify::out() stream and returns that.  If the
//               severity level is disabled, this returns
//               Notify::null().
////////////////////////////////////////////////////////////////////
ostream &NotifyCategory::
out(NotifySeverity severity, bool prefix) const {
  if (is_on(severity)) {

#ifdef ANDROID
    // Android redirects stdio and stderr to /dev/null,
    // but does provide its own logging system.  We use a special
    // type of stream that redirects it to Android's log system.
    if (prefix) {
      return AndroidLogStream::out(severity) << *this << ": ";
    } else {
      return AndroidLogStream::out(severity);
    }
#else

    if (prefix) {
      if (get_notify_timestamp()) {
        // Format a timestamp to include as a prefix as well.
        time_t now = time(NULL) + _server_delta;
        struct tm *ptm = localtime(&now);

        char buffer[128];
        strftime(buffer, 128, ":%m-%d-%Y %H:%M:%S ", ptm);
        nout << buffer;
      }

      if (severity == NS_info) {
        return nout << *this << ": ";
      } else {
        return nout << *this << "(" << severity << "): ";
      }
    } else {
      return nout;
    }
#endif

  } else if (severity <= NS_debug && get_check_debug_notify_protect()) {
    // Someone issued a debug Notify output statement without
    // protecting it within an if statement.  This can cause a
    // significant runtime performance hit, since it forces the
    // iostream library to fully format its output, and then discards
    // the output.
    nout << " **Not protected!** ";
    if (prefix) {
      nout << *this << "(" << severity << "): ";
    }
    if (assert_abort) {
      nassertr(false, nout);
    }

    return nout;

  } else {
    return Notify::null();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::get_num_children
//       Access: Published
//  Description: Returns the number of child Categories of this
//               particular Category.
////////////////////////////////////////////////////////////////////
int NotifyCategory::
get_num_children() const {
  return _children.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::get_child
//       Access: Published
//  Description: Returns the nth child Category of this particular
//               Category.
////////////////////////////////////////////////////////////////////
NotifyCategory *NotifyCategory::
get_child(int i) const {
  assert(i >= 0 && i < (int)_children.size());
  return _children[i];
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::set_server_delta
//       Access: Published, Static
//  Description: Sets a global delta (in seconds) between the local
//               time and the server's time, for the purpose of
//               synchronizing the time stamps in the log messages of
//               the client with that of a known server.
////////////////////////////////////////////////////////////////////
void NotifyCategory::
set_server_delta(long delta) {
  _server_delta = delta;
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::get_config_name
//       Access: Private
//  Description: Returns the name of the config variable that controls
//               this category.  This is called at construction time.
////////////////////////////////////////////////////////////////////
string NotifyCategory::
get_config_name() const {
  string config_name;

  if (_fullname.empty()) {
    config_name = "notify-level";
  } else if (!_basename.empty()) {
    config_name = "notify-level-" + _basename;
  }

  return config_name;
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::update_severity_cache
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void NotifyCategory::
update_severity_cache() {
  if (_severity == NS_unspecified) {
    // If we don't have an explicit severity level, inherit our
    // parent's.
    if (_severity.has_value()) {
      nout << "Invalid severity name for " << _severity.get_name() << ": "
           << _severity.get_string_value() << "\n";
    }
    if (_parent != (NotifyCategory *)NULL) {
      _severity_cache = _parent->get_severity();

    } else {
      // Unless, of course, we're the root.
      _severity_cache = NS_info;
    }
  } else {
    _severity_cache = _severity;
  }
  mark_cache_valid(_local_modified);  
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::get_notify_timestamp
//       Access: Private, Static
//  Description: Returns the value of the notify-timestamp
//               ConfigVariable.  This is defined using a method
//               accessor rather than a static ConfigVariableBool, to
//               protect against the variable needing to be accessed
//               at static init time.
////////////////////////////////////////////////////////////////////
bool NotifyCategory::
get_notify_timestamp() {
  static ConfigVariableBool *notify_timestamp = NULL;
  if (notify_timestamp == (ConfigVariableBool *)NULL) {
    notify_timestamp = new ConfigVariableBool
      ("notify-timestamp", false,
       "Set true to output the date & time with each notify message.");
  }
  return *notify_timestamp;
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::get_check_debug_notify_protect
//       Access: Private, Static
//  Description: Returns the value of the check-debug-notify-protect
//               ConfigVariable.  This is defined using a method
//               accessor rather than a static ConfigVariableBool, to
//               protect against the variable needing to be accessed
//               at static init time.
////////////////////////////////////////////////////////////////////
bool NotifyCategory::
get_check_debug_notify_protect() {
  static ConfigVariableBool *check_debug_notify_protect = NULL;
  if (check_debug_notify_protect == (ConfigVariableBool *)NULL) {
    check_debug_notify_protect = new ConfigVariableBool
      ("check-debug-notify-protect", false,
       "Set true to issue a warning message if a debug or spam "
       "notify output is not protected within an if statement.");
  }
  return *check_debug_notify_protect;
}
