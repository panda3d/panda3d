/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dtool_platform.h
 * @author drose
 * @date 2009-08-03
 */

#ifndef DTOOL_PLATFORM_H
#define DTOOL_PLATFORM_H

/* This file defines the macro DTOOL_PLATFORM, which is used in
   PandaSystem and a few other places to report the current platform
   string.  In practice, this string is primarily useful for the
   packaged runtime system. */

#include "dtool_config.h"

#if defined(DTOOL_PLATFORM)
// This has already been defined explicitly by the Config.pp file.

#elif defined(_WIN64)
#define DTOOL_PLATFORM "win_amd64"

#elif defined(_WIN32)
#define DTOOL_PLATFORM "win_i386"

#elif defined(__APPLE__)
#if defined(BUILD_IPHONE)
#define DTOOL_PLATFORM "iphone"
#elif defined(__ppc__)
#define DTOOL_PLATFORM "osx_ppc"
#elif defined(__i386__)
#define DTOOL_PLATFORM "osx_i386"
#elif defined(__x86_64)
#define DTOOL_PLATFORM "osx_amd64"
#endif

#elif defined(__FreeBSD__)
#if defined(__x86_64)
#define DTOOL_PLATFORM "freebsd_amd64"
#elif defined(__i386__)
#define DTOOL_PLATFORM "freebsd_i386"
#endif

#elif defined(__ANDROID__)
#if defined(__ARM_ARCH_7A__)
#define DTOOL_PLATFORM "android_armv7a"
#elif defined(__aarch64__)
#define DTOOL_PLATFORM "android_aarch64"
#elif defined(__arm__)
#define DTOOL_PLATFORM "android_arm"
#elif defined(__mips__)
#define DTOOL_PLATFORM "android_mips"
#elif defined(__x86_64)
#define DTOOL_PLATFORM "android_amd64"
#elif defined(__i386__)
#define DTOOL_PLATFORM "android_i386"
#endif

#elif defined(__aarch64__)
#define DTOOL_PLATFORM "linux_aarch64"

#elif defined(__x86_64)
#define DTOOL_PLATFORM "linux_amd64"

#elif defined(__i386)
#define DTOOL_PLATFORM "linux_i386"

#elif defined(__arm__)
#define DTOOL_PLATFORM "linux_arm"

#elif defined(__ppc__)
#define DTOOL_PLATFORM "linux_ppc"
#endif

#if !defined(DTOOL_PLATFORM) && !defined(CPPPARSER)
#error "Can't determine platform; please define DTOOL_PLATFORM in Config.pp file."
#endif

#endif
