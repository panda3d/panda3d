// Filename: password_hash.h
// Created by:  drose (01Sep04)
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

#ifndef PASSWORD_HASH_H
#define PASSWORD_HASH_H

#include "pandabase.h"

// The functions defined within this file rely on algorithms defined
// within OpenSSL.
#ifdef HAVE_OPENSSL

BEGIN_PUBLISH

string password_hash(const string &password, const string &salt, 
                     int iters, int keylen);

END_PUBLISH

#endif // HAVE_OPENSSL

#endif
