/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpBasicAuthorization.cxx
 * @author drose
 * @date 2002-10-22
 */

#include "httpBasicAuthorization.h"

#if defined(HAVE_OPENSSL) || defined(__EMSCRIPTEN__)

using std::string;

/**
 *
 */
HTTPBasicAuthorization::
HTTPBasicAuthorization(const HTTPAuthorization::Tokens &tokens,
                       const URLSpec &url, bool is_proxy) :
  HTTPAuthorization(tokens, url, is_proxy)
{
}

/**
 *
 */
HTTPBasicAuthorization::
~HTTPBasicAuthorization() {
}

/**
 * Returns the type of authorization mechanism, represented as a string, e.g.
 * "basic".
 */
const string &HTTPBasicAuthorization::
get_mechanism() const {
  return _mechanism;
}

/**
 * Generates a suitable authorization string to send to the server, based on
 * the data stored within this object, for retrieving the indicated URL with
 * the given username:password.
 */
string HTTPBasicAuthorization::
generate(HTTPEnum::Method, std::string_view,
         std::string_view username, std::string_view) {
  return "Basic " + base64_encode(username);
}

#endif  // HAVE_OPENSSL
