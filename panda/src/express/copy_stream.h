/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file copy_stream.h
 * @author drose
 * @date 2009-08-27
 */

#ifndef COPY_STREAM_H
#define COPY_STREAM_H

#include "pandabase.h"

BEGIN_PUBLISH
EXPCL_PANDA_EXPRESS bool
copy_stream(std::istream &source, std::ostream &dest);
END_PUBLISH

#endif
