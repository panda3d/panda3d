// Filename: notify.cxx
// Created by:  drose (28Feb00)
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

#include "notify.h"
#include "config_notify.h"
#include "dconfig.h"

#include <filename.h>

#include <ctype.h>
#include <time.h>  // for strftime().

#ifdef WIN32
#include <windows.h>   //for DebugBreak()
#endif

Notify *Notify::_global_ptr = (Notify *)NULL;


////////////////////////////////////////////////////////////////////
//     Function: Notify::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Notify::
Notify() {
  _ostream_ptr = &cerr;
  _owns_ostream_ptr = false;
  _null_ostream_ptr = new fstream;

  _assert_handler = (AssertHandler *)NULL;
  _assert_failed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Notify::
~Notify() {
  if (_owns_ostream_ptr) {
    delete _ostream_ptr;
  }
  delete _null_ostream_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::set_ostream_ptr
//       Access: Public
//  Description: Changes the ostream that all subsequent Notify
//               messages will be written to.  If the previous ostream
//               was set with delete_later = true, this will delete
//               the previous ostream.  If ostream_ptr is NULL, this
//               resets the default to cerr.
////////////////////////////////////////////////////////////////////
void Notify::
set_ostream_ptr(ostream *ostream_ptr, bool delete_later) {
  if (_owns_ostream_ptr && ostream_ptr != _ostream_ptr) {
    delete _ostream_ptr;
  }

  if (ostream_ptr == (ostream *)NULL) {
    _ostream_ptr = &cerr;
    _owns_ostream_ptr = false;
  } else {
    _ostream_ptr = ostream_ptr;
    _owns_ostream_ptr = delete_later;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::get_ostream_ptr
//       Access: Public
//  Description: Returns the system-wide ostream for all Notify
//               messages.
////////////////////////////////////////////////////////////////////
ostream *Notify::
get_ostream_ptr() const {
  return _ostream_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::get_literal_flag
//       Access: Public
//  Description: Returns a flag that may be set on the Notify stream
//               via setf() that, when set, enables "literal" mode,
//               which means the Notify stream will not attempt to do
//               any fancy formatting (like word-wrapping).
//
//               Notify does not itself respect this flag; this is
//               left up to the ostream that Notify writes to.  Note
//               that Notify just maps to cerr by default, in which
//               case this does nothing.  But the flag is available in
//               case any extended types want to make use of it.
////////////////////////////////////////////////////////////////////
ios::fmtflags Notify::
get_literal_flag() {
  static bool got_flag = false;
  static ios::fmtflags flag;

  if (!got_flag) {
#ifndef HAVE_IOSTREAM
    flag = ios::bitalloc();
#else
    // We lost bitalloc in the new iostream?  This feature will just be
    // disabled for now.  No big deal.
    flag = (ios::fmtflags)0;
#endif
    got_flag = true;
  }

  return flag;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::set_assert_handler
//       Access: Public
//  Description: Sets a pointer to a C function that will be called
//               when an assertion test fails.  This function may
//               decide what to do when that happens: it may choose to
//               abort or return.  If it returns, it should return
//               true to indicate that the assertion should be
//               respected (and the calling function should return out
//               of its block of code), or false to indicate that the
//               assertion should be completely ignored.
//
//               If an assert handler is installed, it completely
//               replaces the default behavior of nassertr() and
//               nassertv().
////////////////////////////////////////////////////////////////////
void Notify::
set_assert_handler(Notify::AssertHandler *assert_handler) {
  _assert_handler = assert_handler;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::clear_assert_handler
//       Access: Public
//  Description: Removes the installed assert handler and restores
//               default behavior of nassertr() and nassertv().
////////////////////////////////////////////////////////////////////
void Notify::
clear_assert_handler() {
  _assert_handler = (AssertHandler *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::has_assert_handler
//       Access: Public
//  Description: Returns true if a user assert handler has been
//               installed, false otherwise.
////////////////////////////////////////////////////////////////////
bool Notify::
has_assert_handler() const {
  return (_assert_handler != (AssertHandler *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::get_assert_handler
//       Access: Public
//  Description: Returns a pointer to the user-installed assert
//               handler, if one was installed, or NULL otherwise.
////////////////////////////////////////////////////////////////////
Notify::AssertHandler *Notify::
get_assert_handler() const {
  return _assert_handler;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::has_assert_failed
//       Access: Public
//  Description: Returns true if an assertion test has failed (and not
//               been ignored) since the last call to
//               clear_assert_failed().
//
//               When an assertion test fails, the assert handler
//               may decide either to abort, return, or ignore the
//               assertion.  Naturally, if it decides to abort, this
//               flag is irrelevant.  If it chooses to ignore the
//               assertion, the flag is not set.  However, if the
//               assert handler chooses to return out of the
//               function (the normal case), it will also set this
//               flag to indicate that an assertion failure has
//               occurred.
//
//               This will also be the behavior in the absence of a
//               user-defined assert handler.
////////////////////////////////////////////////////////////////////
bool Notify::
has_assert_failed() const {
  return _assert_failed;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::get_assert_error_message
//       Access: Public
//  Description: Returns the error message that corresponds to the
//               assertion that most recently failed.
////////////////////////////////////////////////////////////////////
const string &Notify::
get_assert_error_message() const {
  return _assert_error_message;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::clear_assert_failed
//       Access: Public
//  Description: Resets the assert_failed flag that is set whenever an
//               assertion test fails.  See has_assert_failed().
////////////////////////////////////////////////////////////////////
void Notify::
clear_assert_failed() {
  _assert_failed = false;
}


////////////////////////////////////////////////////////////////////
//     Function: Notify::get_top_category
//       Access: Public
//  Description: Returns the topmost Category in the hierarchy.  This
//               may be used to traverse the hierarchy of available
//               Categories.
////////////////////////////////////////////////////////////////////
NotifyCategory *Notify::
get_top_category() {
  return get_category(string());
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::get_category
//       Access: Public
//  Description: Finds or creates a new Category given the basename of
//               the category and its parent in the category
//               hierarchy.  The parent pointer may be NULL to
//               indicate this is a top-level Category.
////////////////////////////////////////////////////////////////////
NotifyCategory *Notify::
get_category(const string &basename, NotifyCategory *parent_category) {
  // We have to ensure that config_notify has been at least created
  // before we try to create any NotifyCategories, or we'll get an
  // infinite recursion problem.  Calling this function is sufficient.
  config_notify.AmInitializing();

  // The string should not contain colons.
  nassertr(basename.find(':') == string::npos, NULL);

  string fullname;
  if (parent_category != (NotifyCategory *)NULL) {
    fullname = parent_category->get_fullname() + ":" + basename;
  } else {
    // The parent_category is NULL.  If basename is empty, that means
    // we refer to the very top-level category (with an empty
    // fullname); otherwise, it's a new category just below that top
    // level.
    if (!basename.empty()) {
      parent_category = get_top_category();
      fullname = ":" + basename;
    }
  }

  pair<Categories::iterator, bool> result = 
    _categories.insert(Categories::value_type(fullname, NULL));

  bool inserted = result.second;
  NotifyCategory *&category = (*result.first).second;

  if (inserted) {
    // If we just inserted a new record, then we have to create a new
    // Category pointer.  Otherwise, there was already one created
    // from before.
    category = new NotifyCategory(fullname, basename, parent_category);
  }

  return category;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::get_category
//       Access: Public
//  Description: Finds or creates a new Category given the basename of
//               the category and the fullname of its parent.  This is
//               another way to create a category when you don't have
//               a pointer to its parent handy, but you know the name
//               of its parent.  If the parent Category does not
//               already exist, it will be created.
////////////////////////////////////////////////////////////////////
NotifyCategory *Notify::
get_category(const string &basename, const string &parent_fullname) {
  return get_category(basename, get_category(parent_fullname));
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::get_category
//       Access: Public
//  Description: Finds or creates a new Category given the fullname of
//               the Category.  This name should be a sequence of
//               colon-separated names of parent Categories, ending in
//               the basename of this Category,
//               e.g. display:glxdisplay.  This is a shorthand way to
//               define a Category when a pointer to its parent is not
//               handy.
////////////////////////////////////////////////////////////////////
NotifyCategory *Notify::
get_category(const string &fullname) {
  Categories::const_iterator ci;
  ci = _categories.find(fullname);
  if (ci != _categories.end()) {
    return (*ci).second;
  }

  // No such Category; create one.  First identify the parent name,
  // based on the rightmost colon.
  NotifyCategory *parent_category = (NotifyCategory *)NULL;
  string basename = fullname;

  size_t colon = fullname.rfind(':');
  if (colon != string::npos) {
    parent_category = get_category(fullname.substr(0, colon));
    basename = fullname.substr(colon + 1);

  } else if (!fullname.empty()) {
    // The fullname didn't begin with a colon.  Infer one.
    parent_category = get_top_category();
  }

  return get_category(basename, parent_category);
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::out
//       Access: Public, Static
//  Description: A convenient way to get the ostream that should be
//               written to for a Notify-type message.  Also see
//               Category::out() for a message that is specific to a
//               particular Category.
////////////////////////////////////////////////////////////////////
ostream &Notify::
out() {
  return *(ptr()->_ostream_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::null
//       Access: Public, Static
//  Description: A convenient way to get an ostream that doesn't do
//               anything.  Returned by Category::out() when a
//               particular Category and/or Severity is disabled.
////////////////////////////////////////////////////////////////////
ostream &Notify::
null() {
  return *(ptr()->_null_ostream_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::write_string
//       Access: Public, Static
//  Description: A convenient way for scripting languages, which may
//               know nothing about ostreams, to write to Notify.
//               This writes a single string, followed by an implicit
//               newline, to the Notify output stream.
////////////////////////////////////////////////////////////////////
void Notify::
write_string(const string &str) {
  out() << str << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::ptr
//       Access: Public, Static
//  Description: Returns the pointer to the global Notify object.
//               There is only one of these in the world.
////////////////////////////////////////////////////////////////////
Notify *Notify::
ptr() {
  if (_global_ptr == (Notify *)NULL) {
    _global_ptr = new Notify;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::assert_failure
//       Access: Public
//  Description: This function is not intended to be called directly
//               by user code.  It's called from the nassertr() and
//               assertv() macros when an assertion test fails; it
//               handles the job of printing the warning message and
//               deciding what to do about it.
//
//               If this function returns true, the calling function
//               should return out of its function; if it returns
//               false, the calling function should ignore the
//               assertion.
////////////////////////////////////////////////////////////////////
bool Notify::
assert_failure(const char *expression, int line,
               const char *source_file) {
  ostringstream message_str;
  message_str
    << expression << " at line " << line << " of " << source_file;
  string message = message_str.str();

  if (!_assert_failed) {
    // We only save the first assertion failure message, as this is
    // usually the most meaningful when several occur in a row.
    _assert_failed = true;
    _assert_error_message = message;
  }

  if (has_assert_handler()) {
    return (*_assert_handler)(expression, line, source_file);
  }

  nout << "Assertion failed: " << message << "\n";

  if (get_assert_abort()) {
#ifdef WIN32
    // How to trigger an exception in VC++ that offers to take us into
    // the debugger?  abort() doesn't do it.  We used to be able to
    // assert(false), but in VC++ 7 that just throws an exception, and
    // an uncaught exception just exits, without offering to open the
    // debugger.  Guess we'll have to force a segfault.

//    int *ptr = (int *)NULL;
//    *ptr = 1;
      DebugBreak();
#else
    abort();
#endif
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::string_severity
//       Access: Public
//  Description: Given a string, one of "debug", "info", "warning",
//               etc., return the corresponding Severity level, or
//               NS_unspecified if none of the strings matches.
////////////////////////////////////////////////////////////////////
NotifySeverity Notify::
string_severity(const string &str) {
  // Convert the string to lowercase for a case-insensitive
  // comparison.
  string lstring;
  for (string::const_iterator si = str.begin();
       si != str.end();
       ++si) {
    lstring += tolower(*si);
  }

  if (lstring == "spam") {
    return NS_spam;

  } else if (lstring == "debug") {
    return NS_debug;

  } else if (lstring == "info") {
    return NS_info;

  } else if (lstring == "warning") {
    return NS_warning;

  } else if (lstring == "error") {
    return NS_error;

  } else if (lstring == "fatal") {
    return NS_fatal;

  } else {
    return NS_unspecified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Notify::config_initialized
//       Access: Public
//  Description: Intended to be called only by Config, this is a
//               callback that indicated to Notify when Config has
//               done initializing and Notify can safely set up some
//               internal state variables that depend on Config
//               variables.
////////////////////////////////////////////////////////////////////
void Notify::
config_initialized() {
  static bool already_initialized = false;
  if (already_initialized) {
    nout << "Notify::config_initialized() called more than once.\n";
    return;
  }
  already_initialized = true;

  if (_ostream_ptr == &cerr) {
    string notify_output = config_notify.GetString("notify-output", "");
    if (!notify_output.empty()) {
      if (notify_output == "stdout") {
        cout.setf(ios::unitbuf);
        set_ostream_ptr(&cout, false);

      } else if (notify_output == "stderr") {
        set_ostream_ptr(&cerr, false);

      } else {
        Filename filename = notify_output;
        filename.set_text();
        ofstream *out = new ofstream;
        if (!filename.open_write(*out)) {
          nout << "Unable to open file " << filename << " for output.\n";
          delete out;
        } else {
          out->setf(ios::unitbuf);
          set_ostream_ptr(out, true);
        }
      }
    }
  }
}
