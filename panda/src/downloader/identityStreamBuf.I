/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file identityStreamBuf.I
 * @author drose
 * @date 2006-11-14
 */

/**
 *
 */
INLINE bool IdentityStreamBuf::
is_closed() const {
  return (_source == nullptr || (*_source)->is_closed());
}

/**
 *
 */
INLINE ISocketStream::ReadState IdentityStreamBuf::
get_read_state() const {
  return _read_state;
}
