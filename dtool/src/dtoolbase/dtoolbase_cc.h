// Filename: dtoolbase_cc.h
// Created by:  drose (13Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PANDABASE_CC_H
#define PANDABASE_CC_H

// This file should never be included directly; it's intended to be
// included only from dtoolbase.h.  Include that file instead.


#ifdef CPPPARSER
#include <iostream>
#include <string>

using namespace std;

#define INLINE inline
#define TYPENAME typename

#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname)

// We define the macro PUBLISHED to mark C++ methods that are to be
// published via interrogate to scripting languages.  However, if
// we're not running the interrogate pass (CPPPARSER isn't defined),
// this maps to public.
#define PUBLISHED __published

#else  // CPPPARSER

#ifdef HAVE_IOSTREAM
#include <iostream>
#include <fstream>
#include <iomanip>
#else
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#endif

#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include "fakestringstream.h"
#endif

#include <string>

#ifdef HAVE_NAMESPACE
using namespace std;
#endif

#define TYPENAME typename


#if defined(WIN32_VC) && defined(TEST_INLINING)
// If TEST_INLINING is defined, we use the keyword __forceinline,
// which tells VC++ that we really mean it when we say inline.  Of
// course, that doesn't seem to have any additional persuasive effect.
#define INLINE __forceinline
#else
#define INLINE inline
#endif

#if defined(WIN32_VC) && !defined(LINK_ALL_STATIC)
// This macro must be used to export an instantiated template class
// from a DLL.  If the template class name itself contains commas, it
// may be necessary to first define a macro for the class name, to
// allow proper macro parameter passing.
#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname) \
  exptp template class expcl classname;
#else
#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname)
#define INLINE inline
#endif

// We define the macro PUBLISHED to mark C++ methods that are to be
// published via interrogate to scripting languages.  However, if
// we're not running the interrogate pass (CPPPARSER isn't defined),
// this maps to public.
#define PUBLISHED public

#endif  // CPPPARSER


#endif
