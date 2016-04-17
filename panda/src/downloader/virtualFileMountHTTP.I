/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountHTTP.I
 * @author drose
 * @date 2008-10-30
 */

/**
 * Returns the HTTPClient object that services this mount point.
 */
INLINE HTTPClient *VirtualFileMountHTTP::
get_http_client() const {
  return _http;
}

/**
 * Returns the URL that represents the root of this mount point.
 */
INLINE const URLSpec &VirtualFileMountHTTP::
get_root() const {
  return _root;
}
