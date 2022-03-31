/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pt_EggVertex.h
 * @author drose
 * @date 2001-02-22
 */

#ifndef PT_EGGVERTEX_H
#define PT_EGGVERTEX_H

#include "pandabase.h"

#include "eggVertex.h"
#include "pointerTo.h"

/**
 * A PT(EggVertex).  This is defined here solely we can explicitly export the
 * template class.  It's not strictly necessary, but it doesn't hurt.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EGG, EXPTP_PANDA_EGG, PointerToBase<EggVertex>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EGG, EXPTP_PANDA_EGG, PointerTo<EggVertex>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EGG, EXPTP_PANDA_EGG, ConstPointerTo<EggVertex>)

typedef PointerTo<EggVertex> PT_EggVertex;
typedef ConstPointerTo<EggVertex> CPT_EggVertex;

#endif
