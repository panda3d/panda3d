// Filename: sceneGraphAnalyzer.cxx
// Created by:  drose (02Jul00)
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

#include "sceneGraphAnalyzer.h"
#include "config_pgraph.h"

#include "indent.h"
#include "geomNode.h"
#include "geom.h"
#include "geomprimitives.h"
#include "transformState.h"
#include "textureAttrib.h"
#include "pta_ushort.h"

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SceneGraphAnalyzer::
SceneGraphAnalyzer() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SceneGraphAnalyzer::
~SceneGraphAnalyzer() {
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::clear
//       Access: Public
//  Description: Resets all of the data in the analyzer in preparation
//               for a new run.
////////////////////////////////////////////////////////////////////
void SceneGraphAnalyzer::
clear() {
  _nodes.clear();
  _textures.clear();

  _num_nodes = 0;
  _num_instances = 0;
  _num_transforms = 0;
  _num_nodes_with_attribs = 0;
  _num_geom_nodes = 0;
  _num_geoms = 0;

  _num_vertices = 0;
  _num_normals = 0;
  _num_texcoords = 0;
  _num_tris = 0;
  _num_quads = 0;
  _num_polys = 0;
  _num_lines = 0;
  _num_points = 0;
  _num_spheres = 0;

  _num_individual_tris = 0;
  _num_tristrips = 0;
  _num_triangles_in_strips = 0;
  _num_trifans = 0;
  _num_triangles_in_fans = 0;

  _texture_bytes = 0;

  _num_long_normals = 0;
  _num_short_normals = 0;
  _total_normal_length = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::add_node
//       Access: Public
//  Description: Adds a new node to the set of data for analysis.
//               Normally, this would only be called once, and passed
//               the top of the scene graph, but it's possible to
//               repeatedly pass in subgraphs to get an analysis of
//               all the graphs together.
////////////////////////////////////////////////////////////////////
void SceneGraphAnalyzer::
add_node(PandaNode *node) {
  collect_statistics(node, false);
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::write
//       Access: Public
//  Description: Describes all the data collected.
////////////////////////////////////////////////////////////////////
void SceneGraphAnalyzer::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << _num_nodes << " total nodes (including "
    << _num_instances << " instances).\n";

  indent(out, indent_level)
    << _num_transforms << " transforms";

  if (_num_nodes != 0) {
    out << "; " << 100 * _num_nodes_with_attribs / _num_nodes
        << "% of nodes have some render attribute.";
  }
  out << "\n";

  indent(out, indent_level)
    << _num_geoms << " Geoms appear on " << _num_geom_nodes << " GeomNodes.\n";

  indent(out, indent_level)
    << _num_vertices << " vertices, " << _num_normals << " normals, "
    << _num_texcoords << " texture coordinates.\n";

  if (_num_long_normals != 0 || _num_short_normals != 0) {
    indent(out, indent_level)
      << _num_long_normals << " normals are too long, "
      << _num_short_normals << " are too short.  Average normal length is "
      << _total_normal_length / (float)_num_normals << "\n";
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

  indent(out, indent_level + 2)
    << _num_triangles_in_fans
    << " of these are on " << _num_trifans << " trifans";
  if (_num_trifans != 0) {
    out << " ("
        << (double)_num_triangles_in_fans / (double)_num_trifans
        << " average tris per fan)";
  }
  out << ".\n";

  indent(out, indent_level + 2)
    << _num_individual_tris
    << " of these are independent triangles.\n";

  indent(out, indent_level)
    << _num_quads << " quads, " << _num_polys << " general polygons, "
    << _num_lines << " lines, " << _num_points << " points, "
    << _num_spheres << " spheres.\n";

  indent(out, indent_level)
    << _textures.size() << " textures, estimated minimum "
    << (_texture_bytes + 1023) / 1024 << "K texture memory required.\n";
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::collect_statistics
//       Access: Private
//  Description: Recursively visits each node, counting up the
//               statistics.
////////////////////////////////////////////////////////////////////
void SceneGraphAnalyzer::
collect_statistics(PandaNode *node, bool under_instance) {
  _num_nodes++;

  if (!under_instance) {
    Nodes::iterator ni = _nodes.find(node);
    if (ni == _nodes.end()) {
      // This is the first time this node has been encountered.
      _nodes.insert(Nodes::value_type(node, 1));
    } else {
      // This node has been encountered before; that makes it an
      // instance.
      (*ni).second++;
      _num_instances++;
      under_instance = true;
    }
  }

  if (!node->get_state()->is_empty()) {
    _num_nodes_with_attribs++;
    const RenderAttrib *attrib = 
      node->get_attrib(TextureAttrib::get_class_type());
    if (attrib != (RenderAttrib *)NULL) {
      const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
      if (!ta->is_off()) {
        collect_statistics(ta->get_texture());
      }
    }      
  }
  if (!node->get_transform()->is_identity()) {
    _num_transforms++;
  }

  if (node->is_geom_node()) {
    collect_statistics(DCAST(GeomNode, node));
  }

  int num_children = node->get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);
    collect_statistics(child, under_instance);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::collect_statistics
//       Access: Private
//  Description: Recursively visits each node, counting up the
//               statistics.
////////////////////////////////////////////////////////////////////
void SceneGraphAnalyzer::
collect_statistics(GeomNode *geom_node) {
  nassertv(geom_node != (GeomNode *)NULL);

  _num_geom_nodes++;

  int num_geoms = geom_node->get_num_geoms();
  _num_geoms += num_geoms;

  for (int i = 0; i < num_geoms; i++) {
    Geom *geom = geom_node->get_geom(i);
    collect_statistics(DCAST(Geom, geom));

    const RenderState *geom_state = geom_node->get_geom_state(i);

    const RenderAttrib *attrib = 
      geom_state->get_attrib(TextureAttrib::get_class_type());
    if (attrib != (RenderAttrib *)NULL) {
      const TextureAttrib *ta = DCAST(TextureAttrib, attrib);
      if (!ta->is_off()) {
        collect_statistics(ta->get_texture());
      }
    }      
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::collect_statistics
//       Access: Private
//  Description: Recursively visits each node, counting up the
//               statistics.
////////////////////////////////////////////////////////////////////
void SceneGraphAnalyzer::
collect_statistics(Geom *geom) {
  int num_prims;
  int num_verts;
  int num_components;

  num_prims = geom->get_num_prims();
  if (geom->uses_components()) {
    num_verts = 0;
    num_components = 0;
    for (int i = 0; i < num_prims; i++) {
      num_verts += geom->get_length(i);
      num_components += (geom->get_length(i) - geom->get_num_more_vertices_than_components());
    }
  } else {
    num_verts = num_prims * geom->get_num_vertices_per_prim();
    num_components = 1;
  }

  _num_vertices += num_verts;

  PTA_Normalf norms;
  GeomBindType nbind;
  PTA_ushort nindex;
  geom->get_normals(norms, nbind, nindex);

  switch (nbind) {
  case G_OVERALL:
    consider_normals(norms, nindex, 1);
    break;

  case G_PER_PRIM:
    consider_normals(norms, nindex, num_prims);
    break;

  case G_PER_COMPONENT:
    consider_normals(norms, nindex, num_components);
    break;

  case G_PER_VERTEX:
    consider_normals(norms, nindex, num_verts);
    break;

  case G_OFF:
    break;
  }

  if (geom->get_binding(G_TEXCOORD) == G_PER_VERTEX) {
    _num_texcoords += num_verts;
  }

  if (geom->is_of_type(GeomPoint::get_class_type())) {
    _num_points += num_verts;

  } else if (geom->is_of_type(GeomLine::get_class_type())) {
    _num_lines += num_prims;

  } else if (geom->is_of_type(GeomLinestrip::get_class_type())) {
    _num_lines += num_components;

  } else if (geom->is_of_type(GeomPolygon::get_class_type())) {
    _num_polys += num_prims;

  } else if (geom->is_of_type(GeomQuad::get_class_type())) {
    _num_quads += num_prims;

  } else if (geom->is_of_type(GeomTri::get_class_type())) {
    _num_tris += num_prims;
    _num_individual_tris += num_prims;

  } else if (geom->is_of_type(GeomTristrip::get_class_type())) {
    _num_tris += num_components;
    _num_tristrips += num_prims;
    _num_triangles_in_strips += num_components;

  } else if (geom->is_of_type(GeomTrifan::get_class_type())) {
    _num_tris += num_components;
    _num_trifans += num_prims;
    _num_triangles_in_fans += num_components;

  } else if (geom->is_of_type(GeomSphere::get_class_type())) {
    _num_spheres += num_prims;

  } else {
    pgraph_cat.warning()
      << "Unknown GeomType in SceneGraphAnalyzer: "
      << geom->get_type() << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::collect_statistics
//       Access: Private
//  Description: Recursively visits each node, counting up the
//               statistics.
////////////////////////////////////////////////////////////////////
void SceneGraphAnalyzer::
collect_statistics(Texture *texture) {
  nassertv(texture != (Texture *)NULL);

  Textures::iterator ti = _textures.find(texture);
  if (ti == _textures.end()) {
    // This is the first time this texture has been encountered.
    _textures.insert(Textures::value_type(texture, 1));

    // Attempt to guess how many bytes of texture memory this one
    // requires.
    PixelBuffer *pb = texture->_pbuffer;
    if (pb != (PixelBuffer *)NULL) {
      int bytes =
        pb->get_xsize() * pb->get_ysize() * pb->get_num_components() *
        pb->get_component_width();

      if (texture->uses_mipmaps()) {
        bytes *= 4/3;
      }

      _texture_bytes += bytes;
    }

  } else {
    // This texture has been encountered before; don't count it again.
    (*ti).second++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::consider_normals
//       Access: Private
//  Description: Examines the indicated set of normals.
////////////////////////////////////////////////////////////////////
void SceneGraphAnalyzer::
consider_normals(const Normalf *norms, const unsigned short *nindex,
                 int num) {
  _num_normals += num;

  if (nindex != (const unsigned short *)NULL) {
    // An indexed array.
    for (int i = 0; i < num; i++) {
      const Normalf &norm = norms[nindex[i]];
      float l = norm.length();
      if (IS_THRESHOLD_EQUAL(l, 1.0f, 0.01f)) {
        // This normal is close enough to unit length to be ok.
      } else if (l > 1.0f) {
        _num_long_normals++;
      } else { // l < 1.0f
        _num_short_normals++;
      }
      _total_normal_length += l;
    }
  } else {
    // A nonindexed array.
    for (int i = 0; i < num; i++) {
      const Normalf &norm = norms[i];
      float l = norm.length();
      if (IS_THRESHOLD_EQUAL(l, 1.0f, 0.01f)) {
        // This normal is close enough to unit length to be ok.
      } else if (l > 1.0f) {
        _num_long_normals++;
      } else { // l < 1.0
        _num_short_normals++;
      }
      _total_normal_length += l;
    }
  }
}
