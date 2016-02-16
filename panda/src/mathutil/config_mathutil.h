/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_mathutil.h
 * @author drose
 * @date 1999-10-01
 */

#ifndef CONFIG_MATHUTIL_H
#define CONFIG_MATHUTIL_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableEnum.h"
#include "boundingVolume.h"

NotifyCategoryDecl(mathutil, EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL);

extern ConfigVariableDouble fft_offset;
extern ConfigVariableDouble fft_factor;
extern ConfigVariableDouble fft_exponent;
extern ConfigVariableDouble fft_error_threshold;
extern EXPCL_PANDA_MATHUTIL ConfigVariableEnum<BoundingVolume::BoundsType> bounds_type;

extern EXPCL_PANDA_MATHUTIL void init_libmathutil();

#endif
