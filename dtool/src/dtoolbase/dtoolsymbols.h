/* Filename: dtoolsymbols.h
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

#ifndef DTOOLSYMBOLS_H
#define DTOOLSYMBOLS_H


/*
  This file defines a slew of symbols that have particular meaning
  only when compiling in the WIN32 environment.  These symbols are
  prefixed to each class declaration, and each static data member
  within a class, that is to visible outside of a DLL.

  We need to specify a particular string for *exporting* the symbols
  when we are compiling a DLL, and for *importing* the symbols when
  we are accessing the DLL (that is, compiling some code that will
  link with the DLL, e.g. an executable program or a different DLL).
  The trick is that we have to be particular about whether we're
  exporting or importing the symbols for a specific DLL.

  We achieve this by defining a unique pair of symbols for each DLL.
  When we are building a particular DLL, we define the manifest
  BUILDING_libname on the command line.  This file then discovers
  that manifest and toggles the flag to *export* only for the symbols
  that belong to that DLL; the symbols that belong in each other DLL
  are set to *import*.

  Of course, this whole thing only matters under WIN32.  In the rest
  of the world we don't have to deal with this nonsense, and so we
  can define all of these stupid symbols to the empty string.
  */

#define EXPCL_EMPTY

#if defined(WIN32_VC) && !defined(CPPPARSER) && !defined(LINK_ALL_STATIC)

#ifdef BUILDING_DTOOL
  #define EXPCL_DTOOL __declspec(dllexport)
  #define EXPTP_DTOOL
#else
  #define EXPCL_DTOOL __declspec(dllimport)
  #define EXPTP_DTOOL extern
#endif

#ifdef BUILDING_DTOOLCONFIG
  #define EXPCL_DTOOLCONFIG __declspec(dllexport)
  #define EXPTP_DTOOLCONFIG
#else
  #define EXPCL_DTOOLCONFIG __declspec(dllimport)
  #define EXPTP_DTOOLCONFIG extern
#endif

#ifdef BUILDING_MISC
  #define EXPCL_MISC __declspec(dllexport)
  #define EXPTP_MISC
#else /* BUILDING_MISC */
  #define EXPCL_MISC __declspec(dllimport)
  #define EXPTP_MISC extern
#endif /* BUILDING_MISC */

#else   /* !WIN32_VC */

#define EXPCL_DTOOL
#define EXPTP_DTOOL

#define EXPCL_DTOOLCONFIG
#define EXPTP_DTOOLCONFIG

#define EXPCL_MISC
#define EXPTP_MISC

#endif  /* WIN32_VC */

#endif
