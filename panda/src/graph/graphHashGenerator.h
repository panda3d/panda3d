// Filename: graphHashGenerator.h
// Created by:  drose (14May01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef GRAPHHASHGENERATOR_H
#define GRAPHHASHGENERATOR_H

////////////////////////////////////////////////////////////////////
//
// This file defines the typedef for GraphHashGenerator, which defines
// the kind of HashGenerator used by all the NodeTransitions.  By
// changing this typedef we can change the fundamental hash generation
// mechanism for these things, if necessary.
//
// These hash codes are used to uniquify NodeTransitions, particularly
// in the cull traverser, but only if the hashtable based extensions
// to STL are available (e.g. <hash_map> and <hash_set>).
//
////////////////////////////////////////////////////////////////////

#include <checksumHashGenerator.h>

typedef ChecksumHashGenerator GraphHashGenerator;

#endif
