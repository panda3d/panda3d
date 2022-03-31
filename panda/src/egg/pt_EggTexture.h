/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pt_EggTexture.h
 * @author drose
 * @date 2001-05-01
 */

#ifndef PT_EGGTEXTURE_H
#define PT_EGGTEXTURE_H

#include "pandabase.h"

#include "eggTexture.h"
#include "pointerTo.h"

/**
 * A PT(EggTexture).  This is defined here solely we can explicitly export the
 * template class.  It's not strictly necessary, but it doesn't hurt.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EGG, EXPTP_PANDA_EGG, PointerToBase<EggTexture>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EGG, EXPTP_PANDA_EGG, PointerTo<EggTexture>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EGG, EXPTP_PANDA_EGG, ConstPointerTo<EggTexture>)

typedef PointerTo<EggTexture> PT_EggTexture;
typedef ConstPointerTo<EggTexture> CPT_EggTexture;

#endif
