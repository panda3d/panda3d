// Filename: sheetNode.cxx
// Created by:  drose (11Oct03)
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

#include "sheetNode.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "geomTristrip.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "pStatTimer.h"
#include "qpgeom.h"
#include "qpgeomTristrips.h"
#include "qpgeomVertexWriter.h"

TypeHandle SheetNode::_type_handle;

PStatCollector SheetNode::_sheet_node_pcollector("Cull:SheetNode");

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *SheetNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void SheetNode::CData::
write_datagram(BamWriter *writer, Datagram &dg) const {
  // For now, we write a NULL pointer.  Eventually we will write out
  // the NurbsSurfaceEvaluator pointer.
  writer->write_pointer(dg, (TypedWritable *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new SheetNode.
////////////////////////////////////////////////////////////////////
void SheetNode::CData::
fillin(DatagramIterator &scan, BamReader *reader) {
  // For now, we skip over the NULL pointer that we wrote out.
  reader->skip_pointer(scan);
  _surface.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SheetNode::
SheetNode(const string &name) :
  PandaNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
SheetNode::
SheetNode(const SheetNode &copy) :
  PandaNode(copy),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *SheetNode::
make_copy() const {
  return new SheetNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a SheetNode.
////////////////////////////////////////////////////////////////////
bool SheetNode::
safe_to_transform() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool SheetNode::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::cull_callback
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
bool SheetNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Statistics
  PStatTimer timer(_sheet_node_pcollector);

  // Create some geometry on-the-fly to render the sheet.
  if (get_num_u_subdiv() > 0 && get_num_v_subdiv() > 0) {
    NurbsSurfaceEvaluator *surface = get_surface();
    if (surface != (NurbsSurfaceEvaluator *)NULL) {
      PT(NurbsSurfaceResult) result = surface->evaluate(data._node_path.get_node_path());
      
      if (result->get_num_u_segments() > 0 && result->get_num_v_segments() > 0) {
        render_sheet(trav, data, result);
      }
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void SheetNode::
output(ostream &out) const {
  PandaNode::output(out);
  NurbsSurfaceEvaluator *surface = get_surface();
  if (surface != (NurbsSurfaceEvaluator *)NULL) {
    out << " " << *surface;
  } else {
    out << " (no surface)";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void SheetNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
  NurbsSurfaceEvaluator *surface = get_surface();
  if (surface != (NurbsSurfaceEvaluator *)NULL) {
    indent(out, indent_level + 2) << *surface << "\n";
  } else {
    indent(out, indent_level + 2) << "(no surface)\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::reset_bound
//       Access: Published
//  Description: Recomputes the bounding volume.  This is normally
//               called automatically, but it must occasionally be
//               called explicitly when the surface has changed
//               properties outside of this node's knowledge.
////////////////////////////////////////////////////////////////////
void SheetNode::
reset_bound(const NodePath &rel_to) {
  do_recompute_bound(rel_to);
  changed_internal_bound();
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::recompute_internal_bound
//       Access: Protected, Virtual
//  Description: Called when needed to recompute the node's
//               _internal_bound object.  Nodes that contain anything
//               of substance should redefine this to do the right
//               thing.
////////////////////////////////////////////////////////////////////
BoundingVolume *SheetNode::
recompute_internal_bound() {
  return do_recompute_bound(NodePath(this));
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::do_recompute_bound
//       Access: Private
//  Description: Does the actual internal recompute.
////////////////////////////////////////////////////////////////////
BoundingVolume *SheetNode::
do_recompute_bound(const NodePath &rel_to) {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = PandaNode::recompute_internal_bound();
  nassertr(bound != (BoundingVolume *)NULL, bound);
  
  NurbsSurfaceEvaluator *surface = get_surface();
  if (surface != (NurbsSurfaceEvaluator *)NULL) {
    pvector<LPoint3f> verts;
    get_surface()->get_vertices(verts, rel_to);
    
    GeometricBoundingVolume *gbv;
    DCAST_INTO_R(gbv, bound, bound);
    gbv->around(&verts[0], &verts[0] + verts.size());
  }
  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::render_sheet
//       Access: Private
//  Description: Draws the sheet as a series of tristrips along its
//               length.
////////////////////////////////////////////////////////////////////
void SheetNode::
render_sheet(CullTraverser *trav, CullTraverserData &data, 
             NurbsSurfaceResult *result) {
  bool use_vertex_color = get_use_vertex_color();

  int num_u_segments = result->get_num_u_segments();
  int num_v_segments = result->get_num_v_segments();
  int num_u_verts = get_num_u_subdiv() + 1;
  int num_v_verts = get_num_v_subdiv() + 1;

  if (use_qpgeom) {
    CPT(qpGeomVertexFormat) format;
    if (use_vertex_color) {
      format = qpGeomVertexFormat::get_v3n3cpt2();
    } else {
      format = qpGeomVertexFormat::get_v3n3t2();
    }
    PT(qpGeomVertexData) vdata = new qpGeomVertexData
      ("sheet", format, qpGeom::UH_stream);
    qpGeomVertexWriter vertex(vdata, InternalName::get_vertex());
    qpGeomVertexWriter normal(vdata, InternalName::get_normal());
    qpGeomVertexWriter color(vdata, InternalName::get_color());
    qpGeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
    
    PT(qpGeomTristrips) strip = new qpGeomTristrips(qpGeom::UH_stream);
    for (int ui = 0; ui < num_u_segments; ui++) {
      for (int uni = 0; uni < num_u_verts; uni++) {
        float u0 = (float)uni / (float)num_u_verts;
        float u1 = (float)(uni + 1) / (float)num_u_verts;
        float u0_tc = result->get_segment_u(ui, u0);
        float u1_tc = result->get_segment_u(ui, u1);

        for (int vi = 0; vi < num_v_segments; vi++) {
          for (int vni = 0; vni < num_v_verts; vni++) {
            float v = (float)vni / (float)(num_v_verts - 1);
            float v_tc = result->get_segment_v(vi, v);

            LPoint3f point;
            LVector3f norm;
            result->eval_segment_point(ui, vi, u0, v, point);
            result->eval_segment_normal(ui, vi, u0, v, norm);
            vertex.add_data3f(point);
            normal.add_data3f(norm);
            texcoord.add_data2f(u0_tc, v_tc);

            result->eval_segment_point(ui, vi, u1, v, point);
            result->eval_segment_normal(ui, vi, u1, v, norm);
            vertex.add_data3f(point);
            normal.add_data3f(norm);
            texcoord.add_data2f(u1_tc, v_tc);

            if (use_vertex_color) {
              Colorf c0, c1;
              result->eval_segment_extended_points(ui, vi, u0, v, 0, &c0[0], 4);
              result->eval_segment_extended_points(ui, vi, u1, v, 0, &c1[0], 4);

              color.add_data4f(c0);
              color.add_data4f(c1);
            }
          }
          strip->add_next_vertices(num_v_verts * 2);
          strip->close_primitive();
        }
      }
    }

    PT(qpGeom) geom = new qpGeom;
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);

    CullableObject *object = new CullableObject(geom, data._state,
                                                data._render_transform);
    trav->get_cull_handler()->record_object(object, trav);

  } else {
    PTA_Vertexf verts;
    PTA_Normalf normals;
    PTA_TexCoordf uvs;
    PTA_Colorf colors;
    PTA_int lengths;

    // We define a series of triangle strips, parallel to the V
    // direction.

    for (int ui = 0; ui < num_u_segments; ui++) {
      for (int uni = 0; uni < num_u_verts; uni++) {
        float u0 = (float)uni / (float)num_u_verts;
        float u1 = (float)(uni + 1) / (float)num_u_verts;
        float u0_tc = result->get_segment_u(ui, u0);
        float u1_tc = result->get_segment_u(ui, u1);

        for (int vi = 0; vi < num_v_segments; vi++) {
          for (int vni = 0; vni < num_v_verts; vni++) {
            float v = (float)vni / (float)(num_v_verts - 1);
            float v_tc = result->get_segment_v(vi, v);

            LPoint3f point;
            LVector3f normal;
            result->eval_segment_point(ui, vi, u0, v, point);
            result->eval_segment_normal(ui, vi, u0, v, normal);
            verts.push_back(point);
            normals.push_back(normal);
            uvs.push_back(TexCoordf(u0_tc, v_tc));

            result->eval_segment_point(ui, vi, u1, v, point);
            result->eval_segment_normal(ui, vi, u1, v, normal);
            verts.push_back(point);
            normals.push_back(normal);
            uvs.push_back(TexCoordf(u1_tc, v_tc));

            if (use_vertex_color) {
              Colorf c0, c1;
              result->eval_segment_extended_points(ui, vi, u0, v, 0, &c0[0], 4);
              result->eval_segment_extended_points(ui, vi, u1, v, 0, &c1[0], 4);

              colors.push_back(c0);
              colors.push_back(c1);
            }
          }
          lengths.push_back(num_v_verts * 2);
        }
      }
    }

    colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  
    PT(GeomTristrip) geom = new GeomTristrip;
    geom->set_num_prims(lengths.size());
    geom->set_coords(verts);
    geom->set_normals(normals, G_PER_VERTEX);
    geom->set_texcoords(uvs, G_PER_VERTEX);

    if (use_vertex_color) {
      geom->set_colors(colors, G_PER_VERTEX);
    } else {
      colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
      geom->set_colors(colors, G_OVERALL);
    }
    geom->set_lengths(lengths);
  
    CullableObject *object = new CullableObject(geom, data._state,
                                                data._render_transform);
    trav->get_cull_handler()->record_object(object, trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               SheetNode.
////////////////////////////////////////////////////////////////////
void SheetNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void SheetNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type SheetNode is encountered
//               in the Bam file.  It should create the SheetNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *SheetNode::
make_from_bam(const FactoryParams &params) {
  SheetNode *node = new SheetNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new SheetNode.
////////////////////////////////////////////////////////////////////
void SheetNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}
