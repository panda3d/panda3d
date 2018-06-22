/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpEnum.h
 * @author drose
 * @date 2002-10-25
 */

#ifndef HTTPENUM_H
#define HTTPENUM_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even if you do not intend to use
// this to establish https connections; this is because it uses the OpenSSL
// library to portably handle all of the socket communications.

#ifdef HAVE_OPENSSL

/**
 * This class is just used as a namespace wrapper for some of the enumerated
 * types used by various classes within the HTTPClient family.
 */
class EXPCL_PANDA_DOWNLOADER HTTPEnum {
PUBLISHED:
  enum HTTPVersion {
    HV_09,  // HTTP 0.9 or older
    HV_10,  // HTTP 1.0
    HV_11,  // HTTP 1.1
    HV_other,
  };

  enum Method {
    M_options,
    M_get,
    M_head,
    M_post,
    M_put,
    M_delete,
    M_trace,
    M_connect,
  };
};

std::ostream &operator << (std::ostream &out, HTTPEnum::Method method);

#endif // HAVE_OPENSSL

#endif
