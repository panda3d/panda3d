// Filename: ropeNode.cxx
// Created by:  drose (04Dec02)
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

#include "ropeNode.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "geomLinestrip.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle RopeNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *RopeNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void RopeNode::CData::
write_datagram(BamWriter *writer, Datagram &dg) const {
  // For now, we write a NULL pointer.  Eventually we will write out
  // the NurbsCurveEvaluator pointer.
  writer->write_pointer(dg, (TypedWritable *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RopeNode.
////////////////////////////////////////////////////////////////////
void RopeNode::CData::
fillin(DatagramIterator &scan, BamReader *reader) {
  // For now, we skip over the NULL pointer that we wrote out.
  reader->skip_pointer(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
RopeNode::
RopeNode(const string &name) :
  PandaNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
RopeNode::
RopeNode(const RopeNode &copy) :
  PandaNode(copy),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *RopeNode::
make_copy() const {
  return new RopeNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a RopeNode.
////////////////////////////////////////////////////////////////////
bool RopeNode::
safe_to_transform() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool RopeNode::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool RopeNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Create a new linestrip on-the-fly to render the rope.
  int num_verts = get_num_segs() + 1;
  if (num_verts >= 2) {
    PTA_Vertexf verts;
    PTA_int lengths;

    NurbsCurveEvaluator *curve = get_curve();
    PT(NurbsCurveResult) result = curve->evaluate(data._node_path.get_node_path());

    int num_segments = result->get_num_segments();
    if (num_segments > 0) {
      for (int segment = 0; segment < num_segments; segment++) {
        for (int i = 0; i < num_verts; i++) {
          float t = (float)i / (float)(num_verts - 1);
          LPoint3f point;
          result->eval_segment_point(segment, t, point);
          verts.push_back(point);
        }
        lengths.push_back(num_verts);
      }
      
      PT(Geom) geom = new GeomLinestrip;
      geom->set_num_prims(num_segments);
      geom->set_coords(verts);
      geom->set_lengths(lengths);
      
      CullableObject *object = new CullableObject(geom, data._state,
                                                  data._render_transform);
      trav->get_cull_handler()->record_object(object);
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RopeNode::
output(ostream &out) const {
  PandaNode::output(out);
  out << " " << get_curve();
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RopeNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
  indent(out, indent_level) << get_curve() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::reset_bound
//       Access: Published
//  Description: Recomputes the bounding volume.  This is normally
//               called automatically, but it must occasionally be
//               called explicitly when the curve has changed
//               properties outside of this node's knowledge.
////////////////////////////////////////////////////////////////////
BoundingVolume *RopeNode::
reset_bound(const NodePath &rel_to) {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = PandaNode::recompute_internal_bound();
  nassertr(bound != (BoundingVolume *)NULL, bound);

  NurbsCurveEvaluator *curve = get_curve();
  if (curve != (NurbsCurveEvaluator *)NULL) {
    pvector<LPoint3f> verts;
    get_curve()->get_vertices(verts, rel_to);

    GeometricBoundingVolume *gbv;
    DCAST_INTO_R(gbv, bound, bound);
    gbv->around(&verts[0], &verts[verts.size() - 1]);
  }
  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::recompute_internal_bound
//       Access: Protected, Virtual
//  Description: Called when needed to recompute the node's
//               _internal_bound object.  Nodes that contain anything
//               of substance should redefine this to do the right
//               thing.
////////////////////////////////////////////////////////////////////
BoundingVolume *RopeNode::
recompute_internal_bound() {
  return reset_bound(NodePath(this));
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               RopeNode.
////////////////////////////////////////////////////////////////////
void RopeNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void RopeNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type RopeNode is encountered
//               in the Bam file.  It should create the RopeNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *RopeNode::
make_from_bam(const FactoryParams &params) {
  RopeNode *node = new RopeNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RopeNode.
////////////////////////////////////////////////////////////////////
void RopeNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}
