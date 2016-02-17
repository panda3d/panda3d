/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandatoolsymbols.h
 * @author drose
 * @date 2001-04-26
 */

#ifndef PANDATOOLSYMBOLS_H
#define PANDATOOLSYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

#ifdef BUILDING_ASSIMP
  #define EXPCL_ASSIMP EXPORT_CLASS
  #define EXPTP_ASSIMP EXPORT_TEMPL
#else
  #define EXPCL_ASSIMP IMPORT_CLASS
  #define EXPTP_ASSIMP IMPORT_TEMPL
#endif

#ifdef BUILDING_PTLOADER
  #define EXPCL_PTLOADER EXPORT_CLASS
  #define EXPTP_PTLOADER EXPORT_TEMPL
#else
  #define EXPCL_PTLOADER IMPORT_CLASS
  #define EXPTP_PTLOADER IMPORT_TEMPL
#endif

#endif
