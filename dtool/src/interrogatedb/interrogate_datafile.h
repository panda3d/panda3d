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

void idf_output_string(std::ostream &out, const std::string &str, char whitespace = ' ');
void idf_input_string(std::istream &in, std::string &str);

void idf_output_string(std::ostream &out, const char *str, char whitespace = ' ');
void idf_input_string(std::istream &in, const char *&str);

template<class Element>
void idf_output_vector(std::ostream &out, const std::vector<Element> &vec);

template<class Element>
void idf_input_vector(std::istream &in, std::vector<Element> &vec);

#include "interrogate_datafile.I"

#endif
