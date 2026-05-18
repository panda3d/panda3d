/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file encrypt_string.h
 * @author drose
 * @date 2007-01-30
 */

#ifndef ENCRYPT_STRING_H
#define ENCRYPT_STRING_H

#include "pandabase.h"

#ifdef HAVE_OPENSSL

#include "filename.h"
#include "vector_uchar.h"

BEGIN_PUBLISH

EXPCL_PANDA_EXPRESS vector_uchar
encrypt_string(std::string_view source, std::string_view password,
               std::string algorithm = std::string(), int key_length = -1,
               int iteration_count = -1);
EXPCL_PANDA_EXPRESS std::string
decrypt_string(const vector_uchar &source, std::string_view password);

EXPCL_PANDA_EXPRESS bool
encrypt_file(const Filename &source, const Filename &dest, std::string_view password,
             std::string algorithm = std::string(), int key_length = -1,
             int iteration_count = -1);
EXPCL_PANDA_EXPRESS bool
decrypt_file(const Filename &source, const Filename &dest, std::string_view password);

EXPCL_PANDA_EXPRESS bool
encrypt_stream(std::istream &source, std::ostream &dest, std::string_view password,
               std::string algorithm = std::string(), int key_length = -1,
               int iteration_count = -1);
EXPCL_PANDA_EXPRESS bool
decrypt_stream(std::istream &source, std::ostream &dest, std::string_view password);

END_PUBLISH

#endif  // HAVE_OPENSSL

#endif
