// Filename: lookup3.h
// Created by:  drose (01Sep06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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



