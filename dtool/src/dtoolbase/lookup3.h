// Filename: lookup3.h
// Created by:  drose (01Sep06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef LOOKUP3_H
#define LOOKUP3_H

#include "dtoolbase.h"
#include "numeric_types.h"

#ifdef __cplusplus
extern "C" {
#endif

EXPCL_DTOOL PN_uint32 hashword(const PN_uint32 *k,                   /* the key, an array of PN_uint32 values */
                               size_t          length,               /* the length of the key, in PN_uint32s */
                               PN_uint32        initval);

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif



