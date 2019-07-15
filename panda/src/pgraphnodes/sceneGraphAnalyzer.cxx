/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneGraphAnalyzer.cxx
 * @author drose
 * @date 2000-07-02
 */

#include "sceneGraphAnalyzer.h"
#include "config_pgraph.h"

#include "indent.h"
#include "lodNode.h"
#include "geomNode.h"
#include "geomVertexData.h"
#include "geom.h"
#include "geomPrimitive.h"
#include "geomPoints.h"
#include "geomLines.h"
#include "geomLinestrips.h"
#include "geomTriangles.h"
#include "geomTristrips.h"
#include "geomTrifans.h"
#include "geomPatches.h"
#include "transformState.h"
#include "textureAttrib.h"
#include "pta_ushort.h"
#include "geomVertexReader.h"

/**
 *
 */
SceneGraphAnalyzer::
SceneGraphAnalyzer() {
  _lod_mode = LM_all;
  clear();
}

/**
 *
 */
SceneGraphAnalyzer::
~SceneGraphAnalyzer() {
}

/**
 * Resets all of the data in the analyzer in preparation for a new run.
 */
void SceneGraphAnalyzer::
clear() {
  _nodes.clear();
  _vdatas.clear();
  _vformats.clear();
  _vadatas.clear();
  _unique_vdatas.clear();
  _unique_vadatas.clear();
  _textures.clear();

  _num_nodes = 0;
  _num_instances = 0;
  _num_transforms = 0;
  _num_nodes_with_attribs = 0;
  _num_lod_nodes = 0;
  _num_geom_nodes = 0;
  _num_geoms = 0;
  _num_geom_vertex_datas = 0;
  _num_geom_vertex_formats = 0;
  _vertex_data_size = 0;
  _prim_data_size = 0;

  _num_vertices = 0;
  _num_vertices_64 = 0;
  _num_normals = 0;
  _num_colors = 0;
  _num_texcoords = 0;
  _num_tris = 0;
  _num_lines = 0;
  _num_points = 0;
  _num_patches = 0;

  _num_individual_tris = 0;
  _num_tristrips = 0;
  _num_triangles_in_strips = 0;
  _num_trifans = 0;
  _num_triangles_in_fans = 0;
  _num_vertices_in_patches = 0;

  _texture_bytes = 0;

  _num_long_normals = 0;
  _num_short_normals = 0;
  _total_normal_length = 0.0f;
}

/**
 * Adds a new node to the set of data for analysis.  Normally, this would only
 * be called once, and passed the top of the scene graph, but it's possible to
 * repeatedly pass in subgraphs to get an analysis of all the graphs together.
 */
void SceneGraphAnalyzer::
add_node(PandaNode *node) {
  collect_statistics(node, false);
}

/**
 * Describes all the data collected.
 */
void SceneGraphAnalyzer::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << _num_nodes << " total nodes (including "
    << _num_instances << " instances); " << _num_lod_nodes << " LODNodes.\n";

  indent(out, indent_level)
    << _num_transforms << " transforms";

  if (_num_nodes != 0) {
    out << "; " << 100 * _num_nodes_with_attribs / _num_nodes
        << "% of nodes have some render attribute.";
  }
  out << "\n";

  indent(out, indent_level)
    << _num_geoms << " Geoms, with " << _num_geom_vertex_datas
    << " GeomVertexDatas and " << _num_geom_vertex_formats
    << " GeomVertexFormats, appear on " << _num_geom_nodes
    << " GeomNodes.\n";

  indent(out, indent_level);
  if (_num_vertices_64 != 0) {
    out << _num_vertices_64 << " 64-bit vertices, ";
    if (_num_vertices != _num_vertices_64) {
      out << _num_vertices - _num_vertices_64 << " 32-bit vertices, ";
    }
  } else {
    out << _num_vertices << " vertices, ";
  }

  out << _num_normals << " normals, "
      << _num_colors << " colors, "
      << _num_texcoords << " texture coordinates.\n";

  if (_num_long_normals != 0 || _num_short_normals != 0) {
    indent(out, indent_level)
      << _num_long_normals << " normals are too long, "
      << _num_short_normals << " are too short.  Average normal length is "
      << _total_normal_length / (PN_stdfloat)_num_normals << "\n";
  }

  indent(out, indent_level)
    << "GeomVertexData arrays occupy " << (_vertex_data_size + 1023) / 1024
    << "K memory.\n";

  indent(out, indent_level)
    << "GeomPrimitive arrays occupy " << (_prim_data_size + 1023) / 1024
    << "K memory.\n";

  int unreferenced_vertices = 0;
  VDatas::const_iterator vdi;
  for (vdi = _vdatas.begin(); vdi != _vdatas.end(); ++vdi) {
    CPT(GeomVertexData) vdata = (*vdi).first;
    const VDataTracker &tracker = (*vdi).second;
    int num_unreferenced = vdata->get_num_rows() - tracker._referenced_vertices.get_num_on_bits();
    nassertv(num_unreferenced >= 0);
    unreferenced_vertices += num_unreferenced;
  }
  if (unreferenced_vertices != 0) {
    indent(out, indent_level)
      << unreferenced_vertices << " vertices are unreferenced by any GeomPrimitives.\n";
  }
  if (_unique_vdatas.size() != _vdatas.size()) {
    indent(out, indent_level)
      << _vdatas.size() - _unique_vdatas.size()
      << " GeomVertexDatas are redundantly duplicated\n";
  }
  if (_unique_vadatas.size() != _vadatas.size()) {
    int wasted_bytes = 0;

    UniqueVADatas::const_iterator uvai;
    for (uvai = _unique_vadatas.begin();
         uvai != _unique_vadatas.end();
         ++uvai) {
      const GeomVertexArrayData *gvad = (*uvai).first;
      int dup_count = (*uvai).second;
      if (dup_count > 1) {
        wasted_bytes += (dup_count - 1) * gvad->get_data_size_bytes();
      }
    }
    indent(out, indent_level)
      << _vadatas.size() - _unique_vadatas.size()
      << " GeomVertexArrayDatas are redundant, wasting "
      << (wasted_bytes + 1023) / 1024 << "K.\n";
  }
  if (_unique_prim_vadatas.size() != _prim_vadatas.size()) {
    int wasted_bytes = 0;

    UniqueVADatas::const_iterator uvai;
    for (uvai = _unique_prim_vadatas.begin();
         uvai != _unique_prim_vadatas.end();
         ++uvai) {
      const GeomVertexArrayData *gvad = (*uvai).first;
      int dup_count = (*uvai).second;
      if (dup_count > 1) {
        wasted_bytes += (dup_count - 1) * gvad->get_data_size_bytes();
      }
    }
    indent(out, indent_level)
      << _prim_vadatas.size() - _unique_prim_vadatas.size()
      << " GeomPrimitive arrays are redundant, wasting "
      << (wasted_bytes + 1023) / 1024 << "K.\n";
  }

  indent(out, indent_level)
    << _num_tris << " triangles:\n";
  indent(out, indent_level + 2)
    << _num_triangles_in_strips
    << " of these are on " << _num_tristrips << " tristrips";
  if (_num_tristrips != 0) {
    out << " ("
        << (double)_num_triangles_in_strips / (double)_num_tristrips
        << " average tris per strip)";
  }
  out << ".\n";

  if (_num_trifans != 0) {
    indent(out, indent_level + 2)
      << _num_triangles_in_fans
      << " of these are on " << _num_trifans << " trifans";
    if (_num_trifans != 0) {
      out << " ("
          << (double)_num_triangles_in_fans / (double)_num_trifans
          << " average tris per fan)";
    }
    out << ".\n";
  }

  indent(out, indent_level + 2)
    << _num_individual_tris
    << " of these are independent triangles.\n";

  if (_num_patches != 0) {
    indent(out, indent_level)
      << _num_patches << " patches ("
      << (double)_num_vertices_in_patches / (double)_num_patches
      << " average verts per patch).\n";
  }

  if (_num_lines != 0 || _num_points != 0) {
    indent(out, indent_level)
      << _num_lines << " lines, " << _num_points << " points.\n";
  }

  indent(out, indent_level)
    << _textures.size() << " textures, estimated minimum "
    << (_texture_bytes + 1023) / 1024 << "K texture memory required.\n";
}

/**
 * Recursively visits each node, counting up the statistics.
 */
void SceneGraphAnalyzer::
collect_statistics(PandaNode *node, bool under_instance) {
  _num_nodes++;

  if (!under_instance) {
    Nodes::iterator ni = _nodes.find(node);
    if (ni == _nodes.end()) {
      // This is the first time this node has been encountered.
      _nodes.insert(Nodes::value_type(node, 1));
    } else {
      // This node has been encountered before; that makes it an instance.
      (*ni).second++;
      _num_instances++;
      under_instance = true;
    }
  }

  if (!node->get_state()->is_empty()) {
    _num_nodes_with_attribs++;
    const RenderAttrib *attrib =
      node->get_attrib(TextureAttrib::get_class_slot());
    if (attrib != nullptr) {
      const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
      for (int i = 0; i < ta->get_num_on_stages(); i++) {
        collect_statistics(ta->get_on_texture(ta->get_on_stage(i)));
      }
    }
  }
  if (!node->get_transform()->is_identity()) {
    _num_transforms++;
  }

  if (node->is_geom_node()) {
    collect_statistics(DCAST(GeomNode, node));
  }

  if (node->is_lod_node()) {
    LODNode *lod_node = DCAST(LODNode, node);
    ++_num_lod_nodes;

    switch (_lod_mode) {
    case LM_lowest:
    case LM_highest:
      {
        int sw = (_lod_mode == LM_lowest) ? lod_node->get_lowest_switch() : lod_node->get_highest_switch();
        if (sw >= 0 && sw < node->get_num_children()) {
          PandaNode *child = node->get_child(sw);
          collect_statistics(child, under_instance);
        }
        return;
      }

    case LM_none:
      return;

    case LM_all:
      // fall through to the loop below.
      break;
    }
  }

  int num_children = node->get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);
    collect_statistics(child, under_instance);
  }
}

/**
 * Recursively visits each node, counting up the statistics.
 */
void SceneGraphAnalyzer::
collect_statistics(GeomNode *geom_node) {
  nassertv(geom_node != nullptr);

  ++_num_geom_nodes;

  int num_geoms = geom_node->get_num_geoms();
  _num_geoms += num_geoms;

  for (int i = 0; i < num_geoms; i++) {
    const Geom *geom = geom_node->get_geom(i);
    collect_statistics(geom);

    const RenderState *geom_state = geom_node->get_geom_state(i);

    const RenderAttrib *attrib =
      geom_state->get_attrib(TextureAttrib::get_class_slot());
    if (attrib != nullptr) {
      const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
      for (int i = 0; i < ta->get_num_on_stages(); i++) {
        collect_statistics(ta->get_on_texture(ta->get_on_stage(i)));
      }
    }
  }
}

/**
 * Recursively visits each node, counting up the statistics.
 */
void SceneGraphAnalyzer::
collect_statistics(const Geom *geom) {
  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  std::pair<VDatas::iterator, bool> result = _vdatas.insert(VDatas::value_type(vdata, VDataTracker()));
  if (result.second) {
    // This is the first time we've encountered this vertex data.
    ++_num_geom_vertex_datas;

    CPT(GeomVertexFormat) vformat = vdata->get_format();
    bool format_inserted = _vformats.insert(vformat).second;
    if (format_inserted) {
      // This is the first time we've encountered this vertex format.
      ++_num_geom_vertex_formats;
    }

    int &dup_count = (*(_unique_vdatas.insert(UniqueVDatas::value_type(vdata, 0)).first)).second;
    ++dup_count;

    int num_rows = vdata->get_num_rows();
    const GeomVertexFormat *format = vdata->get_format();
    if (format->has_column(InternalName::get_vertex())) {
      _num_vertices += num_rows;
      const GeomVertexColumn *vcolumn = format->get_column(InternalName::get_vertex());
      if (vcolumn->get_numeric_type() == GeomEnums::NT_float64) {
        _num_vertices_64 += num_rows;
      }
    }
    if (format->has_column(InternalName::get_normal())) {
      _num_normals += num_rows;
      GeomVertexReader rnormal(vdata, InternalName::get_normal());
      while (!rnormal.is_at_end()) {
        LVector3f normal = rnormal.get_data3f();
        float length = normal.length();
        if (IS_NEARLY_EQUAL(length, 1.0f)) {
          // Correct length normal.
        } else if (length > 1.0f) {
          ++_num_long_normals;
        } else {  // length < 1.0f
          ++_num_short_normals;
        }
        _total_normal_length += length;
      }
    }
    if (format->has_column(InternalName::get_color())) {
      _num_colors += num_rows;
    }
    int num_texcoords = format->get_num_texcoords();
    _num_texcoords += num_rows * num_texcoords;

    int num_arrays = vdata->get_num_arrays();
    for (int i = 0; i < num_arrays; ++i) {
      collect_statistics(vdata->get_array(i));
    }
  }
  VDataTracker &tracker = (*(result.first)).second;

  // Now consider the primitives in the Geom.
  int num_primitives = geom->get_num_primitives();
  for (int i = 0; i < num_primitives; ++i) {
    CPT(GeomPrimitive) prim = geom->get_primitive(i);

    int num_vertices = prim->get_num_vertices();
    int strip_cut_index = prim->get_strip_cut_index();
    for (int vi = 0; vi < num_vertices; ++vi) {
      int index = prim->get_vertex(vi);
      if (index != strip_cut_index) {
        tracker._referenced_vertices.set_bit(index);
      }
    }

    if (prim->is_indexed()) {
      collect_prim_statistics(prim->get_vertices());
      if (prim->is_composite()) {
        collect_statistics(prim->get_mins());
        collect_statistics(prim->get_maxs());
      }
    }

    if (prim->is_of_type(GeomPoints::get_class_type())) {
      _num_points += prim->get_num_primitives();

    } else if (prim->is_of_type(GeomLines::get_class_type())) {
      _num_lines += prim->get_num_primitives();

    } else if (prim->is_of_type(GeomLinestrips::get_class_type())) {
      _num_lines += prim->get_num_faces();

    } else if (prim->is_of_type(GeomTriangles::get_class_type())) {
      _num_tris += prim->get_num_primitives();
      _num_individual_tris += prim->get_num_primitives();

    } else if (prim->is_of_type(GeomTristrips::get_class_type())) {
      _num_tris += prim->get_num_faces();
      _num_tristrips += prim->get_num_primitives();
      _num_triangles_in_strips += prim->get_num_faces();

    } else if (prim->is_of_type(GeomTrifans::get_class_type())) {
      _num_tris += prim->get_num_faces();
      _num_trifans += prim->get_num_primitives();
      _num_triangles_in_fans += prim->get_num_faces();

    } else if (prim->is_of_type(GeomPatches::get_class_type())) {
      _num_patches += prim->get_num_primitives();
      _num_vertices_in_patches += prim->get_num_vertices();

    } else {
      pgraph_cat.warning()
        << "Unknown GeomPrimitive type in SceneGraphAnalyzer: "
        << prim->get_type() << "\n";
    }
  }
}

/**
 * Recursively visits each node, counting up the statistics.
 */
void SceneGraphAnalyzer::
collect_statistics(Texture *texture) {
  nassertv(texture != nullptr);

  Textures::iterator ti = _textures.find(texture);
  if (ti == _textures.end()) {
    // This is the first time this texture has been encountered.
    _textures.insert(Textures::value_type(texture, 1));

    // Attempt to guess how many bytes of texture memory this one requires.
    size_t bytes =
      texture->get_x_size() * texture->get_y_size() *
      texture->get_num_components() * texture->get_component_width();

    if (texture->uses_mipmaps()) {
      bytes *= 4/3;
    }

    _texture_bytes += bytes;

  } else {
    // This texture has been encountered before; don't count it again.
    (*ti).second++;
  }
}

/**
 * Recursively visits each node, counting up the statistics.
 */
void SceneGraphAnalyzer::
collect_statistics(const GeomVertexArrayData *vadata) {
  nassertv(vadata != nullptr);
  bool inserted = _vadatas.insert(vadata).second;
  if (inserted) {
    // This is the first time we've encountered this vertex array.
    _vertex_data_size += vadata->get_data_size_bytes();
    int &dup_count = (*(_unique_vadatas.insert(UniqueVADatas::value_type(vadata, 0)).first)).second;
    ++dup_count;
  }
}

/**
 * Recursively visits each node, counting up the statistics.  This one records
 * the vertex index array associated with a GeomPrimitive, as opposed to the
 * vertex data array, component of a GeomVertexData.
 */
void SceneGraphAnalyzer::
collect_prim_statistics(const GeomVertexArrayData *vadata) {
  nassertv(vadata != nullptr);
  bool inserted = _prim_vadatas.insert(vadata).second;
  if (inserted) {
    // This is the first time we've encountered this vertex array.
    _prim_data_size += vadata->get_data_size_bytes();
    int &dup_count = (*(_unique_prim_vadatas.insert(UniqueVADatas::value_type(vadata, 0)).first)).second;
    ++dup_count;
  }
}
