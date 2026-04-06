/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stdcompare.h
 * @author drose
 * @date 2001-06-05
 */

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef STDCOMPARE_H
#define STDCOMPARE_H

template<class key>
class less {
public:
};

template<class key, class comp = std::less<key> >
class hash_compare {
public:
};

#endif

