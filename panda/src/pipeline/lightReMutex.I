/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightReMutex.I
 * @author drose
 * @date 2008-10-08
 */

/**
 *
 */
INLINE LightReMutex::
#ifdef DEBUG_THREADS
LightReMutex() : MutexDebug(std::string(), true, true)
#else
LightReMutex()
#endif  // DEBUG_THREADS
{
}

/**
 *
 */
INLINE LightReMutex::
#ifdef DEBUG_THREADS
LightReMutex(const char *name) : MutexDebug(std::string(name), true, true)
#else
LightReMutex(const char *)
#endif  // DEBUG_THREADS
{
}

/**
 *
 */
INLINE LightReMutex::
#ifdef DEBUG_THREADS
LightReMutex(const std::string &name) : MutexDebug(name, true, true)
#else
LightReMutex(const std::string &)
#endif  // DEBUG_THREADS
{
}
