// Filename: notifyCategory.C
// Created by:  drose (29Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "notifyCategory.h"
#include "notify.h"
#include "config_notify.h"

#include <assert.h>

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::Constructor
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Returns the number of child Categories of this
//               particular Category.
////////////////////////////////////////////////////////////////////
int NotifyCategory::
get_num_children() const {
  return _children.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NotifyCategory::get_child
//       Access: Public
//  Description: Returns the nth child Category of this particular
//               Category.
////////////////////////////////////////////////////////////////////
NotifyCategory *NotifyCategory::
get_child(int i) const {
  assert(i >= 0 && i < (int)_children.size());
  return _children[i];
}
