/* Filename: dtool_platform.h
 * Created by:  drose (03Aug09)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DTOOL_PLATFORM_H
#define DTOOL_PLATFORM_H

/* This file defines the macro DTOOL_PLATFORM, which is used in
   PandaSystem and a few other places to report the current platform
   string.  In practice, this string is primarily useful for the
   packaged runtime system. */

#include "dtool_config.h"

#if defined(DTOOL_PLATFORM)
// This has already been defined explicitly by the Config.pp file.

#elif defined(_WIN32)
#define DTOOL_PLATFORM "win32"

#elif defined(_WIN64)
#define DTOOL_PLATFORM "win64"

#elif defined(__APPLE__)
#if defined(BUILD_IPHONE)
#define DTOOL_PLATFORM "iphone"
#elif defined(__ppc__)
#define DTOOL_PLATFORM "osx.ppc"
#else
#define DTOOL_PLATFORM "osx.i386"
#endif

#elif defined(__x86_64)
#define DTOOL_PLATFORM "linux.amd64"

#elif defined(__i386)
#define DTOOL_PLATFORM "linux.i386"

#else
#error Can't determine platform; please define DTOOL_PLATFORM in Config.pp file.

#endif

#endif

