// Filename: config_dgraph.cxx
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_dgraph.h"
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
