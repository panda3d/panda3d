/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lookup3.h
 * @author drose
 * @date 2006-09-01
 */

#ifndef LOOKUP3_H
#define LOOKUP3_H

#include "dtoolbase.h"
#include "numeric_types.h"

#ifdef __cplusplus
extern "C" {
#endif

EXPCL_DTOOL_DTOOLBASE uint32_t hashword(const uint32_t *k,                   /* the key, an array of uint32_t values */
                               size_t          length,               /* the length of the key, in uint32_ts */
                               uint32_t        initval);

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif
