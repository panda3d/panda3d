/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fhandle.h
 * @author drose
 * @date 2009-08-29
 */

#ifndef FHANDLE_H
#define FHANDLE_H

// This header file simply defines the FHandle type, which is used to
// pass around a handle to an open file object.

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef HANDLE FHandle;
static const HANDLE invalid_fhandle = INVALID_HANDLE_VALUE;
#else
// On POSIX, we use a file descriptor as a "handle".
typedef int FHandle;
static const int invalid_fhandle = -1;
#endif

#endif
