// Filename: config_sgraph.cxx
// Created by:  drose (12Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "config_sgraph.h"
#include "renderTraverser.h"
#include "geomNode.h"
#include "camera.h"
#include "planeNode.h"
#include "modelNode.h"
#include "modelRoot.h"
#include "projectionNode.h"

#include <dconfig.h>
#include <config_graph.h>

Configure(config_sgraph);
NotifyCategoryDef(sgraph, "");

ConfigureFn(config_sgraph) {
  init_libsgraph();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libsgraph
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libsgraph() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  init_libgraph();

  RenderTraverser::init_type();
  GeomNode::init_type();
  Camera::init_type();
  PlaneNode::init_type();
  ModelNode::init_type();
  ModelRoot::init_type();
  ProjectionNode::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  GeomNode::register_with_read_factory();
  ModelNode::register_with_read_factory();
  ModelRoot::register_with_read_factory();
}
