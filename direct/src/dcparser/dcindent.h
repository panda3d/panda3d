/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcindent.h
 * @author drose
 * @date 1999-01-16
 */

#ifndef DCINDENT_H
#define DCINDENT_H

#include "dcbase.h"

#ifdef WITHIN_PANDA
// If we're compiling this within Panda, we use the function defined there.
#include "indent.h"

#else
// Otherwise, we must define it for ourselves.

/**
 * A handy function for doing text formatting.  This function simply outputs
 * the indicated number of spaces to the given output stream, returning the
 * stream itself.  Useful for indenting a series of lines of text by a given
 * amount.
 */
std::ostream &
indent(std::ostream &out, int indent_level);

#endif  // WITHIN_PANDA

#endif  // DCINDENT_H
