// Filename: config_graph.cxx
// Created by:  drose (01Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_graph.h"
#include "namedNode.h"
#include "node.h"
#include "nodeRelation.h"
#include "nodeTransition.h"
#include "nodeAttribute.h"
#include "onOffTransition.h"
#include "onOffAttribute.h"
#include "multiNodeTransition.h"
#include "multiNodeAttribute.h"
#include "immediateTransition.h"
#include "immediateAttribute.h"

#include <dconfig.h>

Configure(config_graph);
NotifyCategoryDef(graph, "");
NotifyCategoryDef(wrt, graph_cat);

ConfigureFn(config_graph) {
  init_libgraph();
}

// Set this true if you want to cache some of the work of computing
// wrt(), so that the second time wrt() is called on a particular
// node-node pair it will be much cheaper than the first time.  This
// is the default behavior; you'd only want to turn it off if for some
// reason it was broken.  This is true by default and cannot be turned
// off in optimized (NDEBUG) mode.
const bool cache_wrt = config_graph.GetBool("cache-wrt", true);

// Set this true to force abort() to be called if an ambiguous wrt()
// call is made.  This will hopefully allow the programmer to get a
// stack dump and determine who is issuing the ambiguous wrt().  This
// cannot be turned on in optimized (NDEBUG) mode.
const bool ambiguous_wrt_abort = config_graph.GetBool("ambiguous-wrt-abort", false);

// Set this true to double-check the cached value of wrt(), above, by
// performing an explicit uncached wrt() and comparing the results.
// Obviously very slow.  This cannot be turned on in optimized
// (NDEBUG) mode.
const bool paranoid_wrt = config_graph.GetBool("paranoid-wrt", false);

// This is similar to paranoid-wrt, above.  Set it true to
// double-check each parent/unparent operation, to make sure that it
// always places the arc in the correct position in the parent's arc
// list, and that the arc list always remains properly sorted.  Again,
// it cannot be turned on in NDEBUG mode.
const bool paranoid_graph = config_graph.GetBool("paranoid-graph", false);


////////////////////////////////////////////////////////////////////
//     Function: init_libgraph
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libgraph() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  void init_last_graph_update();
  init_last_graph_update();

  BoundedObject::init_type();
  NamedNode::init_type();
  Node::init_type();
  NodeRelation::init_type();
  NodeTransition::init_type();
  NodeAttribute::init_type();
  OnOffTransition::init_type();
  OnOffAttribute::init_type();
  MultiNodeTransition::init_type();
  MultiNodeAttribute::init_type();
  ImmediateTransition::init_type();
  ImmediateAttribute::init_type();

  NodeRelation::register_with_factory();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  Node::register_with_read_factory();
  NamedNode::register_with_read_factory();
  NodeRelation::register_with_read_factory();
}
