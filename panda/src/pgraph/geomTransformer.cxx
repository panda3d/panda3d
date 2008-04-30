// Filename: geomTransformer.cxx
// Created by:  drose (14Mar02)
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

#include "geomTransformer.h"
#include "sceneGraphReducer.h"
#include "geomNode.h"
#include "geom.h"
#include "geomVertexRewriter.h"
#include "renderState.h"
#include "transformTable.h"
#include "transformBlendTable.h"
#include "sliderTable.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "vector_int.h"
#include "userVertexTransform.h"
#include "geomMunger.h"
#include "config_pgraph.h"

PStatCollector GeomTransformer::_apply_vertex_collector("*:Flatten:apply:vertex");
PStatCollector GeomTransformer::_apply_texcoord_collector("*:Flatten:apply:texcoord");
PStatCollector GeomTransformer::_apply_set_color_collector("*:Flatten:apply:set color");
PStatCollector GeomTransformer::_apply_scale_color_collector("*:Flatten:apply:scale color");
PStatCollector GeomTransformer::_apply_set_format_collector("*:Flatten:apply:set format");

TypeHandle GeomTransformer::NewCollectedData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomTransformer::
GeomTransformer() :
  // The default value here comes from the Config file.
  _max_collect_vertices(max_collect_vertices)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomTransformer::
GeomTransformer(const GeomTransformer &copy) :
  _max_collect_vertices(copy._max_collect_vertices)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomTransformer::
~GeomTransformer() {
  finish_collect(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_vertices
//       Access: Public
//  Description: Transforms the vertices and the normals in the
//               indicated Geom by the indicated matrix.  Returns true
//               if the Geom was changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_vertices(Geom *geom, const LMatrix4f &mat) {
  PStatTimer timer(_apply_vertex_collector);

  nassertr(geom != (Geom *)NULL, false);
  SourceVertices sv;
  sv._mat = mat;
  sv._vertex_data = geom->get_vertex_data();
  
  PT(GeomVertexData) &new_data = _vertices[sv];
  if (new_data.is_null()) {
    // We have not yet converted these vertices.  Do so now.
    new_data = new GeomVertexData(*sv._vertex_data);
    CPT(GeomVertexFormat) format = new_data->get_format();
    
    int ci;
    for (ci = 0; ci < format->get_num_points(); ci++) {
      GeomVertexRewriter data(new_data, format->get_point(ci));
      
      while (!data.is_at_end()) {
        const LPoint3f &point = data.get_data3f();
        data.set_data3f(point * mat);
      }
    }
    for (ci = 0; ci < format->get_num_vectors(); ci++) {
      GeomVertexRewriter data(new_data, format->get_vector(ci));
      
      while (!data.is_at_end()) {
        const LVector3f &vector = data.get_data3f();
        data.set_data3f(normalize(vector * mat));
      }
    }
  }
  
  geom->set_vertex_data(new_data);

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_vertices
//       Access: Public
//  Description: Transforms the vertices and the normals in all of the
//               Geoms within the indicated GeomNode by the indicated
//               matrix.  Does not destructively change Geoms;
//               instead, a copy will be made of each Geom to be
//               changed, in case multiple GeomNodes reference the
//               same Geom. Returns true if the GeomNode was changed,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_vertices(GeomNode *node, const LMatrix4f &mat) {
  bool any_changed = false;

  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(node->_cycler, current_thread) {
    GeomNode::CDStageWriter cdata(node->_cycler, pipeline_stage, current_thread);
    GeomNode::GeomList::iterator gi;
    GeomNode::GeomList &geoms = *(cdata->modify_geoms());
    for (gi = geoms.begin(); gi != geoms.end(); ++gi) {
      GeomNode::GeomEntry &entry = (*gi);
      PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
      if (transform_vertices(new_geom, mat)) {
        entry._geom = new_geom;
        any_changed = true;
      }
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(node->_cycler);

  if (any_changed) {
    node->mark_internal_bounds_stale();
  }

  return any_changed;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_texcoords
//       Access: Public
//  Description: Transforms the texture coordinates in the indicated
//               Geom by the indicated matrix.  Returns true if the
//               Geom was changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_texcoords(Geom *geom, const InternalName *from_name, 
                    InternalName *to_name, const LMatrix4f &mat) {
  PStatTimer timer(_apply_texcoord_collector);

  nassertr(geom != (Geom *)NULL, false);

  SourceTexCoords st;
  st._mat = mat;
  st._from = from_name;
  st._to = to_name;
  st._vertex_data = geom->get_vertex_data();
  
  PT(GeomVertexData) &new_data = _texcoords[st];
  if (new_data.is_null()) {
    if (!st._vertex_data->has_column(from_name)) {
      // No from_name column; no change.
      return false;
    }
    
    // We have not yet converted these texcoords.  Do so now.
    if (st._vertex_data->has_column(to_name)) {
      new_data = new GeomVertexData(*st._vertex_data);
    } else {
      const GeomVertexColumn *old_column = 
        st._vertex_data->get_format()->get_column(from_name);
      new_data = st._vertex_data->replace_column
        (to_name, old_column->get_num_components(),
         old_column->get_numeric_type(),
         old_column->get_contents());
    }
    
    CPT(GeomVertexFormat) format = new_data->get_format();
    
    GeomVertexWriter tdata(new_data, to_name);
    GeomVertexReader fdata(new_data, from_name);
    
    while (!fdata.is_at_end()) {
      const LPoint4f &coord = fdata.get_data4f();
      tdata.set_data4f(coord * mat);
    }
  }
  
  geom->set_vertex_data(new_data);

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_texcoords
//       Access: Public
//  Description: Transforms the texture coordinates in all of the
//               Geoms within the indicated GeomNode by the indicated
//               matrix.  Does not destructively change Geoms;
//               instead, a copy will be made of each Geom to be
//               changed, in case multiple GeomNodes reference the
//               same Geom. Returns true if the GeomNode was changed,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_texcoords(GeomNode *node, const InternalName *from_name,
                    InternalName *to_name, const LMatrix4f &mat) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  GeomNode::GeomList &geoms = *(cdata->modify_geoms());
  for (gi = geoms.begin(); gi != geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    if (transform_texcoords(new_geom, from_name, to_name, mat)) {
      entry._geom = new_geom;
      any_changed = true;
    }
  }

  return any_changed;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::set_color
//       Access: Public
//  Description: Overrides the color indicated within the Geom with
//               the given replacement color.  Returns true if the
//               Geom was changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
set_color(Geom *geom, const Colorf &color) {
  PStatTimer timer(_apply_set_color_collector);

  SourceColors sc;
  sc._color = color;
  sc._vertex_data = geom->get_vertex_data();
  
  CPT(GeomVertexData) &new_data = _fcolors[sc];
  if (new_data.is_null()) {
    // We have not yet converted these colors.  Do so now.
    if (sc._vertex_data->has_column(InternalName::get_color())) {
      new_data = sc._vertex_data->set_color(color);
    } else {
      new_data = sc._vertex_data->set_color
        (color, 1, Geom::NT_packed_dabc, Geom::C_color);
    }
  }
  
  geom->set_vertex_data(new_data);

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::set_color
//       Access: Public
//  Description: Overrides the color indicated within the GeomNode
//               with the given replacement color.  Returns true if
//               any Geom in the GeomNode was changed, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
set_color(GeomNode *node, const Colorf &color) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  GeomNode::GeomList &geoms = *(cdata->modify_geoms());
  for (gi = geoms.begin(); gi != geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    if (set_color(new_geom, color)) {
      entry._geom = new_geom;
      any_changed = true;
    }
  }

  return any_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_colors
//       Access: Public
//  Description: Transforms the colors in the indicated Geom by the
//               indicated scale.  Returns true if the Geom was
//               changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_colors(Geom *geom, const LVecBase4f &scale) {
  PStatTimer timer(_apply_scale_color_collector);

  nassertr(geom != (Geom *)NULL, false);

  SourceColors sc;
  sc._color = scale;
  sc._vertex_data = geom->get_vertex_data();
  
  CPT(GeomVertexData) &new_data = _tcolors[sc];
  if (new_data.is_null()) {
    // We have not yet converted these colors.  Do so now.
    if (sc._vertex_data->has_column(InternalName::get_color())) {
      new_data = sc._vertex_data->scale_color(scale);
    } else {
      new_data = sc._vertex_data->set_color
        (scale, 1, Geom::NT_packed_dabc, Geom::C_color);
    }
  }
  
  geom->set_vertex_data(new_data);

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_colors
//       Access: Public
//  Description: Transforms the colors in all of the Geoms within the
//               indicated GeomNode by the indicated scale.  Does
//               not destructively change Geoms; instead, a copy will
//               be made of each Geom to be changed, in case multiple
//               GeomNodes reference the same Geom. Returns true if
//               the GeomNode was changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_colors(GeomNode *node, const LVecBase4f &scale) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  GeomNode::GeomList &geoms = *(cdata->modify_geoms());
  for (gi = geoms.begin(); gi != geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    if (transform_colors(new_geom, scale)) {
      entry._geom = new_geom;
      any_changed = true;
    }
  }

  return any_changed;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::apply_state
//       Access: Public
//  Description: Applies the indicated render state to all the of
//               Geoms.  Returns true if the GeomNode was changed,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
apply_state(GeomNode *node, const RenderState *state) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  GeomNode::GeomList &geoms = *(cdata->modify_geoms());
  for (gi = geoms.begin(); gi != geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    CPT(RenderState) new_state = state->compose(entry._state);
    if (entry._state != new_state) {
      entry._state = new_state;
      any_changed = true;
    }
  }

  return any_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::set_format
//       Access: Public
//  Description: Changes the GeomVertexData of the indicated Geom to
//               use the specified format.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
set_format(Geom *geom, const GeomVertexFormat *new_format) {
  PStatTimer timer(_apply_set_format_collector);

  nassertr(geom != (Geom *)NULL, false);

  SourceFormat sf;
  sf._format = new_format;
  sf._vertex_data = geom->get_vertex_data();
  
  PT(GeomVertexData) &new_data = _format[sf];
  if (new_data.is_null()) {
    if (sf._vertex_data->get_format() == new_format) {
      // No change.
      return false;
    }

    // We have not yet converted this vertex data.  Do so now.
    new_data = new GeomVertexData(*sf._vertex_data);
    new_data->set_format(new_format);
  }
  
  geom->set_vertex_data(new_data);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::remove_column
//       Access: Public
//  Description: Removes the named column from the vertex data in the
//               Geom.  Returns true if the Geom was changed, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
remove_column(Geom *geom, const InternalName *column) {
  CPT(GeomVertexFormat) format = geom->get_vertex_data()->get_format();
  if (!format->has_column(column)) {
    return false;
  }

  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*format);
  new_format->remove_column(column);
  new_format->pack_columns();
  format = GeomVertexFormat::register_format(new_format);

  return set_format(geom, format);
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::remove_column
//       Access: Public
//  Description: Removes the named column from the vertex datas within
//               the GeomNode.  Returns true if the GeomNode was
//               changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
remove_column(GeomNode *node, const InternalName *column) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  GeomNode::GeomList &geoms = *(cdata->modify_geoms());
  for (gi = geoms.begin(); gi != geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    if (remove_column(new_geom, column)) {
      entry._geom = new_geom;
      any_changed = true;
    }
  }

  return any_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::make_compatible_state
//       Access: Public
//  Description: Checks if the different geoms in the GeomNode have
//               different RenderStates.  If so, tries to make the 
//               RenderStates the same.  It does this by
//               canonicalizing the ColorAttribs, and in the future,
//               possibly other attribs.
//
//               This implementation is not very smart yet.  It 
//               unnecessarily canonicalizes ColorAttribs even if 
//               this will not yield compatible RenderStates.  A
//               better algorithm would:
//
//               - each geom already starts with an original
//                 RenderState.  In addition to this, calculate for
//                 each geom a canonical RenderState.
//
//               - maintain a table mapping canonical RenderState
//                 to a list of geoms.
//
//               - for each group of geoms with the same 
//                 canonical renderstate, see if they already have
//                 matching RenderStates.
//
//               - If they have differing RenderStates, then
//                 actually canonicalize the geoms.
//               
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
make_compatible_state(GeomNode *node) {
  if (node->get_num_geoms() < 2) {
    return false;
  }
  
  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  GeomNode::GeomList &geoms = *(cdata->modify_geoms());

  // For each geom, calculate a canonicalized RenderState, and 
  // classify all the geoms according to that.
  
  typedef pmap <CPT(RenderState), pvector<int> > StateTable;
  StateTable state_table;
  
  for (int i = 0; i < (int)geoms.size(); i++) {
    GeomNode::GeomEntry &entry = geoms[i];
    CPT(RenderState) canon = entry._state->add_attrib(ColorAttrib::make_vertex(), -1);
    state_table[canon].push_back(i);
  }

  // For each group of geoms, check for mismatch.
  
  bool any_changed = false;
  StateTable::iterator si;
  for (si = state_table.begin(); si != state_table.end(); si++) {
    
    // If the geoms in the group already have the same RenderStates,
    // then nothing needs to be done to this group.
    
    pvector<int> &indices = (*si).second;
    bool mismatch = false;
    for (int i = 1; i < (int)indices.size(); i++) {
      if (geoms[indices[i]]._state != geoms[indices[0]]._state) {
        mismatch = true;
        break;
      }
    }
    if (!mismatch) {
      continue;
    }
    
    // The geoms do not have the same RenderState, but they could,
    // since their canonicalized states are the same.  Canonicalize them.
    
    const RenderState *canon_state = (*si).first;
    for (int i = 0; i < (int)indices.size(); i++) {
      GeomNode::GeomEntry &entry = geoms[indices[i]];
      const RenderAttrib *ra = entry._state->get_attrib(ColorAttrib::get_class_type());
      if (ra == (RenderAttrib *)NULL) {
        ra = ColorAttrib::make_off();
      }
      const ColorAttrib *ca = DCAST(ColorAttrib, ra);
      if (ca->get_color_type() == ColorAttrib::T_vertex) {
        // All we need to do is ensure that the geom has a color column.
        if (!entry._geom.get_read_pointer()->get_vertex_data()->has_column(InternalName::get_color())) {
          PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
          if (set_color(new_geom, Colorf(1,1,1,1))) {
            entry._geom = new_geom;
            any_changed = true;
          }
        }
      } else {
        Colorf c(1,1,1,1);
        if (ca->get_color_type() == ColorAttrib::T_flat) {
          c = ca->get_color();
        }
        PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
        if (set_color(new_geom, c)) {
          entry._geom = new_geom;
          any_changed = true;
        }
      }
      entry._state = canon_state;
    }
  }
  
  return any_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::reverse_normals
//       Access: Public
//  Description: Reverses the lighting normals on the vertex data, if
//               any.  Returns true if the Geom was changed, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
reverse_normals(Geom *geom) {
  nassertr(geom != (Geom *)NULL, false);
  CPT(GeomVertexData) orig_data = geom->get_vertex_data();
  CPT(GeomVertexData) &new_data = _reversed_normals[orig_data];
  if (new_data.is_null()) {
    new_data = orig_data->reverse_normals();
  }

  if (new_data == orig_data) {
    // No change.
    return false;
  }

  geom->set_vertex_data(new_data);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::doubleside
//       Access: Public
//  Description: Duplicates triangles in this GeomNode so that each
//               triangle is back-to-back with another triangle facing
//               in the opposite direction.  If the geometry has
//               vertex normals, this will also duplicate and reverse
//               the normals, so that lighting will work correctly
//               from both sides.  Note that calling this when the
//               geometry is already doublesided (with back-to-back
//               polygons) will result in multiple redundant coplanar
//               polygons.
//
//               Also see CullFaceAttrib, which can enable rendering
//               of both sides of a triangle without having to
//               duplicate it (but which doesn't necessarily work in
//               the presence of lighting).
//
//               Returns true if any Geoms are modified, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
doubleside(GeomNode *node) {
  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; ++i) {
    CPT(Geom) orig_geom = node->get_geom(i);
    bool has_normals = (orig_geom->get_vertex_data()->has_column(InternalName::get_normal()));
    if (has_normals) {
      // If the geometry has normals, we have to duplicate it to
      // reverse the normals on the duplicate copy.
      PT(Geom) new_geom = orig_geom->reverse();
      reverse_normals(new_geom);
      node->add_geom(new_geom, node->get_geom_state(i));
      
    } else {
      // If there are no normals, we can just doubleside it in
      // place.  This is preferable because we can share vertices.
      orig_geom.clear();
      node->modify_geom(i)->doubleside_in_place();
    }
  }
  
  return (num_geoms != 0);
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::reverse
//       Access: Public
//  Description: Reverses the winding order of triangles in this
//               GeomNode so that each triangle is facing in the
//               opposite direction.  If the geometry has vertex
//               normals, this will also reverse the normals, so that
//               lighting will work correctly.
//
//               Also see CullFaceAttrib, which can effectively change
//               the facing of a triangle having to modify its
//               vertices (but which doesn't necessarily work in the
//               presence of lighting).
//
//               Returns true if any Geoms are modified, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
reverse(GeomNode *node) {
  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; ++i) {
    PT(Geom) geom = node->modify_geom(i);
    geom->reverse_in_place();
    reverse_normals(geom);
  }
  
  return (num_geoms != 0);
}
  
////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::collect_vertex_data
//       Access: Public
//  Description: Collects together GeomVertexDatas from different
//               geoms into one big (or several big) GeomVertexDatas.
//               Returns the number of unique GeomVertexDatas created.
//
//               If format_only is true, this only makes
//               GeomVertexFormats compatible; it does not otherwise
//               combine vertices.
//
//               You should follow this up with a call to
//               finish_collect(), but you probably don't want to call
//               this method directly anyway.  Call
//               SceneGraphReducer::collect_vertex_data() instead.
////////////////////////////////////////////////////////////////////
int GeomTransformer::
collect_vertex_data(Geom *geom, int collect_bits, bool format_only) {
  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  if (vdata->get_num_rows() > _max_collect_vertices) {
    // Don't even bother.
    return 0;
  }

  CPT(GeomVertexFormat) format = vdata->get_format();

  NewCollectedKey key;
  if ((collect_bits & SceneGraphReducer::CVD_name) != 0) {
    key._name = vdata->get_name();
  }
  if ((collect_bits & SceneGraphReducer::CVD_format) != 0) {
    key._format = format;
  }
  if ((collect_bits & SceneGraphReducer::CVD_usage_hint) != 0) {
    key._usage_hint = vdata->get_usage_hint();
  } else {
    key._usage_hint = Geom::UH_unspecified;
  }
  if ((collect_bits & SceneGraphReducer::CVD_animation_type) != 0) {
    key._animation_type = format->get_animation().get_animation_type();
  } else {
    key._animation_type = Geom::AT_none;
  }

  AlreadyCollectedMap::const_iterator ai;
  ai = _already_collected_map.find(vdata);
  if (ai != _already_collected_map.end()) {
    // We've previously collected this vertex data; reuse it.
    const AlreadyCollectedData &acd = (*ai).second;
    SourceGeom source_geom;
    source_geom._geom = geom;
    source_geom._vertex_offset = acd._vertex_offset;
    acd._ncd->_source_geoms.push_back(source_geom);
    return 0;
  }

  // We haven't collected this vertex data yet; associate it with a
  // new data.
  NewCollectedMap::iterator ni = _new_collected_map.find(key);
  NewCollectedData *ncd;
  if (ni != _new_collected_map.end()) {
    ncd = (*ni).second;

  } else {
    // We haven't encountered a compatible GeomVertexData before.
    // Create a new entry.
    ncd = new NewCollectedData(vdata);
    _new_collected_list.push_back(ncd);
    _new_collected_map[key] = ncd;
  }

  if (ncd->_new_format != format) {
    ncd->_new_format = format->get_union_format(ncd->_new_format);
  }

  int this_num_vertices = vdata->get_num_rows();

  if (!format_only &&
      ncd->_num_vertices + this_num_vertices > _max_collect_vertices) {
    // Whoa, hold the phone!  Too many vertices going into this one
    // GeomVertexData object; we'd better start over.
    ncd = new NewCollectedData(vdata);
    _new_collected_list.push_back(ncd);
    _new_collected_map[key] = ncd;
  }

  int vertex_offset = ncd->_num_vertices;

  AlreadyCollectedData &acd = _already_collected_map[vdata];
  acd._ncd = ncd;
  acd._vertex_offset = vertex_offset;

  SourceGeom source_geom;
  source_geom._geom = geom;
  source_geom._vertex_offset = vertex_offset;
  ncd->_source_geoms.push_back(source_geom);
  
  SourceData source_data;
  source_data._vdata = vdata;
  source_data._num_vertices = this_num_vertices;

  ncd->_source_datas.push_back(source_data);
  ncd->_num_vertices += this_num_vertices;

  return 0;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::collect_vertex_data
//       Access: Public
//  Description: Collects together individual GeomVertexData
//               structures that share the same format into one big
//               GeomVertexData structure.  This is intended to
//               minimize context switches on the graphics card.
//
//               If format_only is true, this only makes
//               GeomVertexFormats compatible; it does not otherwise
//               combine vertices.
//
//               You should follow this up with a call to
//               finish_collect(), but you probably don't want to call
//               this method directly anyway.  Call
//               SceneGraphReducer::collect_vertex_data() instead.
////////////////////////////////////////////////////////////////////
int GeomTransformer::
collect_vertex_data(GeomNode *node, int collect_bits, bool format_only) {
  int num_adjusted = 0;
  GeomTransformer *dynamic = NULL;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  GeomNode::GeomList &geoms = *(cdata->modify_geoms());
  for (gi = geoms.begin(); gi != geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    entry._geom = new_geom;

    if ((collect_bits & SceneGraphReducer::CVD_avoid_dynamic) != 0 &&
        new_geom->get_vertex_data()->get_usage_hint() < Geom::UH_static) {
      // This one has some dynamic properties.  Collect it
      // independently of the outside world.
      if (dynamic == (GeomTransformer *)NULL) {
        dynamic = new GeomTransformer(*this);
      }
      num_adjusted += dynamic->collect_vertex_data(new_geom, collect_bits, format_only);
      
    } else {
      num_adjusted += collect_vertex_data(new_geom, collect_bits, format_only);
    }
  }

  if (dynamic != (GeomTransformer *)NULL) {
    num_adjusted += dynamic->finish_collect(format_only);
    delete dynamic;
  }

  return num_adjusted;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::finish_collect
//       Access: Public
//  Description: This should be called after a call to
//               collect_vertex_data() to finalize the changes and
//               apply them to the vertices in the graph.  If this is
//               not called, it will be called automatically by the
//               GeomTransformer destructor.
//
//               If format_only is true, this returns the number of
//               GeomVertexDatas modified to use a new format.  If
//               false, it returns the number of GeomVertexDatas
//               created.
////////////////////////////////////////////////////////////////////
int GeomTransformer::
finish_collect(bool format_only) {
  int num_adjusted = 0;

  NewCollectedList::iterator nci;
  for (nci = _new_collected_list.begin(); 
       nci != _new_collected_list.end();
       ++nci) {
    NewCollectedData *ncd = (*nci);
    if (format_only) {
      num_adjusted += ncd->apply_format_only_changes();
    } else {
      num_adjusted += ncd->apply_collect_changes();
    }
    delete ncd;
  }

  _new_collected_list.clear();
  _new_collected_map.clear();
  _already_collected_map.clear();

  return num_adjusted;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::premunge_geom
//       Access: Public
//  Description: Uses the indicated munger to premunge the given Geom
//               to optimize it for eventual rendering.  See
//               SceneGraphReducer::premunge().
////////////////////////////////////////////////////////////////////
PT(Geom) GeomTransformer::
premunge_geom(const Geom *geom, GeomMunger *munger) {
  // This method had been originally provided to cache the result for
  // a particular geom/munger and vdata/munger combination, similar to
  // the way other GeomTransformer methods work.  On reflection, this
  // additional caching is not necessary, since the GeomVertexFormat
  // does its own caching, and there's no danger of that cache filling
  // up during the span of one frame.

  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  vdata = munger->premunge_data(vdata);
  CPT(Geom) pgeom = geom;
  munger->premunge_geom(pgeom, vdata);

  PT(Geom) geom_copy = pgeom->make_copy();
  geom_copy->set_vertex_data(vdata);

  return geom_copy;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::NewCollectedData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTransformer::NewCollectedData::
NewCollectedData(const GeomVertexData *source_data) {
  _new_format = source_data->get_format();
  _vdata_name = source_data->get_name();
  _usage_hint = source_data->get_usage_hint();
  _num_vertices = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::NewCollectedData::apply_format_only_changes
//       Access: Public
//  Description: Actually adjusts the GeomVertexDatas found in a
//               collect_vertex_data() format-only call to have the
//               same vertex format.  Returns the number of vdatas
//               modified.
////////////////////////////////////////////////////////////////////
int GeomTransformer::NewCollectedData::
apply_format_only_changes() {
  int num_modified = 0;

  // We probably don't need to use a map, since
  // GeomVertexData::convert_to() already caches its result, but we do
  // it anyway just in case there's danger of overflowing the cache.
  // What the heck, it's easy to do.
  typedef pmap< CPT(GeomVertexData), CPT(GeomVertexData) > VDataMap;
  VDataMap vdata_map;

  SourceGeoms::iterator sgi;
  for (sgi = _source_geoms.begin(); sgi != _source_geoms.end(); ++sgi) {
    SourceGeom &sg = (*sgi);
    CPT(GeomVertexData) orig_data = sg._geom->get_vertex_data();

    if (orig_data->get_format() != _new_format) {
      VDataMap::iterator mi = vdata_map.find(orig_data);
      if (mi != vdata_map.end()) {
        // Already modified this vdata.
        sg._geom->set_vertex_data((*mi).second);

      } else {
        // Modify this vdata to the new format.
        CPT(GeomVertexData) new_data = orig_data->convert_to(_new_format);
        vdata_map[orig_data] = new_data;
        ++num_modified;

        sg._geom->set_vertex_data(new_data);
      }
    }
  }

  return num_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::NewCollectedData::apply_collect_changes
//       Access: Public
//  Description: Actually combines all of the vertex datas found in a
//               previous call to collect_vertex_data().
////////////////////////////////////////////////////////////////////
int GeomTransformer::NewCollectedData::
apply_collect_changes() {
  if (_num_vertices == 0) {
    return 0;
  }

  _new_data =
    new GeomVertexData(_vdata_name, _new_format, _usage_hint);

  _new_data->unclean_set_num_rows(_num_vertices);

  // Copy each source data into the new GeomVertexData, one at a time.
  int vertex_offset = 0;
  SourceDatas::iterator sdi;
  for (sdi = _source_datas.begin(); sdi != _source_datas.end(); ++sdi) {
    SourceData &sd = (*sdi);
    CPT(GeomVertexData) vdata = sd._vdata;

    if (_new_format != vdata->get_format()) {
      // Convert (non-destructively) the current Geom's vertex
      // data to the new format, so we can just blindly append the
      // vertices to _new_data, within append_vdata().
      vdata = vdata->convert_to(_new_format);
    }

    append_vdata(vdata, vertex_offset);
    vertex_offset += sd._num_vertices;
  }

  nassertr(vertex_offset == _num_vertices, 0);

  if (_new_btable != (TransformBlendTable *)NULL) {
    _new_btable->set_rows(_new_btable_rows);
    _new_data->set_transform_blend_table(_new_btable);
  }

  update_geoms();

  _new_data.clear();
  _new_btable.clear();
  _new_btable_rows.clear();

  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::NewCollectedData::append_vdata
//       Access: Public
//  Description: Appends the vertices from the indicated source
//               GeomVertexData to the end of the working data.
////////////////////////////////////////////////////////////////////
void GeomTransformer::NewCollectedData::
append_vdata(const GeomVertexData *vdata, int vertex_offset) {
  for (int i = 0; i < vdata->get_num_arrays(); ++i) {
    PT(GeomVertexArrayData) new_array = _new_data->modify_array(i);
    CPT(GeomVertexArrayData) old_array = vdata->get_array(i);
    int stride = _new_format->get_array(i)->get_stride();
    int start_byte = vertex_offset * stride;
    int copy_bytes = old_array->get_data_size_bytes();
    nassertv(start_byte + copy_bytes <= new_array->get_data_size_bytes());
    
    new_array->modify_handle()->copy_subdata_from
      (start_byte, copy_bytes, 
       old_array->get_handle(), 0, copy_bytes);
  }

  // Also, copy the animation data (if any).  This means combining
  // transform and/or slider tables, and might therefore mean
  // remapping transform indices in the vertices.  Each of these has a
  // slightly different way to handle the remapping, because they have
  // slightly different kinds of data.
  
  if (vdata->get_transform_table() != (TransformTable *)NULL ||
      _new_data->get_transform_table() != (TransformTable *)NULL) {
    // The TransformTable.
    CPT(TransformTable) old_table;
    if (vdata->get_transform_table() != (TransformTable *)NULL) {
      old_table = vdata->get_transform_table();
    } else {
      PT(TransformTable) temp_table = new TransformTable;
      // There's an implicit identity transform for all nodes.
      PT(VertexTransform) identity_transform = new UserVertexTransform("identity");
      temp_table->add_transform(identity_transform);
      old_table = TransformTable::register_table(temp_table);
    }
    
    // First, build a mapping of the transforms we already have in the
    // current table.  We must do this because the TransformTable
    // doesn't automatically unquify index numbers for us (it doesn't
    // store an index).
    typedef pmap<const VertexTransform *, int> AddedTransforms;
    AddedTransforms added_transforms;
    
    int num_old_transforms = old_table->get_num_transforms();
    for (int i = 0; i < num_old_transforms; i++) {
      added_transforms[old_table->get_transform(i)] = i;
    }
    
    // Now create a new table.  We have to create a new table instead
    // of modifying the existing one, since a registered
    // TransformTable cannot be modified.
    PT(TransformTable) new_table;
    if (_new_data->get_transform_table() != (TransformTable *)NULL) {
      new_table = new TransformTable(*_new_data->get_transform_table());
    } else {
      new_table = new TransformTable;
    }
    
    // Now walk through the old table and copy over its transforms.
    // We will build up an IndexMap of old index numbers to new index
    // numbers while we go, which we can use to modify the vertices.
    IndexMap transform_map;
    
    int num_transforms = old_table->get_num_transforms();
    transform_map.reserve(num_transforms);
    for (int ti = 0; ti < num_transforms; ++ti) {
      const VertexTransform *transform = old_table->get_transform(ti);
      AddedTransforms::iterator ai = added_transforms.find(transform);
      if (ai != added_transforms.end()) {
        // Already got this one in the table.
        transform_map.push_back((*ai).second);
      } else {
        // This is a new one.
        int tj = new_table->add_transform(transform);
        transform_map.push_back(tj);
        added_transforms[transform] = tj;
      }
    }
    _new_data->set_transform_table(TransformTable::register_table(new_table));

    // And now modify the vertices to update the indices to their new
    // values in the new table.  This requires a nested loop, since
    // each column of transform_index might define multiple index
    // values.
    GeomVertexRewriter index(_new_data, InternalName::get_transform_index());
    if (index.has_column()) {
      int num_values = index.get_column()->get_num_values();
      int num_rows = vdata->get_num_rows();
      int new_index[4];
      
      index.set_row(vertex_offset);
      for (int ci = 0; ci < num_rows; ++ci) {
        const int *orig_index = index.get_data4i();
        for (int i = 0; i < num_values; i++) {
          nassertv(orig_index[i] >= 0 && orig_index[i] < (int)transform_map.size());
          new_index[i] = transform_map[orig_index[i]];
        }
        index.set_data4i(new_index);
      }
    }
  }

  if (vdata->get_transform_blend_table() != (TransformBlendTable *)NULL) {
    // The TransformBlendTable.  This one is the easiest, because we
    // can modify it directly, and it will uniquify blend objects for
    // us.

    // We have few special optimizations to handle the
    // TransformBlendTable, since it's a very common case and
    // therefore worth spending a bit of effort to optimize deeply.
    
    CPT(TransformBlendTable) old_btable = vdata->get_transform_blend_table();
    
    if (_new_btable == (TransformBlendTable *)NULL) {
      _new_btable = new TransformBlendTable;
      _new_btable->add_blend(TransformBlend());
    }

    SparseArray new_rows = old_btable->get_rows();
    new_rows <<= vertex_offset;
    _new_btable_rows |= new_rows;

    // We still need to build up the IndexMap.
    IndexMap blend_map;

    int num_blends = old_btable->get_num_blends();
    blend_map.reserve(num_blends);
    for (int bi = 0; bi < num_blends; ++bi) {
      int bj = _new_btable->add_blend(old_btable->get_blend(bi));
      blend_map.push_back(bj);
    }

    // Modify the indices.  This is simpler than the transform_index,
    // above, because each column of transform_blend may only define
    // one index value.
    GeomVertexRewriter index(_new_data, InternalName::get_transform_blend());
    if (index.has_column()) {
      int num_rows = vdata->get_num_rows();
      index.set_row(vertex_offset);

      for (int ci = 0; ci < num_rows; ++ci) {
        int orig_index = index.get_data1i();
        nassertv(orig_index >= 0 && orig_index < (int)blend_map.size());
        int new_index = blend_map[orig_index];
        index.set_data1i(new_index);
      }
    }
  }
  
  if (vdata->get_slider_table() != (SliderTable *)NULL) {
    // The SliderTable.  This one requires making a copy, like the
    // TransformTable (since it can't be modified once registered
    // either), but at least it uniquifies sliders added to it.  Also,
    // it doesn't require indexing into it, so we don't have to build
    // an IndexMap to modify the vertices with.
    const SliderTable *old_sliders = vdata->get_slider_table();
    PT(SliderTable) new_sliders;
    if (_new_data->get_slider_table() != (SliderTable *)NULL) {
      new_sliders = new SliderTable(*_new_data->get_slider_table());
    } else {
      new_sliders = new SliderTable;
    }
    int num_sliders = old_sliders->get_num_sliders();
    for (int si = 0; si < num_sliders; ++si) {
      SparseArray new_rows = old_sliders->get_slider_rows(si);
      new_rows <<= vertex_offset;
      new_sliders->add_slider(old_sliders->get_slider(si), new_rows);
    }
    _new_data->set_slider_table(SliderTable::register_table(new_sliders));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::NewCollectedData::update_geoms
//       Access: Public
//  Description: Updates all of the source Geoms to reference the new
//               vertex data.
////////////////////////////////////////////////////////////////////
void GeomTransformer::NewCollectedData::
update_geoms() {
  SourceGeoms::iterator sgi;
  for (sgi = _source_geoms.begin(); sgi != _source_geoms.end(); ++sgi) {
    SourceGeom &sg = (*sgi);
    sg._geom->offset_vertices(_new_data, sg._vertex_offset);
  }
}
