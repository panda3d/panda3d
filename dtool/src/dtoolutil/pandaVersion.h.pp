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
# define PANDA_VERSION $[+ $[* $[PANDA_MAJOR_VERSION],1000000],$[* $[PANDA_MINOR_VERSION],1000],$[PANDA_SEQUENCE_VERSION]]

/* This is the panda version expressed as a string.  It ends in the
   letter "c" if this is not an "official" version (e.g. it was checked
   out from CVS by the builder). */
# define PANDA_VERSION_STR "$[PANDA_VERSION_STR]"

/* This is a string indicating who has provided this distribution. */
# define PANDA_DISTRIBUTOR "$[PANDA_DISTRIBUTOR]"

#end pandaVersion.h


// Let's also define checkPandaVersion.h and checkPandaVersion.cxx here.

#output checkPandaVersion.h notouch
/* Filename: checkPandaVersion.h
 * Created by:  drose (26Jan05)
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

/*******************************************************************
 *  Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
 ***************************** DO NOT EDIT *************************/

# include "dtoolbase.h"

EXPCL_DTOOL int $[PANDA_VERSION_SYMBOL] = 0;

#end checkPandaVersion.cxx
