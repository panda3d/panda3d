/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cyclerHolder.I
 * @author drose
 * @date 2006-02-09
 */

/**
 *
 */
INLINE CyclerHolder::
CyclerHolder(PipelineCyclerBase &cycler) {
#ifdef DO_PIPELINING
  _cycler = &cycler;
  _cycler->acquire();
#endif
}

/**
 *
 */
INLINE CyclerHolder::
~CyclerHolder() {
#ifdef DO_PIPELINING
  _cycler->release();
#endif
}
