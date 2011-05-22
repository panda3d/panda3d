/* Filename: pandatoolsymbols.h
 * Created by:  drose (26Apr01)
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

#ifndef PANDATOOLSYMBOLS_H
#define PANDATOOLSYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

#if defined(WIN32_VC) && !defined(CPPPARSER) && !defined(LINK_ALL_STATIC)

#ifdef BUILDING_ASSIMP
  #define EXPCL_ASSIMP __declspec(dllexport)
  #define EXPTP_ASSIMP
#else
  #define EXPCL_ASSIMP __declspec(dllimport)
  #define EXPTP_ASSIMP extern
#endif

#ifdef BUILDING_PTLOADER
  #define EXPCL_PTLOADER __declspec(dllexport)
  #define EXPTP_PTLOADER
#else
  #define EXPCL_PTLOADER __declspec(dllimport)
  #define EXPTP_PTLOADER extern
#endif

#else   /* !WIN32_VC */

#define EXPCL_ASSIMP
#define EXPTP_ASSIMP

#define EXPCL_PTLOADER
#define EXPTP_PTLOADER

#endif  /* WIN32_VC */

#endif
