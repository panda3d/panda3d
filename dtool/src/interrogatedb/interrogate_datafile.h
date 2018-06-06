/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogate_datafile.h
 * @author drose
 * @date 2000-08-09
 */

#ifndef INTERROGATE_DATAFILE_H
#define INTERROGATE_DATAFILE_H

// This file defines some convenience functions for reading and writing the
// interrogate database files.

#include "dtoolbase.h"
#include <vector>

void idf_output_string(ostream &out, const string &str, char whitespace = ' ');
void idf_input_string(istream &in, string &str);

void idf_output_string(ostream &out, const char *str, char whitespace = ' ');
void idf_input_string(istream &in, const char *&str);

template<class Element>
void idf_output_vector(ostream &out, const std::vector<Element> &vec);

template<class Element>
void idf_input_vector(istream &in, std::vector<Element> &vec);

#include "interrogate_datafile.I"

#endif
