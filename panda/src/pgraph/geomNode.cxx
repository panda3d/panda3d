// Filename: geomNode.cxx
// Created by:  drose (23Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "geomNode.h"
#include "geomTransformer.h"
#include "sceneGraphReducer.h"
#include "accumulatedAttribs.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "indent.h"
#include "pset.h"

TypeHandle GeomNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::CData::
CData(const GeomNode::CData &copy) :
  _geoms(copy._geoms)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *GeomNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomNode::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  int num_geoms = _geoms.size();
  nassertv(num_geoms == (int)(PN_uint16)num_geoms);
  dg.add_uint16(num_geoms);
  
  Geoms::const_iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    manager->write_pointer(dg, entry._geom);
    manager->write_pointer(dg, entry._state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int GeomNode::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  // Get the geom and state pointers.
  Geoms::iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    GeomEntry &entry = (*gi);
    entry._geom = DCAST(Geom, p_list[pi++]);
    entry._state = DCAST(RenderState, p_list[pi++]);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomNode.
////////////////////////////////////////////////////////////////////
void GeomNode::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  int num_geoms = scan.get_uint16();
  // Read the list of geoms and states.  Push back a NULL for each one.
  _geoms.reserve(num_geoms);
  for (int i = 0; i < num_geoms; i++) {
    manager->read_pointer(scan);
    manager->read_pointer(scan);
    _geoms.push_back(GeomEntry(NULL, NULL));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::
GeomNode(const string &name) :
  PandaNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::
GeomNode(const GeomNode &copy) :
  PandaNode(copy),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::
~GeomNode() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated PandaNode that is a shallow
//               copy of this one.  It will be a different pointer,
//               but its internal data may or may not be shared with
//               that of the original PandaNode.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
PandaNode *GeomNode::
make_copy() const {
  return new GeomNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::apply_attribs_to_vertices
//       Access: Public, Virtual
//  Description: Applies whatever attributes are specified in the
//               AccumulatedAttribs object (and by the attrib_types
//               bitmask) to the vertices on this node, if
//               appropriate.  If this node uses geom arrays like a
//               GeomNode, the supplied GeomTransformer may be used to
//               unify shared arrays across multiple different nodes.
//
//               This is a generalization of xform().
////////////////////////////////////////////////////////////////////
void GeomNode::
apply_attribs_to_vertices(const AccumulatedAttribs &attribs, int attrib_types,
                          GeomTransformer &transformer) {
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Transforming geometry.\n";
  }

  if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
    if (!attribs._transform->is_identity()) {
      transformer.transform_vertices(this, attribs._transform->get_mat());
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_color) != 0) {
    if (attribs._color != (const RenderAttrib *)NULL) {
      const ColorAttrib *ca = DCAST(ColorAttrib, attribs._color);
      if (ca->get_color_type() == ColorAttrib::T_flat) {
        transformer.set_color(this, ca->get_color());
      }
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_color_scale) != 0) {
    if (attribs._color_scale != (const RenderAttrib *)NULL) {
      const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attribs._color_scale);
      if (csa->get_scale() != LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f)) {
        transformer.transform_colors(this, csa->get_scale());
      }
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_tex_matrix) != 0) {
    if (attribs._tex_matrix != (const RenderAttrib *)NULL) {
      const TexMatrixAttrib *tma = DCAST(TexMatrixAttrib, attribs._tex_matrix);
      if (tma->get_mat() != LMatrix4f::ident_mat()) {
        transformer.transform_texcoords(this, tma->get_mat());
      }
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_other) != 0) {
    if (!attribs._other->is_empty()) {
      transformer.apply_state(this, attribs._other);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.  For most
//               kinds of nodes, this does nothing.
//
//               For a GeomNode, this does the right thing, but it is
//               better to use a GeomTransformer instead, since it
//               will share the new arrays properly between different
//               GeomNodes.
////////////////////////////////////////////////////////////////////
void GeomNode::
xform(const LMatrix4f &mat) {
  GeomTransformer transformer;
  transformer.transform_vertices(this, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::combine_with
//       Access: Public, Virtual
//  Description: Collapses this node with the other node, if possible,
//               and returns a pointer to the combined node, or NULL
//               if the two nodes cannot safely be combined.
//
//               The return value may be this, other, or a new node
//               altogether.
//
//               This function is called from GraphReducer::flatten(),
//               and need not deal with children; its job is just to
//               decide whether to collapse the two nodes and what the
//               collapsed node should look like.
////////////////////////////////////////////////////////////////////
PandaNode *GeomNode::
combine_with(PandaNode *other) {
  if (is_exact_type(get_class_type()) &&
      other->is_exact_type(get_class_type())) {
    // Two GeomNodes can combine by moving Geoms from one to the other.
    GeomNode *gother = DCAST(GeomNode, other);
    add_geoms_from(gother);
    return this;
  }

  return PandaNode::combine_with(other);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::calc_tight_bounds
//       Access: Public, Virtual
//  Description: This is used to support
//               NodePath::calc_tight_bounds().  It is not intended to
//               be called directly, and it has nothing to do with the
//               normal Panda bounding-volume computation.
//
//               If the node contains any geometry, this updates
//               min_point and max_point to enclose its bounding box.
//               found_any is to be set true if the node has any
//               geometry at all, or left alone if it has none.  This
//               method may be called over several nodes, so it may
//               enter with min_point, max_point, and found_any
//               already set.
////////////////////////////////////////////////////////////////////
CPT(TransformState) GeomNode::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point, bool &found_any,
                  const TransformState *transform) const {
  CPT(TransformState) next_transform = 
    PandaNode::calc_tight_bounds(min_point, max_point, found_any, transform);

  const LMatrix4f &mat = next_transform->get_mat();
  int num_geoms = get_num_geoms();
  for (int i = 0; i < num_geoms; i++) {
    Geom *geom = get_geom(i);
    Geom::VertexIterator vi = geom->make_vertex_iterator();
    int num_prims = geom->get_num_prims();
    
    for (int p = 0; p < num_prims; p++) {
      int length = geom->get_length(p);
      for (int v = 0; v < length; v++) {
        Vertexf vertex = geom->get_next_vertex(vi) * mat;
        
        if (found_any) {
          min_point.set(min(min_point[0], vertex[0]),
                        min(min_point[1], vertex[1]),
                        min(min_point[2], vertex[2]));
          max_point.set(max(max_point[0], vertex[0]),
                        max(max_point[1], vertex[1]),
                        max(max_point[2], vertex[2]));
        } else {
          min_point = vertex;
          max_point = vertex;
          found_any = true;
        }
      }
    }
  }

  return next_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::add_geoms_from
//       Access: Published
//  Description: Copies the Geoms (and their associated RenderStates)
//               from the indicated GeomNode into this one.
////////////////////////////////////////////////////////////////////
void GeomNode::
add_geoms_from(const GeomNode *other) {
  CDReader cdata_other(other->_cycler);
  CDWriter cdata(_cycler);

  Geoms::const_iterator gi;
  for (gi = cdata_other->_geoms.begin(); 
       gi != cdata_other->_geoms.end(); 
       ++gi) {
    const GeomEntry &entry = (*gi);
    cdata->_geoms.push_back(entry);
  }
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::write_geoms
//       Access: Published
//  Description: Writes a short description of all the Geoms in the
//               node.
////////////////////////////////////////////////////////////////////
void GeomNode::
write_geoms(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);
  write(out, indent_level);
  Geoms::const_iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    indent(out, indent_level + 2) 
      << *entry._geom << " " << *entry._state << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::write_verbose
//       Access: Published
//  Description: Writes a detailed description of all the Geoms in the
//               node.
////////////////////////////////////////////////////////////////////
void GeomNode::
write_verbose(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);
  write(out, indent_level);
  Geoms::const_iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    indent(out, indent_level + 2) 
      << *entry._geom << " " << *entry._state << "\n";
    entry._geom->write_verbose(out, indent_level + 4);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomNode::
output(ostream &out) const {
  // Accumulate the total set of RenderAttrib types that are applied
  // to any of our Geoms, so we can output them too.  The result will
  // be the list of attrib types that might be applied to some Geoms,
  // but not necessarily to all Geoms.

  CDReader cdata(_cycler);

  pset<TypeHandle> attrib_types;
  Geoms::const_iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    int num_attribs = entry._state->get_num_attribs();
    for (int i = 0; i < num_attribs; i++) {
      const RenderAttrib *attrib = entry._state->get_attrib(i);
      attrib_types.insert(attrib->get_type());
    }
  }

  PandaNode::output(out);
  out << " (" << cdata->_geoms.size() << " geoms";

  if (!attrib_types.empty()) {
    out << ":";
    pset<TypeHandle>::const_iterator ai;
    for (ai = attrib_types.begin(); ai != attrib_types.end(); ++ai) {
      out << " " << (*ai);
    }
  }

  out << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::is_geom_node
//       Access: Public, Virtual
//  Description: A simple downcast check.  Returns true if this kind
//               of node happens to inherit from GeomNode, false
//               otherwise.
//
//               This is provided as a a faster alternative to calling
//               is_of_type(GeomNode::get_class_type()), since this
//               test is so important to rendering.
////////////////////////////////////////////////////////////////////
bool GeomNode::
is_geom_node() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::recompute_internal_bound
//       Access: Protected, Virtual
//  Description: Called when needed to recompute the node's
//               _internal_bound object.  Nodes that contain anything
//               of substance should redefine this to do the right
//               thing.
////////////////////////////////////////////////////////////////////
BoundingVolume *GeomNode::
recompute_internal_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = PandaNode::recompute_internal_bound();
  nassertr(bound != (BoundingVolume *)NULL, bound);

  // Now actually compute the bounding volume by putting it around all
  // of our geoms' bounding volumes.
  pvector<const BoundingVolume *> child_volumes;

  CDReader cdata(_cycler);
  Geoms::const_iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    const GeomEntry &entry = (*gi);
    child_volumes.push_back(&entry._geom->get_bound());
  }

  const BoundingVolume **child_begin = &child_volumes[0];
  const BoundingVolume **child_end = child_begin + child_volumes.size();

  bound->around(child_begin, child_end);
  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               GeomNode.
////////////////////////////////////////////////////////////////////
void GeomNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type GeomNode is encountered
//               in the Bam file.  It should create the GeomNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomNode::
make_from_bam(const FactoryParams &params) {
  GeomNode *node = new GeomNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomNode.
////////////////////////////////////////////////////////////////////
void GeomNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}
