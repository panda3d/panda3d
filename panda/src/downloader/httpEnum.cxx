// Filename: httpEnum.cxx
// Created by:  drose (25Oct02)
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

#include "httpEnum.h"

#ifdef HAVE_SSL

////////////////////////////////////////////////////////////////////
//     Function: HTTPEnum::Method::output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, HTTPEnum::Method method) {
  switch (method) {
  case HTTPEnum::M_options:
    out << "OPTIONS";
    break;

  case HTTPEnum::M_get:
    out << "GET";
    break;

  case HTTPEnum::M_head:
    out << "HEAD";
    break;

  case HTTPEnum::M_post:
    out << "POST";
    break;

  case HTTPEnum::M_put:
    out << "PUT";
    break;

  case HTTPEnum::M_delete:
    out << "DELETE";
    break;

  case HTTPEnum::M_trace:
    out << "TRACE";
    break;

  case HTTPEnum::M_connect:
    out << "CONNECT";
    break;
  }

  return out;
}

#endif  // HAVE_SSL
