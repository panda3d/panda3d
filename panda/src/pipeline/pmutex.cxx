/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmutex.cxx
 * @author drose
 * @date 2002-08-08
 */

#include "pmutex.h"
#include "thread.h"

Mutex Mutex::_notify_mutex;
