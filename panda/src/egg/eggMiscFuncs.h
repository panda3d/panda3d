// Filename: eggMiscFuncs.h
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGMISCFUNCS_H
#define EGGMISCFUNCS_H

////////////////////////////////////////////////////////////////////
//
// eggMiscFuncs.h
//
// This contains the prototypes for functions that are useful to
// internal egg code.  Also see eggUtilities.h, which contains
// functions that may be useful to the rest of the world.
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include <lmatrix.h>
#include <typedef.h>

#include <string>

#ifndef WIN32_VC
class ostream;
#endif


////////////////////////////////////////////////////////////////////
//     Function: enquote_string
//  Description: Writes the string to the indicated output stream.  If
//               the string contains any characters special to egg,
//               writes quotation marks around it.  If always_quote is
//               true, writes quotation marks regardless.
////////////////////////////////////////////////////////////////////
ostream &
enquote_string(ostream &out, const string &str,
               int indent_level = 0,
               bool always_quote = false);



////////////////////////////////////////////////////////////////////
//     Function: write_transform
//  Description: A helper function to write out a 3x3 transform
//               matrix.
////////////////////////////////////////////////////////////////////
void
write_transform(ostream &out, const LMatrix3d &mat, int indent_level);


////////////////////////////////////////////////////////////////////
//     Function: write_transform
//  Description: A helper function to write out a 4x4 transform
//               matrix.
////////////////////////////////////////////////////////////////////
void
write_transform(ostream &out, const LMatrix4d &mat, int indent_level);

#include "eggMiscFuncs.I"

#endif

