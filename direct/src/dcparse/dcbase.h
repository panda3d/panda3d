// Filename: dcbase.h
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCBASE_H
#define DCBASE_H

// This file defines a few headers and stuff necessary for compilation
// of this project.  Most compiler-specific decisions should be
// grouped up here.

#ifdef WIN32
/* C4786: 255 char debug symbols */
#pragma warning (disable : 4786)
/* C4503: decorated name length exceeded */
#pragma warning (disable : 4503)
#endif  /* WIN32_VC */

#if defined(WIN32) || defined(CPPPARSER)
#include <iostream>
#include <fstream>
#else
#include <iostream.h>
#include <fstream.h>
#endif

#include <string>

// These header files are needed to compile dcLexer.cxx, the output
// from flex.  flex doesn't create a perfectly windows-friendly source
// file right out of the box.
#ifdef WIN32
#include <io.h>
#include <malloc.h>
#else
#include <unistd.h>
#endif

using namespace std;

#ifdef CPPPARSER
// We define the macro PUBLISHED to mark C++ methods that are to be
// published via interrogate to scripting languages.  However, if
// we're not running the interrogate pass (CPPPARSER isn't defined),
// this maps to public.
#define PUBLISHED __published
#else
#define PUBLISHED public
#endif

/*
 We define the macros BEGIN_PUBLISH and END_PUBLISH to bracket
 functions and global variable definitions that are to be published
 via interrogate to scripting languages.
 */
#ifdef CPPPARSER
#define BEGIN_PUBLISH __begin_publish
#define END_PUBLISH __end_publish
#else
#define BEGIN_PUBLISH
#define END_PUBLISH
#endif

#endif

