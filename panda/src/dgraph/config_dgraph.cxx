// Filename: config_dgraph.cxx
// Created by:  drose (01Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#if defined(WIN32_VC) && !defined(NO_PCH)
#include "dgraph_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "dgraph_headers.h"
#include "dataNode.h"
#include "dataRelation.h"
#include "intDataTransition.h"
#include "intDataAttribute.h"
#include "doubleDataTransition.h"
#include "doubleDataAttribute.h"
#include "vec3DataTransition.h"
#include "vec3DataAttribute.h"
#include "matrixDataTransition.h"
#include "matrixDataAttribute.h"
#include "buttonEventDataTransition.h"
#include "buttonEventDataAttribute.h"
#endif

#include <dconfig.h>

Configure(config_dgraph);
NotifyCategoryDef(dgraph, "");

ConfigureFn(config_dgraph) {
  DataNode::init_type();
  DataRelation::init_type();
  IntDataTransition::init_type();
  IntDataAttribute::init_type();
  DoubleDataTransition::init_type();
  DoubleDataAttribute::init_type();
  Vec3DataTransition::init_type();
  Vec3DataAttribute::init_type();
  MatrixDataTransition::init_type();
  MatrixDataAttribute::init_type();
  ButtonEventDataTransition::init_type();
  ButtonEventDataAttribute::init_type();

  DataRelation::register_with_factory();
}
