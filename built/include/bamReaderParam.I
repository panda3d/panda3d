/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamReaderParam.I
 * @author jason
 * @date 2000-06-13
 */

/**
 *
 */
INLINE DatagramIterator &BamReaderParam::
get_iterator() {
  return _iterator;
}

/**
 *
 */
INLINE BamReader *BamReaderParam::
get_manager() {
  return _manager;
}


/**
 *
 */
INLINE BamReaderParam::
BamReaderParam(DatagramIterator &dgi, BamReader *manager) :
  _iterator(dgi),
  _manager(manager)
{
}

/**
 *
 */
INLINE BamReaderParam::
~BamReaderParam() {
}
