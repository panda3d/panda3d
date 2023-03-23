/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGLGraphicsStateGuardian.I
 * @author rdb
 * @date 2012-05-14
 */

/**
 * Gets the FrameBufferProperties for all windows and buffers that use this
 * GSG.
 */
INLINE const FrameBufferProperties &CocoaGLGraphicsStateGuardian::
get_fb_properties() const {
  return _fbprops;
}

/**
 * Locks the context.
 */
INLINE void CocoaGLGraphicsStateGuardian::
lock_context() {
  nassertv(_context != nil);
  CGLLockContext((CGLContextObj) [_context CGLContextObj]);
}

/**
 * Unlocks the context.
 */
INLINE void CocoaGLGraphicsStateGuardian::
unlock_context() {
  nassertv(_context != nil);
  CGLUnlockContext((CGLContextObj) [_context CGLContextObj]);
}
