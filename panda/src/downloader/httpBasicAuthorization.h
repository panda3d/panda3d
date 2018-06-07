/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpBasicAuthorization.h
 * @author drose
 * @date 2002-10-22
 */

#ifndef HTTPBASICAUTHORIZATION_H
#define HTTPBASICAUTHORIZATION_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even though it doesn't actually
// use any OpenSSL code, because it is a support module for HTTPChannel, which
// *does* use OpenSSL code.

#ifdef HAVE_OPENSSL

#include "httpAuthorization.h"

/**
 * Implements the "Basic" type of HTTP authorization.  This authorization
 * sends usernames and passwords over the net in cleartext, so it's not much
 * in the way of security, but it's easy to implement and therefore widely
 * supported.
 */
class HTTPBasicAuthorization : public HTTPAuthorization {
public:
  HTTPBasicAuthorization(const Tokens &tokens, const URLSpec &url,
                         bool is_proxy);
  virtual ~HTTPBasicAuthorization();

  virtual const std::string &get_mechanism() const;
  virtual std::string generate(HTTPEnum::Method method, const std::string &request_path,
                          const std::string &username, const std::string &body);

private:
  static const std::string _mechanism;
};

#include "httpBasicAuthorization.I"

#endif  // HAVE_OPENSSL

#endif
