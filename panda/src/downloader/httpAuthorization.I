/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpAuthorization.I
 * @author drose
 * @date 2002-10-22
 */

/**
 * Returns the realm to which this authorization applies.  This is the server-
 * supplied string that may have meaning to the user, and describes the
 * general collection of things protected by this password.
 */
const std::string &HTTPAuthorization::
get_realm() const {
  return _realm;
}

/**
 * Returns the set of domain strings on which this authorization applies.
 * This is the set of URL prefixes for which this authorization should be
 * used.
 */
const vector_string &HTTPAuthorization::
get_domain() const {
  return _domain;
}
