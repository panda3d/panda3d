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
#include "geomTristrip.h"
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
  // Create some geometry on-the-fly to render the rope.
  if (get_num_subdiv() > 0) {
    NurbsCurveEvaluator *curve = get_curve();
    if (curve != (NurbsCurveEvaluator *)NULL) {
      PT(NurbsCurveResult) result = curve->evaluate(data._node_path.get_node_path());

      if (result->get_num_segments() > 0) {
        switch (get_render_mode()) {
        case RM_thread:
          render_thread(trav, data, result);
          break;
          
        case RM_billboard:
          render_billboard(trav, data, result);
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
    
    GeometricBoundingVolume *gbv;
    DCAST_INTO_R(gbv, bound, bound);
    gbv->around(&verts[0], &verts[verts.size() - 1]);
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
              NurbsCurveResult *result) {
  UVMode uv_mode = get_uv_mode();
  LVecBase2f uv_scale = get_uv_scale();
  bool use_vertex_color = get_use_vertex_color();

  PTA_Vertexf verts;
  PTA_TexCoordf uvs;
  PTA_Colorf colors;
  PTA_int lengths;

  int num_verts = get_num_subdiv() + 1;
  int num_segments = result->get_num_segments();
  float dist = 0.0f;
  for (int segment = 0; segment < num_segments; segment++) {
    LPoint3f last_point;
    for (int i = 0; i < num_verts; i++) {
      float t = (float)i / (float)(num_verts - 1);
      LPoint3f point;
      result->eval_segment_point(segment, t, point);
      verts.push_back(point);

      if (use_vertex_color) {
        Colorf color(result->eval_segment_extended_point(segment, t, 0),
                     result->eval_segment_extended_point(segment, t, 1),
                     result->eval_segment_extended_point(segment, t, 2),
                     result->eval_segment_extended_point(segment, t, 3));
        colors.push_back(color);
      }

      t = result->get_segment_t(segment, t);
      switch (uv_mode) {
      case UV_none:
        break;
        
      case UV_parametric:
        uvs.push_back(TexCoordf(t * uv_scale[0], 0.0f));
        break;

      case UV_distance:
        if (i != 0) {
          LVector3f vec = point - last_point;
          dist += vec.length();
        }
        uvs.push_back(TexCoordf(dist * uv_scale[0], 0.0f));
        break;

      case UV_distance2:
        if (i != 0) {
          LVector3f vec = point - last_point;
          dist += vec.length_squared();
        }
        uvs.push_back(TexCoordf(dist * uv_scale[0], 0.0f));
        break;
      }

      last_point = point;
    }
    lengths.push_back(num_verts);
  }
  
  PT(GeomLinestrip) geom = new GeomLinestrip;
  geom->set_width(get_thickness());
  geom->set_num_prims(num_segments);
  geom->set_coords(verts);
  if (uv_mode != UV_none) {
    geom->set_texcoords(uvs, G_PER_VERTEX);
  }

  if (use_vertex_color) {
    geom->set_colors(colors, G_PER_VERTEX);
  } else {
    colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
    geom->set_colors(colors, G_OVERALL);
  }
  geom->set_lengths(lengths);
  
  CullableObject *object = new CullableObject(geom, data._state,
                                              data._render_transform);
  trav->get_cull_handler()->record_object(object);
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::render_billboard
//       Access: Private
//  Description: Draws the rope in RM_billboard mode.  This draws a
//               series of triangle strips oriented to be
//               perpendicular to the camera plane.
//
//               In this mode, thickness is in spatial units, and
//               determines the with of the triangle strips.
////////////////////////////////////////////////////////////////////
void RopeNode::
render_billboard(CullTraverser *trav, CullTraverserData &data, 
                 NurbsCurveResult *result) {
  const TransformState *net_transform = data._net_transform;
  const TransformState *camera_transform = trav->get_camera_transform();

  CPT(TransformState) rel_transform =
    net_transform->invert_compose(camera_transform);
  LVector3f camera_vec = LVector3f::forward() * rel_transform->get_mat();

  float thickness = get_thickness();
  float radius = thickness * 0.5f;
  UVMode uv_mode = get_uv_mode();
  LVecBase2f uv_scale = get_uv_scale();

  // We can't just build one tristrip per segment.  Instead, we should
  // build one continuous tristrip for all connected segments, so we
  // can stitch them together properly at the seams.

  int num_verts = get_num_subdiv() + 1;
  int num_segments = result->get_num_segments();

  vector_Vertexf center_verts;
  vector_int center_lengths;
  vector_float center_t;

  LPoint3f point;
  int cur_length = 0;
  for (int segment = 0; segment < num_segments; segment++) {
    LPoint3f first_point;
    result->eval_segment_point(segment, 0.0f, first_point);
    if (cur_length == 0 || point != first_point) {
      // If the first point of this segment is different from the last
      // point of the previous segment, end the tristrip and store the
      // point.
      if (cur_length != 0) {
        center_lengths.push_back(cur_length);
      }
      center_verts.push_back(first_point);
      center_t.push_back(result->get_segment_t(segment, 0.0f));
      cur_length = 1;
    }

    // Store all the remaining points in this segment.
    for (int i = 1; i < num_verts; i++) {
      float t = (float)i / (float)(num_verts - 1);
      result->eval_segment_point(segment, t, point);
      center_verts.push_back(point);
      center_t.push_back(result->get_segment_t(segment, t));
      cur_length++;
    }
  }
  if (cur_length != 0) {
    center_lengths.push_back(cur_length);
  }

  // Now we have stored one or more sequences of vertices down the
  // center strips.  Go back and convert them into actual tristrips.

  PTA_Vertexf verts;
  PTA_TexCoordf uvs;
  PTA_Colorf colors;
  PTA_int lengths;

  int vi = 0;
  int num_prims = 0;
  float dist = 0.0f;
  for (int i = 0; i < (int)center_lengths.size(); i++) {
    int length = center_lengths[i];
    for (int j = 0; j < length; j++) {
      const Vertexf &point = center_verts[vi + j];
      float t = center_t[vi + j];
      LVector3f tangent;
      // Rather than evaluating the curve for the tangent, we derive
      // it from the neighboring points.  This gives us better
      // behavior at the endpoints, where the tangent might go to
      // zero.
      if (j == 0) {
        tangent = center_verts[vi + j + 1] - point;
      } else if (j == length - 1) {
        tangent = point - center_verts[vi + j - 1];
      } else {
        tangent = center_verts[vi + j + 1] - center_verts[vi + j - 1];
      }
      LVector3f cross = normalize(tangent.cross(camera_vec));
      cross *= radius;
      verts.push_back(point + cross);
      verts.push_back(point - cross);
      switch (uv_mode) {
      case UV_none:
        break;

      case UV_parametric:
        uvs.push_back(TexCoordf(t * uv_scale[0], uv_scale[1]));
        uvs.push_back(TexCoordf(t * uv_scale[0], 0.0f));
        break;

      case UV_distance:
        if (j != 0) {
          LVector3f vec = point - center_verts[vi + j - 1];
          dist += vec.length();
        }
        uvs.push_back(TexCoordf(dist * uv_scale[0], thickness * uv_scale[1]));
        uvs.push_back(TexCoordf(dist * uv_scale[0], 0.0f));
        break;

      case UV_distance2:
        if (j != 0) {
          LVector3f vec = point - center_verts[vi + j - 1];
          dist += vec.length_squared();
        }
        uvs.push_back(TexCoordf(dist * uv_scale[0], thickness * uv_scale[1]));
        uvs.push_back(TexCoordf(dist * uv_scale[0], 0.0f));
        break;
      }
    }
    vi += length;
    lengths.push_back(length * 2);
    num_prims++;
  }

  colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  
  PT(Geom) geom = new GeomTristrip;
  geom->set_num_prims(num_prims);
  geom->set_coords(verts);
  if (uv_mode != UV_none) {
    geom->set_texcoords(uvs, G_PER_VERTEX);
  }
  geom->set_colors(colors, G_OVERALL);
  geom->set_lengths(lengths);
  
  CullableObject *object = new CullableObject(geom, data._state,
                                              data._render_transform);
  trav->get_cull_handler()->record_object(object);
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
