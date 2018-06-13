/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpEnum.cxx
 * @author drose
 * @date 2002-10-25
 */

#include "httpEnum.h"

#ifdef HAVE_OPENSSL

/**
 *
 */
std::ostream &
operator << (std::ostream &out, HTTPEnum::Method method) {
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

#endif  // HAVE_OPENSSL
