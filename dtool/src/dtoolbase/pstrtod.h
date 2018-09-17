/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pstrtod.h
 * @author drose
 * @date 2009-06-13
 */

#ifndef PSTRTOD_H
#define PSTRTOD_H

#include "dtoolbase.h"

#ifdef __cplusplus
extern "C" {
#endif

EXPCL_DTOOL_DTOOLBASE double
pstrtod(const char *nptr, char **endptr);

EXPCL_DTOOL_DTOOLBASE double
patof(const char *str);

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif
