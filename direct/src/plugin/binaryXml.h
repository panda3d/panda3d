/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file binaryXml.h
 * @author drose
 * @date 2009-07-13
 */

#ifndef BINARYXML_H
#define BINARYXML_H

#include "get_tinyxml.h"
#include "handleStream.h"
#include <iostream>

// A pair of functions to input and output the TinyXml constructs on the
// indicated streams.  We could, of course, use the TinyXml output operators,
// but this is a smidge more efficient and gives us more control.

void init_xml();
void write_xml(std::ostream &out, TiXmlDocument *doc, std::ostream &logfile);
TiXmlDocument *read_xml(std::istream &in, std::ostream &logfile);

#endif
