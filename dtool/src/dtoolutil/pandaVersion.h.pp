//
// pandaVersion.h.pp
//
// This file defines the script to auto-generate pandaVersion.h at
// ppremake time.
//

#output pandaVersion.h notouch
/* Filename: pandaVersion.h
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
 *  Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
 ***************************** DO NOT EDIT *************************

   Do NOT attempt to edit the version number in this file.  This is a
   generated file, and your changes to this file will not persist.  To
   increment the version number, modify dtool/PandaVersion.pp and
   re-run ppremake.

 ***************************** DO NOT EDIT *************************/

/* Include this file anywhere you need to determine the Panda version
   number at compile time.  If you need the runtime Panda version, use
   pandaSystem.h instead. */

/* Try to avoid including this file from another .h file; include it
   only from .cxx instead.  This helps prevent unnecessarily long
   rebuilds just because the version number changes; if this file is
   included in a .h file, then any other files which also include that
   .h file will need to be rebuilt when the version number changes. */

$[cdefine PANDA_MAJOR_VERSION]
$[cdefine PANDA_MINOR_VERSION]
$[cdefine PANDA_SEQUENCE_VERSION]

/* Define if this is an "official" version, undefine otherwise. */
$[cdefine PANDA_OFFICIAL_VERSION]

/* This is the panda numeric version as a single number, with three
   digits reserved for each component. */
$[cdefine PANDA_NUMERIC_VERSION]

/* This is the panda version expressed as a string.  It ends in the
   letter "c" if this is not an "official" version (e.g. it was checked
   out from CVS by the builder). */
# define PANDA_VERSION_STR "$[PANDA_VERSION_STR]"

/* This is the version of the Panda3D ABI expressed as a string.
   This usually means the major and minor version. It should be the
   same for Panda3D versions that are supposed to be backward
   ABI compatible with each other. */
# define PANDA_ABI_VERSION_STR "$[PANDA_MAJOR_VERSION].$[PANDA_MINOR_VERSION]"

/* This is a string indicating who has provided this distribution. */
# define PANDA_DISTRIBUTOR "$[PANDA_DISTRIBUTOR]"

/* The string indicating the version number of the associated Panda3D
   distributable package, or empty string if there is no associated
   package. */
# define PANDA_PACKAGE_VERSION_STR "$[PANDA_PACKAGE_VERSION]"

/* The string indicating the URL from which the associated Panda3D
   distributable package may be downloaded, or empty string if there
   is no associated package. */
# define PANDA_PACKAGE_HOST_URL "$[PANDA_PACKAGE_HOST_URL]"

#if HAVE_P3D_PLUGIN
/* Similar definitions for the plugin versioning, if in use. */
$[cdefine P3D_PLUGIN_MAJOR_VERSION]
$[cdefine P3D_PLUGIN_MINOR_VERSION]
$[cdefine P3D_PLUGIN_SEQUENCE_VERSION]
# define P3D_COREAPI_VERSION_STR "$[join .,$[P3D_COREAPI_VERSION]]"
#endif

#end pandaVersion.h


// Let's also define checkPandaVersion.h and checkPandaVersion.cxx here.

#output checkPandaVersion.h notouch
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
 *  Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
 ***************************** DO NOT EDIT *************************/

/* Include this file in code that compiles with Panda to guarantee
   that it is linking with the same version of the Panda DLL's that it
   was compiled with.  You should include it in one .cxx file only. */

/* We guarantee this by defining an external symbol which is based on
   the version number.  If that symbol is defined, then our DLL's
   (probably) match.  Otherwise, we must be running with the wrong
   DLL; but the system linker will prevent the DLL from loading with
   an undefined symbol. */

# include "dtoolbase.h"

extern EXPCL_DTOOL int $[PANDA_VERSION_SYMBOL];

# ifndef WIN32
/* For Windows, exporting the symbol from the DLL is sufficient; the
   DLL will not load unless all expected public symbols are defined.
   Other systems may not mind if the symbol is absent unless we
   explictly write code that references it. */
static int check_panda_version = $[PANDA_VERSION_SYMBOL];
# endif

#end checkPandaVersion.h

#output checkPandaVersion.cxx notouch
/* Filename: checkPandaVersion.cxx
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
 *  Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
 ***************************** DO NOT EDIT *************************/

# include "dtoolbase.h"

EXPCL_DTOOL int $[PANDA_VERSION_SYMBOL] = 0;

#end checkPandaVersion.cxx
