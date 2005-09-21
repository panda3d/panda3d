// Filename: httpBasicAuthorization.h
// Created by:  drose (22Oct02)
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

#ifndef HTTPBASICAUTHORIZATION_H
#define HTTPBASICAUTHORIZATION_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even though it doesn't
// actually use any OpenSSL code, because it is a support module for
// HTTPChannel, which *does* use OpenSSL code.

#ifdef HAVE_OPENSSL

#include "httpAuthorization.h"

////////////////////////////////////////////////////////////////////
//       Class : HTTPBasicAuthorization
// Description : Implements the "Basic" type of HTTP authorization.
//               This authorization sends usernames and passwords over
//               the net in cleartext, so it's not much in the way of
//               security, but it's easy to implement and therefore
//               widely supported.
////////////////////////////////////////////////////////////////////
class HTTPBasicAuthorization : public HTTPAuthorization {
public:
  HTTPBasicAuthorization(const Tokens &tokens, const URLSpec &url,
                         bool is_proxy);
  virtual ~HTTPBasicAuthorization();

  virtual const string &get_mechanism() const;
  virtual string generate(HTTPEnum::Method method, const string &request_path,
                          const string &username, const string &body);

private:
  static const string _mechanism;
};

#include "httpBasicAuthorization.I"

#endif  // HAVE_OPENSSL

#endif

