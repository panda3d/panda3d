// Filename: graphHashGenerator.h
// Created by:  drose (14May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef GRAPHHASHGENERATOR_H
#define GRAPHHASHGENERATOR_H

////////////////////////////////////////////////////////////////////
//
// This file defines the typedef for GraphHashGenerator, which defines
// the kind of HashGenerator used by all the NodeTransitions and
// NodeAttributes.  By changing this typedef we can change the
// fundamental hash generation mechanism for these things, if
// necessary.
//
// These hash codes are used to uniquify NodeTransitions, particularly
// in the cull traverser, but only if the hashtable based extensions
// to STL are available (e.g. <hash_map> and <hash_set>).
//
////////////////////////////////////////////////////////////////////

#include <checksumHashGenerator.h>

typedef ChecksumHashGenerator GraphHashGenerator;

#endif
