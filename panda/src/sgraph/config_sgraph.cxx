// Filename: config_sgraph.cxx
// Created by:  drose (12Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "config_sgraph.h"
#include "renderTraverser.h"
#include "geomNode.h"
#include "camera.h"
#include "planeNode.h"
#include "projectionNode.h"

#include <dconfig.h>

Configure(config_sgraph);
NotifyCategoryDef(sgraph, "");

ConfigureFn(config_sgraph) {
  RenderTraverser::init_type();
  GeomNode::init_type();
  Camera::init_type();
  PlaneNode::init_type();
  ProjectionNode::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  GeomNode::register_with_read_factory();
}
