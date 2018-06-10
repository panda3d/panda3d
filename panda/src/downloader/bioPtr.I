/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bioPtr.I
 * @author drose
 * @date 2002-10-15
 */

/**
 *
 */
INLINE BioPtr::
BioPtr(BIO *bio) : _bio(bio) {
}

/**
 *
 */
INLINE BIO &BioPtr::
operator *() const {
  return *_bio;
}

/**
 *
 */
INLINE BIO *BioPtr::
operator ->() const {
  return _bio;
}

/**
 *
 */
INLINE BioPtr::
operator BIO * () const {
  return _bio;
}

/**
 *
 */
INLINE void BioPtr::
set_bio(BIO *bio) {
  _bio = bio;
}

/**
 *
 */
INLINE BIO *BioPtr::
get_bio() const {
  return _bio;
}

/**
 * Returns the name of the server we are (or should be) connected to.
 */
INLINE const std::string &BioPtr::
get_server_name() const {
  return _server_name;
}

/**
 * Returns the port on which we are (or should be) connected.
 */
INLINE int BioPtr::
get_port() const {
  return _port;
}
