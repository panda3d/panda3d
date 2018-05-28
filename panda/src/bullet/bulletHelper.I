/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletHelper.I
 * @author enn0x
 * @date 2011-01-19
 */

/**
 *
 */
INLINE PT(InternalName) BulletHelper::
get_sb_index() {

  if (_sb_index == nullptr) {
    _sb_index = InternalName::make("sb_index");
  }
  return _sb_index;
}

/**
 *
 */
INLINE PT(InternalName) BulletHelper::
get_sb_flip() {

  if (_sb_flip == nullptr) {
    _sb_flip = InternalName::make("sb_flip");
  }
  return _sb_flip;
}
