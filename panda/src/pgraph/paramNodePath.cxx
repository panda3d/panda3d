/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramNodePath.cxx
 * @author rdb
 * @date 2015-02-25
 */

#include "paramNodePath.h"
#include "dcast.h"
#include "pandaNode.h"
#include "light.h"

TypeHandle ParamNodePath::_type_handle;

/**
 * Creates a new ParamNodePath storing the given node path object.
 */
ParamNodePath::
ParamNodePath(NodePath node_path) :
  _node_path(std::move(node_path))
{
  // The reason to construct a ParamNodePath is often to apply a light to a
  // shader input, so we want to keep track of the fact that the light is in
  // use.
  if (!_node_path.is_empty()) {
    Light *light = _node_path.node()->as_light();
    if (light != nullptr) {
      light->attrib_ref();
    }
  }
}

/**
 *
 */
ParamNodePath::
~ParamNodePath() {
  if (!_node_path.is_empty()) {
    Light *light = _node_path.node()->as_light();
    if (light != nullptr) {
      light->attrib_unref();
    }
  }
}

/**
 *
 */
void ParamNodePath::
output(std::ostream &out) const {
  out << "node path " << _node_path;
}

/**
 * Tells the BamReader how to create objects of type ParamValue.
 */
void ParamNodePath::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ParamNodePath::
write_datagram(BamWriter *manager, Datagram &dg) {
  ParamValueBase::write_datagram(manager, dg);

  if (manager->get_file_minor_ver() < 40) {
    // Before bam 6.40, we did not support writing NodePaths.  Instaed, we
    // write the PandaNode pointer and pray there is an unambiguous path.
    if (_node_path.is_empty()) {
      manager->write_pointer(dg, nullptr);
    } else {
      manager->write_pointer(dg, _node_path.node());
    }
  } else {
    _node_path.write_datagram(manager, dg);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int ParamNodePath::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ParamValueBase::complete_pointers(p_list, manager);

  if (manager->get_file_minor_ver() >= 40) {
    pi += _node_path.complete_pointers(p_list + pi, manager);
  } else {
    _node_path = NodePath(DCAST(PandaNode, p_list[pi++]));
  }

  if (!_node_path.is_empty()) {
    Light *light = _node_path.node()->as_light();
    if (light != nullptr) {
      light->attrib_ref();
    }
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ParamValue is encountered in the Bam file.  It should create the
 * ParamValue and extract its information from the file.
 */
TypedWritable *ParamNodePath::
make_from_bam(const FactoryParams &params) {
  ParamNodePath *param = new ParamNodePath;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ParamValue.
 */
void ParamNodePath::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParamValueBase::fillin(scan, manager);

  if (!_node_path.is_empty()) {
    Light *light = _node_path.node()->as_light();
    if (light != nullptr) {
      light->attrib_unref();
    }
    _node_path.clear();
  }

  if (manager->get_file_minor_ver() >= 40) {
    _node_path.fillin(scan, manager);
  } else {
    manager->read_pointer(scan);
  }
}
