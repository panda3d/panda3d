// Filename: binaryXml.h
// Created by:  drose (13Jul09)
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

#ifndef BINARYXML_H
#define BINARYXML_H

#include "get_tinyxml.h"
#include "handleStream.h"
#include <iostream>

using namespace std;

// A pair of functions to input and output the TinyXml constructs on
// the indicated streams.  We could, of course, use the TinyXml output
// operators, but this is a smidge more efficient and gives us more
// control.

void init_xml();
void write_xml(ostream &out, TiXmlDocument *doc, ostream &logfile);
TiXmlDocument *read_xml(istream &in, ostream &logfile);

#endif
