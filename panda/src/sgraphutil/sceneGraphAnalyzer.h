// Filename: sceneGraphAnalyzer.h
// Created by:  drose (02Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SCENEGRAPHANALYZER_H
#define SCENEGRAPHANALYZER_H

#include <pandabase.h>

#include <typeHandle.h>
#include <renderRelation.h>

#include <map>

class Node;
class GeomNode;
class Geom;
class Texture;

///////////////////////////////////////////////////////////////////
// 	 Class : SceneGraphAnalyzer
// Description : A handy class that can scrub over a scene graph and
//               collect interesting statistics on it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SceneGraphAnalyzer {
public:
  SceneGraphAnalyzer(TypeHandle graph_type = RenderRelation::get_class_type());
  ~SceneGraphAnalyzer();

  void clear();
  void add_node(Node *node);

  void write(ostream &out, int indent_level = 0) const;

private:
  void collect_statistics(Node *node, bool under_instance);
  void collect_statistics(NodeRelation *arc, bool under_instance);
  void collect_statistics(GeomNode *geom_node);
  void collect_statistics(Geom *geom);
  void collect_statistics(Texture *texture);

  void consider_normals(const Normalf *norms, const ushort *nindex, int num);

  typedef map<Node *, int> Nodes;
  typedef map<Texture *, int> Textures;

  Nodes _nodes;
  Textures _textures;

public:
  int _num_nodes;
  int _num_instances;
  int _num_transforms;
  int _num_arcs;
  int _num_arcs_with_transitions;
  int _num_geom_nodes;
  int _num_geoms;
  int _num_non_geoms;

  int _num_vertices;
  int _num_normals;
  int _num_texcoords;
  int _num_tris;
  int _num_quads;
  int _num_polys;
  int _num_lines;
  int _num_points;
  int _num_spheres;

  int _num_individual_tris;
  int _num_tristrips;
  int _num_triangles_in_strips;
  int _num_trifans;
  int _num_triangles_in_fans;

  int _texture_bytes;

  int _num_long_normals;
  int _num_short_normals;
  float _total_normal_length;

  TypeHandle _graph_type;
};

#endif
