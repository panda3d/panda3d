// Filename: ropeNode.cxx
// Created by:  drose (04Dec02)
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

#include "ropeNode.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "geomLinestrip.h"
#include "geomTristrip.h"
#include "renderState.h"
#include "renderModeAttrib.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "pStatTimer.h"
#include "qpgeom.h"
#include "qpgeomLinestrips.h"
#include "qpgeomTristrips.h"
#include "qpgeomVertexWriter.h"

TypeHandle RopeNode::_type_handle;

PStatCollector RopeNode::_rope_node_pcollector("Cull:RopeNode");

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
  // Statistics
  PStatTimer timer(_rope_node_pcollector);

  // Create some geometry on-the-fly to render the rope.
  if (get_num_subdiv() > 0) {
    NurbsCurveEvaluator *curve = get_curve();
    if (curve != (NurbsCurveEvaluator *)NULL) {
      PT(NurbsCurveResult) result;
      if (has_matrix()) {
        result = curve->evaluate(data._node_path.get_node_path(), get_matrix());
      } else {
        result = curve->evaluate(data._node_path.get_node_path());
      }

      if (result->get_num_segments() > 0) {
        switch (get_render_mode()) {
        case RM_thread:
          render_thread(trav, data, result);
          break;
          
        case RM_tape:
          render_tape(trav, data, result);
          break;
          
        case RM_billboard:
          render_billboard(trav, data, result);
          break;
          
        case RM_tube:
          render_tube(trav, data, result);
          break;
        }
      }
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
  NurbsCurveEvaluator *curve = get_curve();
  if (curve != (NurbsCurveEvaluator *)NULL) {
    out << " " << *curve;
  } else {
    out << " (no curve)";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RopeNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
  indent(out, indent_level) << *get_curve() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::reset_bound
//       Access: Published
//  Description: Recomputes the bounding volume.  This is normally
//               called automatically, but it must occasionally be
//               called explicitly when the curve has changed
//               properties outside of this node's knowledge.
////////////////////////////////////////////////////////////////////
void RopeNode::
reset_bound(const NodePath &rel_to) {
  do_recompute_bound(rel_to);
  changed_internal_bound();
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
  return do_recompute_bound(NodePath(this));
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::get_format
//       Access: Private
//  Description: Returns the appropriate GeomVertexFormat for
//               rendering, according to the user-specified
//               requirements.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexFormat) RopeNode::
get_format(bool support_normals) const {
  PT(qpGeomVertexArrayFormat) array_format = new qpGeomVertexArrayFormat
    (InternalName::get_vertex(), 3, qpGeom::NT_float32,
     qpGeom::C_point);

  if (support_normals && get_normal_mode() == NM_vertex) {
    array_format->add_column
      (InternalName::get_normal(), 3, qpGeom::NT_float32,
       qpGeom::C_vector);
  }
  if (get_use_vertex_color()) {
    array_format->add_column
      (InternalName::get_color(), 1, qpGeom::NT_packed_dabc,
       qpGeom::C_color);
  }
  if (get_uv_mode() != UV_none) {
    array_format->add_column
      (InternalName::get_texcoord(), 2, qpGeom::NT_float32,
       qpGeom::C_texcoord);
  }

  return qpGeomVertexFormat::register_format(array_format);
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::do_recompute_bound
//       Access: Private
//  Description: Does the actual internal recompute.
////////////////////////////////////////////////////////////////////
BoundingVolume *RopeNode::
do_recompute_bound(const NodePath &rel_to) {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = PandaNode::recompute_internal_bound();
  nassertr(bound != (BoundingVolume *)NULL, bound);
  
  NurbsCurveEvaluator *curve = get_curve();
  if (curve != (NurbsCurveEvaluator *)NULL) {
    pvector<LPoint3f> verts;
    get_curve()->get_vertices(verts, rel_to);

    if (has_matrix()) {
      // And then apply the indicated matrix.
      const LMatrix4f &mat = get_matrix();
      pvector<LPoint3f>::iterator vi;
      for (vi = verts.begin(); vi != verts.end(); ++vi) {
        (*vi) = (*vi) * mat;
      }
    }
    
    GeometricBoundingVolume *gbv;
    DCAST_INTO_R(gbv, bound, bound);
    gbv->around(&verts[0], &verts[0] + verts.size());
  }
  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::render_thread
//       Access: Private
//  Description: Draws the rope in RM_thread mode.  This uses a
//               GeomLinestrip to draw the rope in the simplest
//               possible method, generally resulting in a
//               one-pixel-wide curve.
//
//               In this mode, the thickness parameter represents a
//               thickness in pixels, and is passed to the linestrip.
//               However, you should be aware the DirectX does not
//               support line thickness.
////////////////////////////////////////////////////////////////////
void RopeNode::
render_thread(CullTraverser *trav, CullTraverserData &data, 
              NurbsCurveResult *result) const {
  CurveSegments curve_segments;
  get_connected_segments(curve_segments, result);

  if (use_qpgeom) {
    // Now we have stored one or more sequences of vertices down the
    // center strips.  Go back through and calculate the vertices on
    // either side.
    PT(qpGeomVertexData) vdata = new qpGeomVertexData
      ("rope", get_format(false), qpGeom::UH_stream);

    compute_thread_vertices(vdata, curve_segments);
    
    PT(qpGeomLinestrips) strip = new qpGeomLinestrips(qpGeom::UH_stream);
    CurveSegments::const_iterator si;
    for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
      const CurveSegment &segment = (*si);
      
      strip->add_next_vertices(segment.size());
      strip->close_primitive();
    }

    PT(qpGeom) geom = new qpGeom;
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);

    CPT(RenderAttrib) thick = RenderModeAttrib::make(RenderModeAttrib::M_unchanged, get_thickness());
    CPT(RenderState) state = data._state->add_attrib(thick);
    
    CullableObject *object = new CullableObject(geom, state,
                                                data._render_transform);
    trav->get_cull_handler()->record_object(object, trav);

  } else {
    // Now we have stored one or more sequences of vertices down the
    // center strips.  Go back through and calculate the vertices on
    // either side.
    PTA_Vertexf verts;
    PTA_TexCoordf uvs;
    PTA_Colorf colors;
    
    compute_thread_vertices(verts, uvs, colors, curve_segments);

    // Finally, build the lengths array to make them into proper
    // line strips.
    
    PTA_int lengths;
    int num_prims = 0;
    
    CurveSegments::const_iterator si;
    for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
      const CurveSegment &segment = (*si);
      
      lengths.push_back(segment.size());
      num_prims++;
    }
    
    PT(GeomLinestrip) geom = new GeomLinestrip;
    geom->set_num_prims(num_prims);
    geom->set_coords(verts);
    if (get_uv_mode() != UV_none) {
      geom->set_texcoords(uvs, G_PER_VERTEX);
    }
    
    if (get_use_vertex_color()) {
      geom->set_colors(colors, G_PER_VERTEX);
    } else {
      geom->set_colors(colors, G_OVERALL);
    }
    geom->set_lengths(lengths);
    
    CPT(RenderAttrib) thick = RenderModeAttrib::make(RenderModeAttrib::M_unchanged, get_thickness());
    CPT(RenderState) state = data._state->add_attrib(thick);
    
    CullableObject *object = new CullableObject(geom, state,
                                                data._render_transform);
    trav->get_cull_handler()->record_object(object, trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::render_tape
//       Access: Private
//  Description: Draws the rope in RM_tape mode.  This draws a
//               series of triangle strips oriented to be
//               perpendicular to the tube_up vector.
//
//               In this mode, thickness is in spatial units, and
//               determines the width of the triangle strips.
////////////////////////////////////////////////////////////////////
void RopeNode::
render_tape(CullTraverser *trav, CullTraverserData &data, 
            NurbsCurveResult *result) const {
  CurveSegments curve_segments;
  get_connected_segments(curve_segments, result);

  if (use_qpgeom) {
    // Now we have stored one or more sequences of vertices down the
    // center strips.  Go back through and calculate the vertices on
    // either side.
    PT(qpGeomVertexData) vdata = new qpGeomVertexData
      ("rope", get_format(false), qpGeom::UH_stream);

    compute_billboard_vertices(vdata, -get_tube_up(), 
                               curve_segments, result);
    
    PT(qpGeomTristrips) strip = new qpGeomTristrips(qpGeom::UH_stream);
    CurveSegments::const_iterator si;
    for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
      const CurveSegment &segment = (*si);
      
      strip->add_next_vertices(segment.size() * 2);
      strip->close_primitive();
    }

    PT(qpGeom) geom = new qpGeom;
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);

    CullableObject *object = new CullableObject(geom, data._state,
                                                data._render_transform);
    trav->get_cull_handler()->record_object(object, trav);

  } else {
    // Now we have stored one or more sequences of vertices down the
    // center strips.  Go back through and calculate the vertices on
    // either side.
    
    PTA_Vertexf verts;
    PTA_TexCoordf uvs;
    PTA_Colorf colors;
    
    compute_billboard_vertices(verts, uvs, colors, -get_tube_up(), 
                               curve_segments, result);
    
    // Finally, build the lengths array to make them into proper
    // triangle strips.  We don't need a vindex array here, since the
    // vertices just happened to end up in tristrip order.
    
    PTA_int lengths;
    int num_prims = 0;
    
    CurveSegments::const_iterator si;
    for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
      const CurveSegment &segment = (*si);
      
      lengths.push_back(segment.size() * 2);
      num_prims++;
    }

    // And create a Geom for the rendering.
  
    PT(Geom) geom = new GeomTristrip;
    geom->set_num_prims(num_prims);
    geom->set_coords(verts);
    if (get_uv_mode() != UV_none) {
      geom->set_texcoords(uvs, G_PER_VERTEX);
    }
    if (get_use_vertex_color()) {
      geom->set_colors(colors, G_PER_VERTEX);
    } else {
      geom->set_colors(colors, G_OVERALL);
    }
    geom->set_lengths(lengths);
    
    CullableObject *object = new CullableObject(geom, data._state,
                                                data._render_transform);
    trav->get_cull_handler()->record_object(object, trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::render_billboard
//       Access: Private
//  Description: Draws the rope in RM_billboard mode.  This draws a
//               series of triangle strips oriented to be
//               perpendicular to the camera plane.
//
//               In this mode, thickness is in spatial units, and
//               determines the width of the triangle strips.
////////////////////////////////////////////////////////////////////
void RopeNode::
render_billboard(CullTraverser *trav, CullTraverserData &data, 
                 NurbsCurveResult *result) const {
  const TransformState *net_transform = data._net_transform;
  const TransformState *camera_transform = trav->get_camera_transform();

  CPT(TransformState) rel_transform =
    net_transform->invert_compose(camera_transform);
  LVector3f camera_vec = LVector3f::forward() * rel_transform->get_mat();

  CurveSegments curve_segments;
  get_connected_segments(curve_segments, result);

  if (use_qpgeom) {
    // Now we have stored one or more sequences of vertices down the
    // center strips.  Go back through and calculate the vertices on
    // either side.
    PT(qpGeomVertexData) vdata = new qpGeomVertexData
      ("rope", get_format(false), qpGeom::UH_stream);

    compute_billboard_vertices(vdata, camera_vec, 
                               curve_segments, result);
    
    PT(qpGeomTristrips) strip = new qpGeomTristrips(qpGeom::UH_stream);
    CurveSegments::const_iterator si;
    for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
      const CurveSegment &segment = (*si);
      
      strip->add_next_vertices(segment.size() * 2);
      strip->close_primitive();
    }

    PT(qpGeom) geom = new qpGeom;
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);

    CullableObject *object = new CullableObject(geom, data._state,
                                                data._render_transform);
    trav->get_cull_handler()->record_object(object, trav);

  } else {
    // Now we have stored one or more sequences of vertices down the
    // center strips.  Go back through and calculate the vertices on
    // either side.
    
    PTA_Vertexf verts;
    PTA_TexCoordf uvs;
    PTA_Colorf colors;
    
    compute_billboard_vertices(verts, uvs, colors, camera_vec, 
                               curve_segments, result);
    
    // Finally, build the lengths array to make them into proper
    // triangle strips.  We don't need a vindex array here, since the
    // vertices just happened to end up in tristrip order.
    
    PTA_int lengths;
    int num_prims = 0;
    
    CurveSegments::const_iterator si;
    for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
      const CurveSegment &segment = (*si);
      
      lengths.push_back(segment.size() * 2);
      num_prims++;
    }
    
    // And create a Geom for the rendering.
    
    PT(Geom) geom = new GeomTristrip;
    geom->set_num_prims(num_prims);
    geom->set_coords(verts);
    if (get_uv_mode() != UV_none) {
      geom->set_texcoords(uvs, G_PER_VERTEX);
    }
    if (get_use_vertex_color()) {
      geom->set_colors(colors, G_PER_VERTEX);
    } else {
      geom->set_colors(colors, G_OVERALL);
    }
    geom->set_lengths(lengths);
    
    CullableObject *object = new CullableObject(geom, data._state,
                                                data._render_transform);
    trav->get_cull_handler()->record_object(object, trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::render_tube
//       Access: Private
//  Description: Draws the rope in RM_tube mode.  This draws a hollow
//               tube centered around the string.
//
//               In this mode, thickness is in spatial units, and
//               determines the diameter of the tube.
////////////////////////////////////////////////////////////////////
void RopeNode::
render_tube(CullTraverser *trav, CullTraverserData &data, 
            NurbsCurveResult *result) const {
  CurveSegments curve_segments;
  get_connected_segments(curve_segments, result);

  // Now, we build up a table of vertices, in a series of rings
  // around the circumference of the tube.

  int num_slices = get_num_slices();
  int num_verts_per_slice;

  if (use_qpgeom) {
    PT(qpGeomVertexData) vdata = new qpGeomVertexData
      ("rope", get_format(true), qpGeom::UH_stream);

    compute_tube_vertices(vdata,
                          num_verts_per_slice, curve_segments, result);
    
    PT(qpGeomTristrips) strip = new qpGeomTristrips(qpGeom::UH_stream);
    // Finally, go through build up the index array, to tie all the
    // triangle strips together.
    int vi = 0;
    CurveSegments::const_iterator si;
    for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
      const CurveSegment &segment = (*si);
      
      for (int s = 0; s < num_slices; ++s) {
        int s1 = (s + 1) % num_verts_per_slice;
        
        for (size_t j = 0; j < segment.size(); ++j) {
          strip->add_vertex((vi + j) * num_verts_per_slice + s);
          strip->add_vertex((vi + j) * num_verts_per_slice + s1);
        }
        
        strip->close_primitive();
      }
      vi += (int)segment.size();
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
    
    compute_tube_vertices(verts, normals, uvs, colors,
                          num_verts_per_slice, curve_segments, result);
    
    // Finally, go back one more time and build up the vindex array, to
    // tie all the triangle strips together.
    
    PTA_ushort vindex;
    PTA_int lengths;
    
    int num_prims = 0;
    int vi = 0;
    CurveSegments::const_iterator si;
    for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
      const CurveSegment &segment = (*si);
      
      for (int s = 0; s < num_slices; ++s) {
        int s1 = (s + 1) % num_verts_per_slice;
        
        for (size_t j = 0; j < segment.size(); ++j) {
          vindex.push_back((vi + j) * num_verts_per_slice + s);
          vindex.push_back((vi + j) * num_verts_per_slice + s1);
        }
        
        lengths.push_back(segment.size() * 2);
        num_prims++;
      }
      vi += (int)segment.size();
    }
    
    // And create a Geom for the rendering.
    
    PT(Geom) geom = new GeomTristrip;
    geom->set_num_prims(num_prims);
    geom->set_coords(verts, vindex);
    if (get_uv_mode() != UV_none) {
      geom->set_texcoords(uvs, G_PER_VERTEX, vindex);
    }
    
    if (get_normal_mode() == NM_vertex) {
      geom->set_normals(normals, G_PER_VERTEX, vindex);
    }
    
    if (get_use_vertex_color()) {
      geom->set_colors(colors, G_PER_VERTEX, vindex);
    } else {
      geom->set_colors(colors, G_OVERALL);
    }
    geom->set_lengths(lengths);
    
    CullableObject *object = new CullableObject(geom, data._state,
                                                data._render_transform);
    trav->get_cull_handler()->record_object(object, trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::get_connected_segments
//       Access: Private
//  Description: Evaluates the string of vertices along the curve, and
//               also breaks them up into connected segments.
//
//               Since the NurbsCurveEvaluator describes the curve as
//               a sequence of possibly-connected piecewise continuous
//               segments, this means joining together some adjacent
//               segments from the NurbsCurveEvaluator into a single
//               CurveSegment, if they happen to be connected (as most
//               will be).
////////////////////////////////////////////////////////////////////
void RopeNode::
get_connected_segments(RopeNode::CurveSegments &curve_segments,
                       const NurbsCurveResult *result) const {
  int num_verts = get_num_subdiv() + 1;
  int num_segments = result->get_num_segments();
  bool use_vertex_color = get_use_vertex_color();

  CurveSegment *curve_segment = NULL;
  LPoint3f last_point;

  for (int segment = 0; segment < num_segments; ++segment) {
    LPoint3f point;
    result->eval_segment_point(segment, 0.0f, point);

    if (curve_segment == (CurveSegment *)NULL || 
        !point.almost_equal(last_point)) {
      // If the first point of this segment is different from the last
      // point of the previous segment, end the previous segment and
      // begin a new one.
      curve_segments.push_back(CurveSegment());
      curve_segment = &curve_segments.back();

      CurveVertex vtx;
      vtx._p = point;
      vtx._t = result->get_segment_t(segment, 0.0f);
      if (use_vertex_color) {
        result->eval_segment_extended_points(segment, 0.0f, 0, &vtx._c[0], 4);
      }

      curve_segment->push_back(vtx);
    }

    // Store all the remaining points in this segment.
    for (int i = 1; i < num_verts; ++i) {
      float t = (float)i / (float)(num_verts - 1);

      CurveVertex vtx;
      result->eval_segment_point(segment, t, vtx._p);
      vtx._t = result->get_segment_t(segment, t);
      if (use_vertex_color) {
        result->eval_segment_extended_points(segment, t, 0, &vtx._c[0], 4);
      }

      curve_segment->push_back(vtx);

      last_point = vtx._p;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::compute_thread_vertices
//       Access: Private
//  Description: Calculates the vertices for a RM_thread render.  This
//               just copies the vertices more-or-less directly into
//               the array.
////////////////////////////////////////////////////////////////////
void RopeNode::
compute_thread_vertices(qpGeomVertexData *vdata,
                        const RopeNode::CurveSegments &curve_segments) const {
  qpGeomVertexWriter vertex(vdata, InternalName::get_vertex());
  qpGeomVertexWriter color(vdata, InternalName::get_color());
  qpGeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  UVMode uv_mode = get_uv_mode();
  float uv_scale = get_uv_scale();
  bool u_dominant = get_uv_direction();
  bool use_vertex_color = get_use_vertex_color();

  float dist = 0.0f;
  CurveSegments::const_iterator si;
  for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
    const CurveSegment &segment = (*si);
    for (size_t j = 0; j < segment.size(); ++j) {
      vertex.add_data3f(segment[j]._p);

      if (use_vertex_color) {
        color.add_data4f(segment[j]._c);
      }

      float uv_t = compute_uv_t(dist, uv_mode, uv_scale, segment, j);

      if (uv_mode != UV_none) {
        if (u_dominant) {
          texcoord.add_data2f(uv_t, 0.0f);
        } else {
          texcoord.add_data2f(0.0f, uv_t);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::compute_billboard_vertices
//       Access: Private
//  Description: Calculates the vertices for a RM_billboard render.  This
//               puts a pair of vertices on either side of each
//               computed point in curve_segments.
////////////////////////////////////////////////////////////////////
void RopeNode::
compute_billboard_vertices(qpGeomVertexData *vdata,
                           const LVector3f &camera_vec,
                           const RopeNode::CurveSegments &curve_segments,
                           NurbsCurveResult *result) const {
  qpGeomVertexWriter vertex(vdata, InternalName::get_vertex());
  qpGeomVertexWriter color(vdata, InternalName::get_color());
  qpGeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  float thickness = get_thickness();
  float radius = thickness * 0.5f;
  UVMode uv_mode = get_uv_mode();
  float uv_scale = get_uv_scale();
  bool u_dominant = get_uv_direction();
  bool use_vertex_color = get_use_vertex_color();

  float dist = 0.0f;
  CurveSegments::const_iterator si;
  for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
    const CurveSegment &segment = (*si);
    for (size_t j = 0; j < segment.size(); ++j) {
      LVector3f tangent;
      compute_tangent(tangent, segment, j, result);

      LVector3f norm = cross(tangent, camera_vec);
      norm.normalize();

      vertex.add_data3f(segment[j]._p + norm * radius);
      vertex.add_data3f(segment[j]._p - norm * radius);

      if (use_vertex_color) {
        color.add_data4f(segment[j]._c);
        color.add_data4f(segment[j]._c);
      }

      float uv_t = compute_uv_t(dist, uv_mode, uv_scale, segment, j);

      if (uv_mode != UV_none) {
        if (u_dominant) {
          texcoord.add_data2f(uv_t, 1.0f);
          texcoord.add_data2f(uv_t, 0.0f);
        } else {
          texcoord.add_data2f(1.0f, uv_t);
          texcoord.add_data2f(0.0f, uv_t);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::compute_tube_vertices
//       Access: Private
//  Description: Calculates the vertices for a RM_tube render.  This
//               puts a ring of vertices around each computed point in
//               curve_segments.
////////////////////////////////////////////////////////////////////
void RopeNode::
compute_tube_vertices(qpGeomVertexData *vdata,
                      int &num_verts_per_slice,
                      const RopeNode::CurveSegments &curve_segments,
                      NurbsCurveResult *result) const {
  qpGeomVertexWriter vertex(vdata, InternalName::get_vertex());
  qpGeomVertexWriter normal(vdata, InternalName::get_normal());
  qpGeomVertexWriter color(vdata, InternalName::get_color());
  qpGeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  int num_slices = get_num_slices();
  num_verts_per_slice = num_slices;

  float thickness = get_thickness();
  float radius = thickness * 0.5f;
  UVMode uv_mode = get_uv_mode();
  float uv_scale = get_uv_scale();
  bool u_dominant = get_uv_direction();
  NormalMode normal_mode = get_normal_mode();
  bool use_vertex_color = get_use_vertex_color();

  // If we are generating UV's, we will need to duplicate the vertices
  // along the seam so that the UV's go through the whole range of
  // 0..1 instead of reflecting in the last polygon before the seam.
  if (uv_mode != UV_none) {
    ++num_verts_per_slice;
  }

  LVector3f up = get_tube_up();

  float dist = 0.0f;
  CurveSegments::const_iterator si;
  for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
    const CurveSegment &segment = (*si);
    for (size_t j = 0; j < segment.size(); ++j) {
      LVector3f tangent;
      compute_tangent(tangent, segment, j, result);

      LVector3f norm = cross(tangent, up);
      norm.normalize();
      up = cross(norm, tangent);

      LMatrix3f rotate = LMatrix3f::rotate_mat(360.0f / (float)num_slices,
                                               tangent);

      float uv_t = compute_uv_t(dist, uv_mode, uv_scale, segment, j);

      for (int s = 0; s < num_verts_per_slice; ++s) {
        vertex.add_data3f(segment[j]._p + norm * radius);

        if (normal_mode == NM_vertex) {
          normal.add_data3f(norm);
        }

        if (use_vertex_color) {
          color.add_data4f(segment[j]._c);
        }

        norm = norm * rotate;

        if (uv_mode != UV_none) {
          float uv_s = (float)s / (float)num_slices;
          if (u_dominant) {
            texcoord.add_data2f(uv_t, uv_s);
          } else {
            texcoord.add_data2f(uv_s, uv_t);
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::compute_thread_vertices
//       Access: Private
//  Description: Calculates the vertices for a RM_thread render.  This
//               just copies the vertices more-or-less directly into
//               the array.
////////////////////////////////////////////////////////////////////
void RopeNode::
compute_thread_vertices(PTA_Vertexf &verts, PTA_TexCoordf &uvs, 
                        PTA_Colorf &colors,
                        const RopeNode::CurveSegments &curve_segments) const {
  UVMode uv_mode = get_uv_mode();
  float uv_scale = get_uv_scale();
  bool u_dominant = get_uv_direction();
  bool use_vertex_color = get_use_vertex_color();

  float dist = 0.0f;
  CurveSegments::const_iterator si;
  for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
    const CurveSegment &segment = (*si);
    for (size_t j = 0; j < segment.size(); ++j) {
      verts.push_back(segment[j]._p);

      if (use_vertex_color) {
        colors.push_back(segment[j]._c);
      }

      float uv_t = compute_uv_t(dist, uv_mode, uv_scale, segment, j);

      if (uv_mode != UV_none) {
        if (u_dominant) {
          uvs.push_back(TexCoordf(uv_t, 0.0f));
        } else {
          uvs.push_back(TexCoordf(0.0f, uv_t));
        }
      }
    }
  }

  if (!use_vertex_color) {
    colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::compute_billboard_vertices
//       Access: Private
//  Description: Calculates the vertices for a RM_billboard render.  This
//               puts a pair of vertices on either side of each
//               computed point in curve_segments.
////////////////////////////////////////////////////////////////////
void RopeNode::
compute_billboard_vertices(PTA_Vertexf &verts, PTA_TexCoordf &uvs, 
                           PTA_Colorf &colors, const LVector3f &camera_vec,
                           const RopeNode::CurveSegments &curve_segments,
                           NurbsCurveResult *result) const {
  float thickness = get_thickness();
  float radius = thickness * 0.5f;
  UVMode uv_mode = get_uv_mode();
  float uv_scale = get_uv_scale();
  bool u_dominant = get_uv_direction();
  bool use_vertex_color = get_use_vertex_color();

  float dist = 0.0f;
  CurveSegments::const_iterator si;
  for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
    const CurveSegment &segment = (*si);
    for (size_t j = 0; j < segment.size(); ++j) {
      LVector3f tangent;
      compute_tangent(tangent, segment, j, result);

      LVector3f normal = cross(tangent, camera_vec);
      normal.normalize();

      verts.push_back(segment[j]._p + normal * radius);
      verts.push_back(segment[j]._p - normal * radius);

      if (use_vertex_color) {
        colors.push_back(segment[j]._c);
        colors.push_back(segment[j]._c);
      }

      float uv_t = compute_uv_t(dist, uv_mode, uv_scale, segment, j);

      if (uv_mode != UV_none) {
        if (u_dominant) {
          uvs.push_back(TexCoordf(uv_t, 1.0f));
          uvs.push_back(TexCoordf(uv_t, 0.0f));
        } else {
          uvs.push_back(TexCoordf(1.0f, uv_t));
          uvs.push_back(TexCoordf(0.0f, uv_t));
        }
      }
    }
  }

  if (!use_vertex_color) {
    colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::compute_tube_vertices
//       Access: Private
//  Description: Calculates the vertices for a RM_tube render.  This
//               puts a ring of vertices around each computed point in
//               curve_segments.
////////////////////////////////////////////////////////////////////
void RopeNode::
compute_tube_vertices(PTA_Vertexf &verts, PTA_Normalf &normals,
                      PTA_TexCoordf &uvs, PTA_Colorf &colors,
                      int &num_verts_per_slice,
                      const RopeNode::CurveSegments &curve_segments,
                      NurbsCurveResult *result) const {
  int num_slices = get_num_slices();
  num_verts_per_slice = num_slices;

  float thickness = get_thickness();
  float radius = thickness * 0.5f;
  UVMode uv_mode = get_uv_mode();
  float uv_scale = get_uv_scale();
  bool u_dominant = get_uv_direction();
  NormalMode normal_mode = get_normal_mode();
  bool use_vertex_color = get_use_vertex_color();

  // If we are generating UV's, we will need to duplicate the vertices
  // along the seam so that the UV's go through the whole range of
  // 0..1 instead of reflecting in the last polygon before the seam.
  if (uv_mode != UV_none) {
    ++num_verts_per_slice;
  }

  LVector3f up = get_tube_up();

  float dist = 0.0f;
  CurveSegments::const_iterator si;
  for (si = curve_segments.begin(); si != curve_segments.end(); ++si) {
    const CurveSegment &segment = (*si);
    for (size_t j = 0; j < segment.size(); ++j) {
      LVector3f tangent;
      compute_tangent(tangent, segment, j, result);

      LVector3f normal = cross(tangent, up);
      normal.normalize();
      up = cross(normal, tangent);

      LMatrix3f rotate = LMatrix3f::rotate_mat(360.0f / (float)num_slices,
                                               tangent);

      float uv_t = compute_uv_t(dist, uv_mode, uv_scale, segment, j);

      for (int s = 0; s < num_verts_per_slice; ++s) {
        verts.push_back(segment[j]._p + normal * radius);

        if (normal_mode == NM_vertex) {
          normals.push_back(normal);
        }

        if (use_vertex_color) {
          colors.push_back(segment[j]._c);
        }

        normal = normal * rotate;

        if (uv_mode != UV_none) {
          float uv_s = (float)s / (float)num_slices;
          if (u_dominant) {
            uvs.push_back(TexCoordf(uv_t, uv_s));
          } else {
            uvs.push_back(TexCoordf(uv_s, uv_t));
          }
        }
      }
    }
  }

  if (!use_vertex_color) {
    colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::compute_tangent
//       Access: Private, Static
//  Description: Computes the tangent to the curve at the indicated
//               point in the segment.
////////////////////////////////////////////////////////////////////
void RopeNode::
compute_tangent(LVector3f &tangent, const RopeNode::CurveSegment &segment, 
                size_t j, NurbsCurveResult *result) {
  // First, try to evaluate the tangent at the curve.  This gives
  // better results at the ends at the endpoints where the tangent
  // does not go to zero.

  /*
    Actually, on second thought this looks terrible.

  if (result->eval_tangent(segment[j]._t, tangent)) {
    if (!tangent.almost_equal(LVector3f::zero())) {
      return;
    }
  }
  */

  // If that failed (or produced a zero tangent), then derive the
  // tangent from the neighboring points instead.
  if (j == 0) {
    tangent = segment[j + 1]._p - segment[j]._p;
  } else if (j == segment.size() - 1) {
    tangent = segment[j]._p - segment[j - 1]._p;
  } else {
    tangent = segment[j + 1]._p - segment[j - 1]._p;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::compute_uv_t
//       Access: Private, Static
//  Description: Computes the texture coordinate along the curve for
//               the indicated point in the segment.
////////////////////////////////////////////////////////////////////
float RopeNode::
compute_uv_t(float &dist, const RopeNode::UVMode &uv_mode,
             float uv_scale, const RopeNode::CurveSegment &segment,
             size_t j) {
  switch (uv_mode) {
  case UV_none:
    return 0.0f;
    
  case UV_parametric:
    return segment[j]._t * uv_scale;
    
  case UV_distance:
    if (j != 0) {
      LVector3f vec = segment[j]._p - segment[j - 1]._p;
      dist += vec.length();
    }
    return dist * uv_scale;
    
  case UV_distance2:
    if (j != 0) {
      LVector3f vec = segment[j]._p - segment[j - 1]._p;
      dist += vec.length_squared();
    }
    return dist * uv_scale;
  }

  return 0.0f;
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
