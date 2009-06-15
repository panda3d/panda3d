/* Filename: directsymbols.h
 * Created by:  drose (18Feb00)
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

#ifndef DIRECTSYMBOLS_H
#define DIRECTSYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

#if defined(WIN32_VC) && !defined(CPPPARSER) && !defined(LINK_ALL_STATIC)

#ifdef BUILDING_DIRECT
  #define EXPCL_DIRECT __declspec(dllexport)
  #define EXPTP_DIRECT
#else
  #define EXPCL_DIRECT __declspec(dllimport)
  #define EXPTP_DIRECT extern
#endif

#else   /* !WIN32_VC */

#define EXPCL_DIRECT
#define EXPTP_DIRECT

#endif  /* WIN32_VC */

#endif
