/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightMutex.I
 * @author drose
 * @date 2008-10-08
 */

/**
 *
 */
INLINE LightMutex::
#ifdef DEBUG_THREADS
LightMutex() : MutexDebug(std::string(), false, true)
#else
LightMutex()
#endif  // DEBUG_THREADS
{
}

/**
 *
 */
INLINE LightMutex::
#ifdef DEBUG_THREADS
LightMutex(const char *name) : MutexDebug(std::string(name), false, true)
#else
LightMutex(const char *)
#endif  // DEBUG_THREADS
{
}

/**
 *
 */
INLINE LightMutex::
#ifdef DEBUG_THREADS
LightMutex(const std::string &name) : MutexDebug(name, false, true)
#else
LightMutex(const std::string &)
#endif  // DEBUG_THREADS
{
}
