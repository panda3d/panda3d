/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file notifySeverity.cxx
 * @author drose
 * @date 2000-02-29
 */

#include "notifySeverity.h"
#include "pnotify.h"

using std::istream;
using std::ostream;
using std::string;

ostream &
operator << (ostream &out, NotifySeverity severity) {
  switch (severity) {
  case NS_spam:
    return out << "spam";

  case NS_debug:
    return out << "debug";

  case NS_info:
    return out << "info";

  case NS_warning:
    return out << "warning";

  case NS_error:
    return out << "error";

  case NS_fatal:
    return out << "fatal";

  case NS_unspecified:
    return out << "unspecified";
  }

  return out << "**invalid severity**";
}

istream &
operator >> (istream &in, NotifySeverity &severity) {
  string word;
  in >> word;
  severity = Notify::string_severity(word);
  return in;
}
