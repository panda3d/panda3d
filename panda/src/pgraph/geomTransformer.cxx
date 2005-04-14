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

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomTransformer::
GeomTransformer() :
  _usage_hint(qpGeom::UH_static),
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
  _usage_hint(copy._usage_hint),
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
  bool transformed = false;

  nassertr(geom != (Geom *)NULL, false);

  if (geom->is_of_type(qpGeom::get_class_type())) {
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
  bool transformed = false;

  nassertr(geom != (Geom *)NULL, false);
  if (geom->is_of_type(qpGeom::get_class_type())) {
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
           old_column->get_contents(), 
           min(_usage_hint, st._vertex_data->get_usage_hint()),
           false);
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
  bool transformed = false;

  if (geom->is_of_type(qpGeom::get_class_type())) {
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
  bool transformed = false;

  nassertr(geom != (Geom *)NULL, false);

  if (geom->is_of_type(qpGeom::get_class_type())) {
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

  if (vdata->get_num_vertices() > _max_collect_vertices) {
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

  int offset = new_data->get_num_vertices();
  int new_num_vertices = offset + vdata->get_num_vertices();
  if (new_num_vertices > _max_collect_vertices) {
    // Whoa, hold the phone!  Too many vertices going into this one
    // GeomVertexData object; we'd better start over.
    new_data = new qpGeomVertexData(vdata->get_name(), format, 
                                    vdata->get_usage_hint());
    _new_collected_data[key] = new_data;
    offset = 0;
    new_num_vertices = vdata->get_num_vertices();
    ++num_created;
  }

  new_data->set_num_vertices(new_num_vertices);

  for (int i = 0; i < vdata->get_num_arrays(); ++i) {
    qpGeomVertexArrayData *new_array = new_data->modify_array(i);
    const qpGeomVertexArrayData *old_array = vdata->get_array(i);
    int stride = format->get_array(i)->get_stride();
    int start_byte = offset * stride;
    int copy_bytes = old_array->get_data_size_bytes();
    nassertr(start_byte + copy_bytes == new_array->get_data_size_bytes(), false);

    memcpy(new_array->modify_data() + start_byte,
           old_array->get_data(), copy_bytes);
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
//               GeomVertexData structure.  This is designed to
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
    if (entry._geom->is_of_type(qpGeom::get_class_type())) {
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
