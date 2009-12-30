/* Filename: contribsymbols.h
 * Created by:  rdb (30Dec09)
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

#ifndef CONTRIBSYMBOLS_H
#define CONTRIBSYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

/* Note that the symbols declared in this file appear in alphabetical
   order.  Also note that we must use C-style comments only here, not
   C++-style comments, since this file is occasionally included by a C
   file. */

#if defined(WIN32_VC) && !defined(CPPPARSER) && !defined(LINK_ALL_STATIC)

#ifdef BUILDING_PANDAAI
  #define EXPCL_PANDAAI __declspec(dllexport)
  #define EXPTP_PANDAAI
#else
  #define EXPCL_PANDAAI __declspec(dllimport)
  #define EXPTP_PANDAAI extern
#endif

#else   /* !WIN32_VC */

#define EXPCL_PANDAAI
#define EXPTP_PANDAAI

#endif  /* WIN32_VC */

#endif
