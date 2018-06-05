/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmutex.I
 * @author drose
 * @date 2002-08-08
 */

/**
 *
 */
INLINE Mutex::
#ifdef DEBUG_THREADS
Mutex() : MutexDebug(string(), false, false)
#else
Mutex()
#endif  // DEBUG_THREADS
{
}

/**
 *
 */
INLINE Mutex::
#ifdef DEBUG_THREADS
Mutex(const char *name) : MutexDebug(string(name), false, false)
#else
Mutex(const char *)
#endif  // DEBUG_THREADS
{
}

/**
 *
 */
INLINE Mutex::
#ifdef DEBUG_THREADS
Mutex(const string &name) : MutexDebug(name, false, false)
#else
Mutex(const string &)
#endif  // DEBUG_THREADS
{
}
