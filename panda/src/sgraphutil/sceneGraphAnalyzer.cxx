// Filename: sceneGraphAnalyzer.cxx
// Created by:  drose (02Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "sceneGraphAnalyzer.h"
#include "config_sgraphutil.h"

#include <indent.h>
#include <geomNode.h>
#include <geom.h>
#include <geomprimitives.h>
#include <transformTransition.h>
#include <textureTransition.h>

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SceneGraphAnalyzer::
SceneGraphAnalyzer(TypeHandle graph_type) :
  _graph_type(graph_type)
{
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
  _num_arcs = 0;
  _num_arcs_with_transitions = 0;
  _num_geom_nodes = 0;
  _num_geoms = 0;
  _num_non_geoms = 0;

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
add_node(Node *node) {
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

  if (_num_arcs != 0) {
    out << "; " << 100 * _num_arcs_with_transitions / _num_arcs 
	<< "% of arcs have some transition.";
  }
  out << "\n";

  indent(out, indent_level)
    << _num_geoms << " Geoms ";
  if (_num_non_geoms != 0) {
    out << "(plus " << _num_non_geoms << " non-Geom Drawables) ";
  }
  out << "appear on " << _num_geom_nodes << " GeomNodes.\n";

  indent(out, indent_level)
    << _num_vertices << " vertices, " << _num_normals << " normals, "
    << _num_texcoords << " texture coordinates.\n";

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
collect_statistics(Node *node, bool under_instance) {
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
  
  if (node->is_of_type(GeomNode::get_class_type())) {
    collect_statistics(DCAST(GeomNode, node));
  }
  
  int num_children = node->get_num_children(_graph_type);
  for (int i = 0; i < num_children; i++) {
    NodeRelation *arc = node->get_child(_graph_type, i);
    collect_statistics(arc, under_instance);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphAnalyzer::collect_statistics
//       Access: Private
//  Description: Recursively visits each node, counting up the
//               statistics.
////////////////////////////////////////////////////////////////////
void SceneGraphAnalyzer::
collect_statistics(NodeRelation *arc, bool under_instance) {
  _num_arcs++;
  if (arc->has_any_transition()) {
    _num_arcs_with_transitions++;
  }
  if (arc->has_transition(TransformTransition::get_class_type())) {
    _num_transforms++;
  }

  TextureTransition *tt;
  if (get_transition_into(tt, arc)) {
    if (tt->is_on()) {
      collect_statistics(tt->get_texture());
    }
  }

  collect_statistics(arc->get_child(), under_instance);
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
  for (int i = 0; i < num_geoms; i++) {
    dDrawable *geom = geom_node->get_geom(i);

    if (geom->is_of_type(Geom::get_class_type())) {
      _num_geoms++;

      collect_statistics(DCAST(Geom, geom));
    } else {
      _num_non_geoms++;
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

  switch (geom->get_binding(G_NORMAL)) {
  case G_OVERALL:
    _num_normals++;
    break;

  case G_PER_PRIM:
    _num_normals += num_prims;
    break;

  case G_PER_COMPONENT:
    _num_normals += num_components;
    break;

  case G_PER_VERTEX:
    _num_normals += num_verts;
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
    sgraphutil_cat.warning()
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
      
      bool is_mipmapped = false;
      switch (texture->get_minfilter()) {
      case Texture::FT_nearest_mipmap_nearest:
      case Texture::FT_linear_mipmap_nearest:
      case Texture::FT_nearest_mipmap_linear:
      case Texture::FT_linear_mipmap_linear:
	is_mipmapped = true;
      }
      
      if (is_mipmapped) {
	bytes *= 4/3;
      }

      _texture_bytes += bytes;
    }
  
  } else {
    // This texture has been encountered before; don't count it again.
    (*ti).second++;
  }
}

