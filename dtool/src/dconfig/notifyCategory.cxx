// Filename: notifyCategory.cxx
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

#include "notifyCategory.h"
#include "notify.h"
#include "config_notify.h"

#include <time.h>  // for strftime().
#include <assert.h>

time_t NotifyCategory::_server_delta = 0;

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
  _parent(parent)
{
  if (_parent != (NotifyCategory *)NULL) {
    _parent->_children.push_back(this);
  }

  _severity = NS_unspecified;

  // See if there's a config option to set the severity level for this
  // Category.

  string config_name;

  if (_fullname.empty()) {
    config_name = "notify-level";
  } else if (!_basename.empty()) {
    config_name = "notify-level-" + _basename;
  }

  if (!config_name.empty()) {
    string severity_name;
    if (!config_notify.AmInitializing())
      severity_name = config_notify.GetString(config_name, "");
    if (!severity_name.empty()) {
      // The user specified a particular severity for this category at
      // config time.  Use it.
      _severity = Notify::string_severity(severity_name);

      if (_severity == NS_unspecified) {
        nout << "Invalid severity name for " << config_name << ": "
             << severity_name << "\n";
      }
    }
  }

  if (_severity == NS_unspecified) {
    // If we didn't get an explicit severity level, inherit our
    // parent's.
    if (_parent != (NotifyCategory *)NULL) {
      _severity = _parent->_severity;
    } else {
      // Unless, of course, we're the root.
      _severity = NS_info;
    }
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
set_server_delta(time_t delta) {
  _server_delta = delta;
}
