// Filename: indent.h
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef INDENT_H
#define INDENT_H

#include "dcbase.h"

// We rename indent() so it won't clash with the similar function
// defined in Panda.
#define indent dcindent

////////////////////////////////////////////////////////////////////
//     Function: indent
//  Description: A handy function for doing text formatting.  This
//               function simply outputs the indicated number of
//               spaces to the given output stream, returning the
//               stream itself.  Useful for indenting a series of
//               lines of text by a given amount.
////////////////////////////////////////////////////////////////////
ostream &
indent(ostream &out, int indent_level);

#endif

 

