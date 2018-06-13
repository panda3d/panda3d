/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcKeyword.cxx
 * @author drose
 * @date 2005-07-22
 */

#include "dcKeyword.h"
#include "hashGenerator.h"
#include "dcindent.h"

/**
 *
 */
DCKeyword::
DCKeyword(const std::string &name, int historical_flag) :
  _name(name),
  _historical_flag(historical_flag)
{
}

/**
 *
 */
DCKeyword::
~DCKeyword() {
}

/**
 * Returns the name of this keyword.
 */
const std::string &DCKeyword::
get_name() const {
  return _name;
}

/**
 * Returns the bitmask associated with this keyword, if any.  This is the
 * value that was historically associated with this keyword, and was used to
 * generate a hash code before we had user-customizable keywords.  It will
 * return ~0 if this is not an historical keyword.
 */
int DCKeyword::
get_historical_flag() const {
  return _historical_flag;
}

/**
 * Resets the historical flag to ~0, as if the keyword were not one of the
 * historically defined keywords.
 */
void DCKeyword::
clear_historical_flag() {
  _historical_flag = ~0;
}

/**
 * Write a string representation of this instance to <out>.
 */
void DCKeyword::
output(std::ostream &out, bool brief) const {
  out << "keyword " << _name;
}

/**
 *
 */
void DCKeyword::
write(std::ostream &out, bool, int indent_level) const {
  indent(out, indent_level)
    << "keyword " << _name << ";\n";
}

/**
 * Accumulates the properties of this keyword into the hash.
 */
void DCKeyword::
generate_hash(HashGenerator &hashgen) const {
  hashgen.add_string(_name);
}
