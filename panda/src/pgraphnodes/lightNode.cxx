/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightNode.cxx
 * @author drose
 * @date 2002-03-26
 */

#include "lightNode.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle LightNode::_type_handle;

/**
 *
 */
LightNode::
LightNode(const std::string &name) :
  PandaNode(name)
{
}

/**
 *
 */
LightNode::
LightNode(const LightNode &copy) :
  Light(copy),
  PandaNode(copy)
{
}

/**
 * Returns the Light object upcast to a PandaNode.
 */
PandaNode *LightNode::
as_node() {
  return this;
}

/**
 * Cross-casts the node to a Light pointer, if it is one of the four kinds of
 * Light nodes, or returns NULL if it is not.
 */
Light *LightNode::
as_light() {
  return this;
}

/**
 *
 */
void LightNode::
output(std::ostream &out) const {
  PandaNode::output(out);
}

/**
 *
 */
void LightNode::
write(std::ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void LightNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  Light::write_datagram(manager, dg);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new LightNode.
 */
void LightNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  Light::fillin(scan, manager);
}
