/* Filename: directsymbols.h
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

#ifndef DIRECTSYMBOLS_H
#define DIRECTSYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

#ifdef BUILDING_DIRECT
  #define EXPCL_DIRECT EXPORT_CLASS
  #define EXPTP_DIRECT EXPORT_TEMPL
#else
  #define EXPCL_DIRECT IMPORT_CLASS
  #define EXPTP_DIRECT IMPORT_TEMPL
#endif

#endif
