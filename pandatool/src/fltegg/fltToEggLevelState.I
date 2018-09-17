/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltToEggLevelState.I
 * @author drose
 * @date 2001-04-18
 */

/**
 *
 */
INLINE FltToEggLevelState::
FltToEggLevelState(FltToEggConverter *converter) :
  _converter(converter)
{
  _flt_object = nullptr;
  _egg_parent = nullptr;
}

/**
 *
 */
INLINE FltToEggLevelState::
FltToEggLevelState(const FltToEggLevelState &copy) :
  _flt_object(copy._flt_object),
  _egg_parent(copy._egg_parent),
  _converter(copy._converter)
{
  // We don't bother to copy the _parents map.
}

/**
 *
 */
INLINE void FltToEggLevelState::
operator = (const FltToEggLevelState &copy) {
  _flt_object = copy._flt_object;
  _egg_parent = copy._egg_parent;
  _converter = copy._converter;
  // We don't bother to copy the _parents map.
}
