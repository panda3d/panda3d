// Filename: notifySeverity.cxx
// Created by:  drose (29Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "notifySeverity.h"

ostream &operator << (ostream &out, NotifySeverity severity) {
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
