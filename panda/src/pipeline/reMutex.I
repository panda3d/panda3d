/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file reMutex.I
 * @author drose
 * @date 2006-01-15
 */

/**
 *
 */
INLINE ReMutex::
#ifdef DEBUG_THREADS
ReMutex() : MutexDebug(std::string(), true, false)
#else
ReMutex()
#endif  // DEBUG_THREADS
{
}

/**
 *
 */
INLINE ReMutex::
#ifdef DEBUG_THREADS
ReMutex(const char *name) : MutexDebug(std::string(name), true, false)
#else
ReMutex(const char *)
#endif  // DEBUG_THREADS
{
}

/**
 *
 */
INLINE ReMutex::
#ifdef DEBUG_THREADS
ReMutex(const std::string &name) : MutexDebug(name, true, false)
#else
ReMutex(const std::string &)
#endif  // DEBUG_THREADS
{
}
