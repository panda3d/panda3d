// Filename: config_switchnode.cxx
// Created by:  drose (19Jan00)
// 
////////////////////////////////////////////////////////////////////

#include "config_switchnode.h"
#include "LODNode.h"
#include "sequenceNode.h"

#include <dconfig.h>

Configure(config_switchnode);
NotifyCategoryDef(switchnode, "");

ConfigureFn(config_switchnode) {
  LODNode::init_type();
  SequenceNode::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  LODNode::register_with_read_factory();
  SequenceNode::register_with_read_factory();
}
