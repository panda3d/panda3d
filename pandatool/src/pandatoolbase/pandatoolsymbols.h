/* Filename: pandatoolsymbols.h
 * Created by:  drose (26Apr01)
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

#ifndef PANDATOOLSYMBOLS_H
#define PANDATOOLSYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

#if defined(WIN32_VC) && !defined(CPPPARSER) && !defined(LINK_ALL_STATIC)

#ifdef BUILDING_PTLOADER
  #define EXPCL_PTLOADER __declspec(dllexport)
  #define EXPTP_PTLOADER
#else
  #define EXPCL_PTLOADER __declspec(dllimport)
  #define EXPTP_PTLOADER extern
#endif

#else   /* !WIN32_VC */

#define EXPCL_PTLOADER
#define EXPTP_PTLOADER

#endif  /* WIN32_VC */

#endif
