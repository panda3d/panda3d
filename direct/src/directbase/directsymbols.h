/*
// Filename: directsymbols.h
// Created by:  drose (18Feb00)
// 
////////////////////////////////////////////////////////////////////
*/

#ifndef DIRECTSYMBOLS_H
#define DIRECTSYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

#if defined(PENV_WIN32) && !defined(CPPPARSER)

#ifdef BUILDING_DIRECT
  #define EXPCL_DIRECT __declspec(dllexport)
  #define EXPTP_DIRECT
#else
  #define EXPCL_DIRECT __declspec(dllimport)
  #define EXPTP_DIRECT extern
#endif

#else   /* !PENV_WIN32 */

#define EXPCL_DIRECT
#define EXPTP_DIRECT

#endif  /* PENV_WIN32 */

#endif
