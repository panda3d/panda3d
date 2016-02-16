/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rotate_to.h
 * @author drose
 * @date 1999-11-04
 */

#ifndef ROTATE_TO_H
#define ROTATE_TO_H

/*
 * rotate_to() This function computes a suitable rotation matrix to rotate
 * vector a onto vector b.  That is, it computes mat so that a * mat = b.  The
 * rotation axis is chosen to give the smallest possible rotation angle.
 */

#include <math.h>
#include "pandabase.h"
#include "luse.h"

BEGIN_PUBLISH

EXPCL_PANDA_MATHUTIL void rotate_to(LMatrix3f &mat, const LVector3f &a, const LVector3f &b);
EXPCL_PANDA_MATHUTIL void rotate_to(LMatrix3d &mat, const LVector3d &a, const LVector3d &b);

EXPCL_PANDA_MATHUTIL void rotate_to(LMatrix4f &mat, const LVector3f &a, const LVector3f &b);
EXPCL_PANDA_MATHUTIL void rotate_to(LMatrix4d &mat, const LVector3d &a, const LVector3d &b);

END_PUBLISH

#endif
