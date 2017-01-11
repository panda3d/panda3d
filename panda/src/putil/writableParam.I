/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file writableParam.I
 * @author jason
 * @date 2000-06-13
 */

/**
 *
 */
INLINE WritableParam::
WritableParam(const Datagram &datagram) :
  _packet(datagram)
{
}

/**
 *
 */
INLINE WritableParam::
WritableParam(const WritableParam &copy) :
  FactoryParam(copy),
  _packet(copy._packet)
{
}

/**
 *
 */
INLINE const Datagram& WritableParam::
get_datagram() {
  return _packet;
}
