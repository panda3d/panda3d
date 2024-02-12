/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpClient_emscripten.I
 * @author rdb
 * @date 2021-02-10
 */

/**
 * Implements HTTPAuthorization::base64_encode().  This is provided here just
 * as a convenient place to publish it for access by the scripting language;
 * C++ code should probably use HTTPAuthorization directly.
 */
INLINE std::string HTTPClient::
base64_encode(const std::string &s) {
  return HTTPAuthorization::base64_encode(s);
}

/**
 * Implements HTTPAuthorization::base64_decode().  This is provided here just
 * as a convenient place to publish it for access by the scripting language;
 * C++ code should probably use HTTPAuthorization directly.
 */
INLINE std::string HTTPClient::
base64_decode(const std::string &s) {
  return HTTPAuthorization::base64_decode(s);
}
