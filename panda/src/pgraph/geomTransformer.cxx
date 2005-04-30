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
#include "qpgeom.h"
#include "qpgeomVertexRewriter.h"
#include "renderState.h"
#include "transformTable.h"
#include "transformBlendTable.h"
#include "sliderTable.h"
#include "pStatCollector.h"
#include "pStatTimer.h"

static PStatCollector apply_vertex_collector("*:Flatten:apply:vertex");
static PStatCollector apply_texcoord_collector("*:Flatten:apply:texcoord");
static PStatCollector apply_set_color_collector("*:Flatten:apply:set color");
static PStatCollector apply_scale_color_collector("*:Flatten:apply:scale color");

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
  PStatTimer timer(apply_vertex_collector);
  bool transformed = false;

  nassertr(geom != (Geom *)NULL, false);

  if (geom->is_qpgeom()) {
    qpGeom *qpgeom = DCAST(qpGeom, geom);

    qpSourceVertices sv;
    sv._mat = mat;
    sv._vertex_data = qpgeom->get_vertex_data();

    PT(qpGeomVertexData) &new_data = _qpvertices[sv];
    if (new_data.is_null()) {
      // We have not yet converted these vertices.  Do so now.
      new_data = new qpGeomVertexData(*sv._vertex_data);
      CPT(qpGeomVertexFormat) format = new_data->get_format();

      int ci;
      for (ci = 0; ci < format->get_num_points(); ci++) {
        qpGeomVertexRewriter data(new_data, format->get_point(ci));

        while (!data.is_at_end()) {
          const LPoint3f &point = data.get_data3f();
          data.set_data3f(point * mat);
        }
      }
      for (ci = 0; ci < format->get_num_vectors(); ci++) {
        qpGeomVertexRewriter data(new_data, format->get_vector(ci));

        while (!data.is_at_end()) {
          const LVector3f &vector = data.get_data3f();
          data.set_data3f(normalize(vector * mat));
        }
      }
    }

    qpgeom->set_vertex_data(new_data);
    transformed = true;

  } else {
    PTA_Vertexf coords;
    PTA_ushort index;
    
    geom->get_coords(coords, index);
    
    if (!coords.empty()) {
      // Look up the Geom's coords in our table--have we already
      // transformed this array?
      SourceVertices sv;
      sv._mat = mat;
      sv._coords = coords;
      
      PTA_Vertexf &new_coords = _vertices[sv];
      
      if (new_coords.is_null()) {
        // We have not transformed the array yet.  Do so now.
        new_coords.reserve(coords.size());
        PTA_Vertexf::const_iterator vi;
        for (vi = coords.begin(); vi != coords.end(); ++vi) {
          new_coords.push_back((*vi) * mat);
        }
        nassertr(new_coords.size() == coords.size(), false);
      }
      
      geom->set_coords(new_coords, index);
      transformed = true;
    }

    // Now do the same thing for normals.
    PTA_Normalf norms;
    GeomBindType bind;
    
    geom->get_normals(norms, bind, index);
    
    if (bind != G_OFF) {
      SourceNormals sn;
      sn._mat = mat;
      sn._norms = norms;
      
      PTA_Normalf &new_norms = _normals[sn];
      
      if (new_norms.is_null()) {
        // We have not transformed the array yet.  Do so now.
        new_norms.reserve(norms.size());
        PTA_Normalf::const_iterator ni;
        for (ni = norms.begin(); ni != norms.end(); ++ni) {
          Normalf new_norm = (*ni) * mat;
          new_norm.normalize();
          new_norms.push_back(new_norm);
        }
        nassertr(new_norms.size() == norms.size(), false);
      }
      
      geom->set_normals(new_norms, bind, index);
      transformed = true;
    }
  }

  return transformed;
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

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::Geoms::iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom->make_copy();
    if (transform_vertices(new_geom, mat)) {
      entry._geom = new_geom;
      any_changed = true;
    }
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
                    const InternalName *to_name, const LMatrix4f &mat) {
  PStatTimer timer(apply_texcoord_collector);
  bool transformed = false;

  nassertr(geom != (Geom *)NULL, false);
  if (geom->is_qpgeom()) {
    qpGeom *qpgeom = DCAST(qpGeom, geom);

    qpSourceTexCoords st;
    st._mat = mat;
    st._from = from_name;
    st._to = to_name;
    st._vertex_data = qpgeom->get_vertex_data();

    PT(qpGeomVertexData) &new_data = _qptexcoords[st];
    if (new_data.is_null()) {
      if (!st._vertex_data->has_column(from_name)) {
        // No from_name column; no change.
        return false;
      }

      // We have not yet converted these texcoords.  Do so now.
      if (st._vertex_data->has_column(to_name)) {
        new_data = new qpGeomVertexData(*st._vertex_data);
      } else {
        const qpGeomVertexColumn *old_column = 
          st._vertex_data->get_format()->get_column(from_name);
        new_data = st._vertex_data->replace_column
          (to_name, old_column->get_num_components(),
           old_column->get_numeric_type(),
           old_column->get_contents());
      }

      CPT(qpGeomVertexFormat) format = new_data->get_format();
      
      qpGeomVertexWriter tdata(new_data, to_name);
      qpGeomVertexReader fdata(new_data, from_name);
      
      while (!fdata.is_at_end()) {
        const LPoint4f &coord = fdata.get_data4f();
        tdata.set_data4f(coord * mat);
      }
    }

    qpgeom->set_vertex_data(new_data);
    transformed = true;

  } else {
    PTA_TexCoordf texcoords = geom->get_texcoords_array(from_name);
    PTA_ushort index = geom->get_texcoords_index(from_name);

    if (!texcoords.is_null()) {
      // Look up the Geom's texcoords in our table--have we already
      // transformed this array?
      SourceTexCoords stc;
      stc._mat = mat;
      stc._texcoords = texcoords;
      
      PTA_TexCoordf &new_texcoords = _texcoords[stc];
      
      if (new_texcoords.is_null()) {
        // We have not transformed the array yet.  Do so now.
        new_texcoords.reserve(texcoords.size());
        PTA_TexCoordf::const_iterator tci;
        for (tci = texcoords.begin(); tci != texcoords.end(); ++tci) {
          const TexCoordf &tc = (*tci);
          LVecBase4f v4(tc[0], tc[1], 0.0f, 1.0f);
          v4 = v4 * mat;
          new_texcoords.push_back(TexCoordf(v4[0] / v4[3], v4[1] / v4[3]));
        }
        nassertr(new_texcoords.size() == texcoords.size(), false);
      }

      geom->set_texcoords(to_name, new_texcoords, index);
      transformed = true;
    }
  }

  return transformed;
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
                    const InternalName *to_name, const LMatrix4f &mat) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::Geoms::iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom->make_copy();
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
  PStatTimer timer(apply_set_color_collector);
  bool transformed = false;

  if (geom->is_qpgeom()) {
    qpGeom *qpgeom = DCAST(qpGeom, geom);

    qpSourceColors sc;
    sc._color = color;
    sc._vertex_data = qpgeom->get_vertex_data();

    CPT(qpGeomVertexData) &new_data = _qpfcolors[sc];
    if (new_data.is_null()) {
      // We have not yet converted these colors.  Do so now.
      if (sc._vertex_data->has_column(InternalName::get_color())) {
        new_data = sc._vertex_data->set_color(color);
      } else {
        new_data = sc._vertex_data->set_color
          (color, 1, qpGeom::NT_packed_dabc, qpGeom::C_color);
      }
    }

    qpgeom->set_vertex_data(new_data);
    transformed = true;

  } else {
    // In this case, we always replace whatever color array was there
    // with a new color array containing just this color.
    
    // We do want to share this one-element array between Geoms, though.
    PTA_Colorf &new_colors = _fcolors[color];
    
    if (new_colors.is_null()) {
      // We haven't seen this color before; define a new color array.
      new_colors.push_back(color);
    }
    
    geom->set_colors(new_colors, G_OVERALL);
    transformed = true;
  }

  return transformed;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_texcoords
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
  GeomNode::Geoms::iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom->make_copy();
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
  PStatTimer timer(apply_scale_color_collector);
  bool transformed = false;

  nassertr(geom != (Geom *)NULL, false);

  if (geom->is_qpgeom()) {
    qpGeom *qpgeom = DCAST(qpGeom, geom);

    qpSourceColors sc;
    sc._color = scale;
    sc._vertex_data = qpgeom->get_vertex_data();

    CPT(qpGeomVertexData) &new_data = _qptcolors[sc];
    if (new_data.is_null()) {
      // We have not yet converted these colors.  Do so now.
      if (sc._vertex_data->has_column(InternalName::get_color())) {
        new_data = sc._vertex_data->scale_color(scale);
      } else {
        new_data = sc._vertex_data->set_color
          (scale, 1, qpGeom::NT_packed_dabc, qpGeom::C_color);
      }
    }

    qpgeom->set_vertex_data(new_data);
    transformed = true;

  } else {
    PTA_Colorf colors;
    GeomBindType bind;
    PTA_ushort index;
    
    geom->get_colors(colors, bind, index);
    
    if (bind != G_OFF) {
      // Look up the Geom's colors in our table--have we already
      // transformed this array?
      SourceColors sc;
      sc._scale = scale;
      sc._colors = colors;
      
      PTA_Colorf &new_colors = _tcolors[sc];
      
      if (new_colors.is_null()) {
        // We have not transformed the array yet.  Do so now.
        new_colors.reserve(colors.size());
        PTA_Colorf::const_iterator ci;
        for (ci = colors.begin(); ci != colors.end(); ++ci) {
          const Colorf &c = (*ci);
          Colorf transformed(c[0] * scale[0], 
                             c[1] * scale[1], 
                             c[2] * scale[2],
                             c[3] * scale[3]);
          new_colors.push_back(transformed);
        }
        nassertr(new_colors.size() == colors.size(), false);
      }
      
      geom->set_colors(new_colors, bind, index);
      transformed = true;
    }
  }

  return transformed;
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
  GeomNode::Geoms::iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom->make_copy();
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
  GeomNode::Geoms::iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
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
//     Function: GeomTransformer::collect_vertex_data
//       Access: Public
//  Description: Collects together GeomVertexDatas from different
//               geoms into one big (or several big) GeomVertexDatas.
//               Returns the number of unique GeomVertexDatas created.
////////////////////////////////////////////////////////////////////
int GeomTransformer::
collect_vertex_data(qpGeom *geom, int collect_bits) {
  const qpGeomVertexData *vdata = geom->get_vertex_data();

  if (vdata->get_num_rows() > _max_collect_vertices) {
    // Don't even bother.
    return 0;
  }

  const qpGeomVertexFormat *format = vdata->get_format();

  NewCollectedKey key;
  if ((collect_bits & SceneGraphReducer::CVD_name) != 0) {
    key._name = vdata->get_name();
  }
  key._format = format;
  key._usage_hint = vdata->get_usage_hint();

  AlreadyCollected::const_iterator ai;
  ai = _already_collected.find(vdata);
  if (ai != _already_collected.end()) {
    // We've previously collected this vertex data; reuse it.
    const AlreadyCollectedData &acd = (*ai).second;
    geom->offset_vertices(acd._data, acd._offset);
    return 0;
  }

  // We haven't collected this vertex data yet; append the vertices
  // onto the new data.
  int num_created = 0;

  NewCollectedData::iterator ni = _new_collected_data.find(key);
  PT(qpGeomVertexData) new_data;
  if (ni != _new_collected_data.end()) {
    new_data = (*ni).second;
  } else {
    new_data = new qpGeomVertexData(vdata->get_name(), format, 
                                    vdata->get_usage_hint());
    _new_collected_data[key] = new_data;
    ++num_created;
  }

  int offset = new_data->get_num_rows();
  int new_num_vertices = offset + vdata->get_num_rows();
  if (new_num_vertices > _max_collect_vertices) {
    // Whoa, hold the phone!  Too many vertices going into this one
    // GeomVertexData object; we'd better start over.
    new_data = new qpGeomVertexData(vdata->get_name(), format, 
                                    vdata->get_usage_hint());
    _new_collected_data[key] = new_data;
    offset = 0;
    new_num_vertices = vdata->get_num_rows();
    ++num_created;
  }

  new_data->set_num_rows(new_num_vertices);

  for (int i = 0; i < vdata->get_num_arrays(); ++i) {
    qpGeomVertexArrayData *new_array = new_data->modify_array(i);
    const qpGeomVertexArrayData *old_array = vdata->get_array(i);
    int stride = format->get_array(i)->get_stride();
    int start_byte = offset * stride;
    int copy_bytes = old_array->get_data_size_bytes();
    nassertr(start_byte + copy_bytes == new_array->get_data_size_bytes(), 0);

    memcpy(new_array->modify_data() + start_byte,
           old_array->get_data(), copy_bytes);
  }

  // Also, copy the animation data (if any).  This means combining
  // transform and/or slider tables, and might therefore mean
  // remapping transform indices in the vertices.  Each of these has a
  // slightly different way to handle the remapping, because they have
  // slightly different kinds of data.
  typedef pvector<int> IndexMap;

  if (vdata->get_transform_table() != (TransformTable *)NULL) {
    // The TransformTable.
    const TransformTable *old_table = vdata->get_transform_table();
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
    if (new_data->get_transform_table() != (TransformTable *)NULL) {
      new_table = new TransformTable(*new_data->get_transform_table());
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
    new_data->set_transform_table(TransformTable::register_table(new_table));

    // And now modify the vertices to update the indices to their new
    // values in the new table.  This requires a nested loop, since
    // each column of transform_index might define multiple index
    // values.
    qpGeomVertexRewriter index(new_data, InternalName::get_transform_index());
    if (index.has_column()) {
      int num_values = index.get_column()->get_num_values();
      int new_index[4];

      index.set_row(offset);
      while (!index.is_at_end()) {
        const int *orig_index = index.get_data4i();
        for (int i = 0; i < num_values; i++) {
          nassertr(orig_index[i] >= 0 && orig_index[i] < (int)transform_map.size(), 0);
          new_index[i] = transform_map[orig_index[i]];
        }
        index.set_data4i(new_index);
      }
    }
  }

  if (vdata->get_transform_blend_table() != (TransformBlendTable *)NULL)  {
    // The TransformBlendTable.  This one is the easiest, because we
    // can modify it directly, and it will uniquify blend objects for
    // us.
    const TransformBlendTable *old_btable = vdata->get_transform_blend_table();
    PT(TransformBlendTable) new_btable;
    if (new_data->get_transform_blend_table() != (TransformBlendTable *)NULL) {
      new_btable = new_data->modify_transform_blend_table();
    } else {
      new_btable = new TransformBlendTable;
      new_data->set_transform_blend_table(new_btable);
    }

    // We still need to build up the IndexMap.
    IndexMap blend_map;

    int num_blends = old_btable->get_num_blends();
    blend_map.reserve(num_blends);
    for (int bi = 0; bi < num_blends; ++bi) {
      int bj = new_btable->add_blend(old_btable->get_blend(bi));
      blend_map.push_back(bj);
    }

    // Modify the indices.  This is simpler than the transform_index,
    // above, because each column of transform_blend may only define
    // one index value.
    qpGeomVertexRewriter index(new_data, InternalName::get_transform_blend());
    if (index.has_column()) {
      index.set_row(offset);
      while (!index.is_at_end()) {
        int orig_index = index.get_data1i();
        nassertr(orig_index >= 0 && orig_index < (int)blend_map.size(), 0);
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
    if (new_data->get_slider_table() != (SliderTable *)NULL) {
      new_sliders = new SliderTable(*new_data->get_slider_table());
    } else {
      new_sliders = new SliderTable;
    }
    int num_sliders = old_sliders->get_num_sliders();
    for (int si = 0; si < num_sliders; ++si) {
      new_sliders->add_slider(old_sliders->get_slider(si));
    }
    new_data->set_slider_table(SliderTable::register_table(new_sliders));
  }


  AlreadyCollectedData &acd = _already_collected[vdata];
  acd._data = new_data;
  acd._offset = offset;
  geom->offset_vertices(new_data, offset);

  return num_created;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::collect_vertex_data
//       Access: Public
//  Description: Collects together individual GeomVertexData
//               structures that share the same format into one big
//               GeomVertexData structure.  This is intended to
//               minimize context switches on the graphics card.
////////////////////////////////////////////////////////////////////
int GeomTransformer::
collect_vertex_data(GeomNode *node, int collect_bits) {
  int num_created = 0;

  GeomTransformer *dynamic = NULL;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::Geoms::iterator gi;
  for (gi = cdata->_geoms.begin(); gi != cdata->_geoms.end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    if (entry._geom->is_qpgeom()) {
      PT(qpGeom) new_geom = DCAST(qpGeom, entry._geom->make_copy());
      entry._geom = new_geom;

      if ((collect_bits & SceneGraphReducer::CVD_avoid_dynamic) != 0 &&
          new_geom->get_vertex_data()->get_usage_hint() < qpGeom::UH_static) {
        // This one has some dynamic properties.  Collect it
        // independently of the outside world.
        if (dynamic == (GeomTransformer *)NULL) {
          dynamic = new GeomTransformer(*this);
        }
        num_created += dynamic->collect_vertex_data(new_geom, collect_bits);
        
      } else {
        num_created += collect_vertex_data(new_geom, collect_bits);
      }
    }
  }

  if (dynamic != (GeomTransformer *)NULL) {
    delete dynamic;
  }

  return num_created;
}
