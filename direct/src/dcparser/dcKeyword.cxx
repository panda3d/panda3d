// Filename: dcKeyword.cxx
// Created by:  drose (22Jul05)
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

#include "dcKeyword.h"
#include "hashGenerator.h"
#include "dcindent.h"

////////////////////////////////////////////////////////////////////
//     Function: DCKeyword::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCKeyword::
DCKeyword(const string &name, int historical_flag) :
  _name(name),
  _historical_flag(historical_flag)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCKeyword::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DCKeyword::
~DCKeyword() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCKeyword::get_name
//       Access: Published
//  Description: Returns the name of this keyword.
////////////////////////////////////////////////////////////////////
const string &DCKeyword::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: DCKeyword::get_historical_flag
//       Access: Public
//  Description: Returns the bitmask associated with this keyword, if
//               any.  This is the value that was historically
//               associated with this keyword, and was used to
//               generate a hash code before we had user-customizable
//               keywords.  It will return ~0 if this is not an
//               historical keyword.
////////////////////////////////////////////////////////////////////
int DCKeyword::
get_historical_flag() const {
  return _historical_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: DCKeyword::clear_historical_flag
//       Access: Public
//  Description: Resets the historical flag to ~0, as if the keyword
//               were not one of the historically defined keywords.
////////////////////////////////////////////////////////////////////
void DCKeyword::
clear_historical_flag() {
  _historical_flag = ~0;
}

////////////////////////////////////////////////////////////////////
//     Function : DCKeyword::output
//       Access : Public, Virtual
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void DCKeyword::
output(ostream &out, bool brief) const {
  out << "keyword " << _name;
}

////////////////////////////////////////////////////////////////////
//     Function: DCKeyword::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DCKeyword::
write(ostream &out, bool, int indent_level) const {
  indent(out, indent_level)
    << "keyword " << _name << ";\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCKeyword::generate_hash
//       Access: Public
//  Description: Accumulates the properties of this keyword into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCKeyword::
generate_hash(HashGenerator &hashgen) const {
  hashgen.add_string(_name);
}
