/* Filename: directsymbols.h
 * Created by:  drose (18Feb00)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://etc.cmu.edu/panda3d/docs/license/ .
 *
 * To contact the maintainers of this program write to
 * panda3d-general@lists.sourceforge.net .
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DIRECTSYMBOLS_H
#define DIRECTSYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

#if defined(WIN32_VC) && !defined(CPPPARSER)

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
