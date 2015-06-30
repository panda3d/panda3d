/* Filename: dtoolsymbols.h
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

#ifndef DTOOLSYMBOLS_H
#define DTOOLSYMBOLS_H


/*
  This file defines a slew of symbols that have particular meaning
  only when compiling in the WIN32 environment.  These symbols are
  prefixed to each class declaration, and each global function, that
  is to be made visible outside of a DLL.

  The problem is that in Windows, you must prefix each DLL-public
  symbol with "__declspec(dllexport)" when you are compiling the DLL,
  but with "__declspec(dllimport)" when you are compiling code that
  links with the DLL.  This strange convention means that you must, in
  principle, have two different .h files for a DLL: one to use for
  compiling the DLL (it must export all of the symbols), and a
  different one for presenting the public interface for other users to
  use (it must import all of the same symbols).

  In practice, of course, maintaining two different .h files is silly
  and error-prone; almost everyone solves this problem by defining a
  macro that evaluates to "__declspec(dllexport)" in one case and
  "__declspec(dllimport)" in another case.  Many systems use the
  system macro _DLL to switch between these two case, which works well
  in a simple system because _DLL is defined only if compiler is
  currently generating code for a DLL.  So you can export the symbols
  if _DLL is defined, and import them if it is not defined.

  However, this fails if you are compiling a DLL that wants to import
  symbols from another DLL, since in this case _DLL is defined, but
  the symbols in the other DLL need to be imported, not exported.

  In the general case of compiling multiple DLL's that might reference
  each other's symbols, we need have a separate macro for each DLL.
  Then when we are compiling code for each DLL, the macro for that
  particular DLL evaluates to "__declspec(dllexport)", exporting all
  the symbols from the DLL, while all the other DLL's macros evaluate
  to "__declspec(dllimport)", importing all the symbols from the other
  DLL's.

  That is the approach we have taken here in Panda.  When we are
  compiling code for a particular DLL, the build scripts define the
  macro BUILDING_libname on the command line.  This file then uses
  that macro to define EXPCL_libname appropriately for each DLL; this
  macro is then used to prefix each symbol to be exported from the
  DLL.  The macro name stands for "export class", since it is used
  most often to mark a class for export, although the same macro can
  be used to export global functions.  (We also define EXPTP_libname,
  which is used in conjunction with exporting template instantiations,
  another dicey task in Windows.  It is used far less often.)

  Of course, this whole thing only matters under WIN32.  In the rest
  of the world we don't have to deal with this nonsense, and so we
  can define all of these stupid symbols to the empty string.
  */

#define EXPCL_EMPTY

#ifdef BUILDING_DTOOL
  #define EXPCL_DTOOL EXPORT_CLASS
  #define EXPTP_DTOOL EXPORT_TEMPL
#else
  #define EXPCL_DTOOL IMPORT_CLASS
  #define EXPTP_DTOOL IMPORT_TEMPL
#endif

#ifdef BUILDING_DTOOLCONFIG
  #define EXPCL_DTOOLCONFIG EXPORT_CLASS
  #define EXPTP_DTOOLCONFIG EXPORT_TEMPL
#else
  #define EXPCL_DTOOLCONFIG IMPORT_CLASS
  #define EXPTP_DTOOLCONFIG IMPORT_TEMPL
#endif

#ifdef BUILDING_INTERROGATEDB
  #define EXPCL_INTERROGATEDB EXPORT_CLASS
  #define EXPTP_INTERROGATEDB EXPORT_TEMPL
#else
  #define EXPCL_INTERROGATEDB IMPORT_CLASS
  #define EXPTP_INTERROGATEDB IMPORT_TEMPL
#endif

#ifdef BUILDING_MISC
  #define EXPCL_MISC EXPORT_CLASS
  #define EXPTP_MISC EXPORT_TEMPL
#else /* BUILDING_MISC */
  #define EXPCL_MISC IMPORT_CLASS
  #define EXPTP_MISC IMPORT_TEMPL
#endif /* BUILDING_MISC */

/* These two are always defined empty, because pystub is statically
   built.  But we leave the symbol around in case we change our minds
   to make pystub once again be a dynamic library. */
#if __GNUC__ >= 4
/* In GCC, though, we still need to mark the symbols as visible. */
#define EXPCL_PYSTUB __attribute__((visibility("default")))
#else
#define EXPCL_PYSTUB
#endif
#define EXPTP_PYSTUB

#endif
