/*
// Filename: pandatoolsymbols.h
// Created by:  drose (26Apr01)
// 
////////////////////////////////////////////////////////////////////
*/

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
