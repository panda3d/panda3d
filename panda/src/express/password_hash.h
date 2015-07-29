// Filename: password_hash.h
// Created by:  drose (01Sep04)
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

#ifndef PASSWORD_HASH_H
#define PASSWORD_HASH_H

#include "pandabase.h"

// The functions defined within this file rely on algorithms defined
// within OpenSSL.
#ifdef HAVE_OPENSSL

BEGIN_PUBLISH

EXPCL_PANDAEXPRESS string password_hash(const string &password,
                                        const string &salt,
                                        int iters, int keylen);

END_PUBLISH

#endif // HAVE_OPENSSL

#endif
