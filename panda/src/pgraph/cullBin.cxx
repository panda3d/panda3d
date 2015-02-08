// Filename: cullBin.cxx
// Created by:  drose (28Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "cullBin.h"
#include "config_pgraph.h"
#include "pandaNode.h"
#include "geomNode.h"
#include "cullableObject.h"
#include "decalEffect.h"
#include "string_utils.h"

PStatCollector CullBin::_cull_bin_pcollector("Cull:Sort");

TypeHandle CullBin::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullBin::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CullBin::
~CullBin() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::make_next
//       Access: Public, Virtual
//  Description: Returns a newly-allocated CullBin object that
//               contains a copy of just the subset of the data from
//               this CullBin object that is worth keeping around
//               for next frame.
//
//               If a particular CullBin object has no data worth
//               preserving till next frame, it is acceptable to
//               return NULL (which is the default behavior of this
//               method).
////////////////////////////////////////////////////////////////////
PT(CullBin) CullBin::
make_next() const {
  return (CullBin *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::finish_cull
//       Access: Public, Virtual
//  Description: Called after all the geoms have been added, this
//               indicates that the cull process is finished for this
//               frame and gives the bins a chance to do any
//               post-processing (like sorting) before moving on to
//               draw.
////////////////////////////////////////////////////////////////////
void CullBin::
finish_cull(SceneSetup *, Thread *) {
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::make_result_graph
//       Access: Public
//  Description: Returns a special scene graph constructed to
//               represent the results of the cull.  This will be a
//               single node with a list of GeomNode children, which
//               represent the various geom objects discovered by the
//               cull.
//
//               This is useful mainly for high-level debugging and
//               abstraction tools; it should not be mistaken for the
//               low-level cull result itself.  For the low-level cull
//               result, use draw() to efficiently draw the culled
//               scene.
////////////////////////////////////////////////////////////////////
PT(PandaNode) CullBin::
make_result_graph() {
  PT(PandaNode) root_node = new PandaNode(get_name());
  ResultGraphBuilder builder(root_node);
  fill_result_graph(builder);
  return root_node;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::ResultGraphBuilder::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CullBin::ResultGraphBuilder::
ResultGraphBuilder(PandaNode *root_node) :
  _object_index(0),
  _root_node(root_node)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::ResultGraphBuilder::add_object
//       Access: Public
//  Description: Called in fill_result_graph() by a derived CullBin
//               class to add each culled object to the result
//               returned by make_result_graph().
////////////////////////////////////////////////////////////////////
void CullBin::ResultGraphBuilder::
add_object(CullableObject *object) {
  if (_current_transform != object->_internal_transform ||
      _current_state != object->_state) {
    // Create a new GeomNode to hold the net transform and state.  We
    // choose to create a new GeomNode for each new state, to make it
    // clearer to the observer when the state changes.
    _current_transform = object->_internal_transform;
    _current_state = object->_state;
    _current_node = new GeomNode("object_" + format_string(_object_index));
    _root_node->add_child(_current_node);
    _current_node->set_transform(_current_transform);
    _current_node->set_state(_current_state);
  }

  record_one_object(_current_node, object);
  ++_object_index;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::ResultGraphBuilder::record_one_object
//       Access: Private
//  Description: Records a single object.
////////////////////////////////////////////////////////////////////
void CullBin::ResultGraphBuilder::
record_one_object(GeomNode *node, CullableObject *object) {
  PT(Geom) new_geom = object->_geom->make_copy();
  new_geom->set_vertex_data(object->_munged_data);
  node->add_geom(new_geom);
}
