/* Filename: checkPandaVersion.h
 * Created by:  drose (26Jan05)
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

/*******************************************************************
 *  Generated automatically by CMake.
 ***************************** DO NOT EDIT *************************/

/* Include this file in code that compiles with Panda to guarantee
   that it is linking with the same version of the Panda DLL's that it
   was compiled with.  You should include it in one .cxx file only. */

/* We guarantee this by defining an external symbol which is based on
   the version number.  If that symbol is defined, then our DLL's
   (probably) match.  Otherwise, we must be running with the wrong
   DLL; but the system linker will prevent the DLL from loading with
   an undefined symbol. */

#include "dtoolbase.h"

extern EXPCL_DTOOL int @PANDA_VERSION_SYMBOL@;

#ifndef WIN32
/* For Windows, exporting the symbol from the DLL is sufficient; the
   DLL will not load unless all expected public symbols are defined.
   Other systems may not mind if the symbol is absent unless we
   explictly write code that references it. */
static int check_panda_version = @PANDA_VERSION_SYMBOL@;
#endif
